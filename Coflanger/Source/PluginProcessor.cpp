/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CoflangerAudioProcessor::CoflangerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    addParameter(mDryWetParameter = new juce::AudioParameterFloat("drywet", "Dry Wet", 0.0, 1.0, 0.5));

    addParameter(mDepthParameter = new juce::AudioParameterFloat("depth", "Depth", 0, 1.0, 0.5));

    addParameter(mRateParameter = new juce::AudioParameterFloat("rate", "Rate", 0.1f, 20.0f, 10.0f));

    addParameter(mPhaseOffsetParameter = new juce::AudioParameterFloat("phaseoffset", "Phase Offset", 0.0f, 1.0f, 0.0f));

    addParameter(mFeedbackParameter = new juce::AudioParameterFloat("feedback", "Feedback", 0, 0.98, 0.5));

    addParameter(mTypeParameter = new juce::AudioParameterInt("type", "Type", 0, 1, 1));

    mCircularBufferLeft = nullptr;
    mCircularBufferRight = nullptr;
    mCircularBufferWriteHead = 0;
    mCircularBufferLength = 0;

    mFeedbackLeft = 0.0;
    mFeedbackRight = 0.0;


    mLFOPhaseL = 0.0;
    mLFOPhaseR = 0.0;
}

CoflangerAudioProcessor::~CoflangerAudioProcessor()
{
    if (mCircularBufferLeft != nullptr) {
        delete[] mCircularBufferLeft;
    }

    if (mCircularBufferRight != nullptr) {
        delete[] mCircularBufferRight;
    }
}

//==============================================================================
const juce::String CoflangerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CoflangerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CoflangerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CoflangerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CoflangerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CoflangerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CoflangerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CoflangerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CoflangerAudioProcessor::getProgramName (int index)
{
    return {};
}

void CoflangerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CoflangerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    //mDelayTimeInSamples = *mDelayTimeParameter * sampleRate;//not needed here
    //mDelayTimeSmoothed = *mDelayTimeParameter;



    if (mCircularBufferLeft != nullptr) {
        delete[] mCircularBufferLeft;
        mCircularBufferLeft = new float[sampleRate * MAX_DELAY_TIME];
        for (int i = 0; i < sampleRate * MAX_DELAY_TIME; i++)
        {
            mCircularBufferLeft[i] = 0.0;
        }
    }

    if (mCircularBufferLeft == nullptr) {
        mCircularBufferLeft = new float[sampleRate * MAX_DELAY_TIME];
        for (int i = 0; i < sampleRate * MAX_DELAY_TIME; i++)
        {
            mCircularBufferLeft[i] = 0.0;
        }
    }

    if (mCircularBufferRight != nullptr) {
        delete[] mCircularBufferRight;
        mCircularBufferRight = new float[sampleRate * MAX_DELAY_TIME];
        for (int i = 0; i < sampleRate * MAX_DELAY_TIME; i++)
        {
            mCircularBufferRight[i] = 0.0;
        }
    }

    if (mCircularBufferRight == nullptr) {
        mCircularBufferRight = new float[sampleRate * MAX_DELAY_TIME];
        for (int i = 0; i < sampleRate * MAX_DELAY_TIME; i++)
        {
            mCircularBufferRight[i] = 0.0;
        }
    }
    mCircularBufferLength = MAX_DELAY_TIME * sampleRate;
    mCircularBufferWriteHead = 0;

    
    mLFOPhaseL = 0.0;
    mLFOPhaseR = 0.0;
}

void CoflangerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CoflangerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void CoflangerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    //DBG("DRY WET:" << *mDryWetParameter); // asaa se face debuuug


    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);

    for (int i = 0; i < buffer.getNumSamples(); i++) {
        float lfoOutLeft = std::sin(juce::MathConstants<float>::twoPi * mLFOPhaseL);
        float lfoOutRight = std::sin(juce::MathConstants<float>::twoPi * mLFOPhaseR);
        //Add Chorus Depth
        lfoOutLeft *= *mDepthParameter;
        lfoOutRight *= *mDepthParameter;

        float lfoOutMappedLeft = 0.0;
        float lfoOutMappedRight = 0.0;

        //CHORUS
        if (*mTypeParameter == 0) {
            //Map lfo to delayTime
            lfoOutMappedLeft = juce::jmap<float>(lfoOutLeft, -1.0, 1.0, 0.005f, 0.03f);
            lfoOutMappedRight = juce::jmap<float>(lfoOutRight, -1.0, 1.0, 0.005f, 0.03f);
        }
        else
        {//FLANGER
            //Map lfo to delayTime
            lfoOutMappedLeft = juce::jmap<float>(lfoOutLeft, -1.0, 1.0, 0.001f, 0.005f);
            lfoOutMappedRight = juce::jmap<float>(lfoOutRight, -1.0, 1.0, 0.001f, 0.005f);
        }

        

        //calculate delayTime
        float delayTimeSamplesLeft = lfoOutMappedLeft * getSampleRate();
        float delayTimeSamplesRight = lfoOutMappedRight * getSampleRate();
        //LFO phase
        mLFOPhaseR = mLFOPhaseL + *mPhaseOffsetParameter;
        mLFOPhaseL += *mRateParameter / getSampleRate();//first calculate then add up
        //verify wrapping
        if (mLFOPhaseL > 1.0)
            mLFOPhaseL -= 1.0;
        if (mLFOPhaseR > 1) {
            mLFOPhaseR -= 1;
        }
        
        mCircularBufferLeft[mCircularBufferWriteHead] = leftChannel[i] + mFeedbackLeft;
        mCircularBufferRight[mCircularBufferWriteHead] = rightChannel[i] + mFeedbackRight;

        float delayReadHeadLeft = mCircularBufferWriteHead - delayTimeSamplesLeft;
        float delayReadHeadRight = mCircularBufferWriteHead - delayTimeSamplesRight;
        //verify readheads for < 0
        if (delayReadHeadLeft < 0)
            delayReadHeadLeft += mCircularBufferLength;
        if (delayReadHeadRight < 0)
            delayReadHeadRight += mCircularBufferLength;

        //interpolation LEFT
        int readHeadLeft_x = (int)delayReadHeadLeft;
        int readHeadLeft_x1 = (readHeadLeft_x + 1) % mCircularBufferLength; //wrap around if not in interval
        float readHeadFloatLeft = delayReadHeadLeft - readHeadLeft_x;
        //interpolation RIGHT
        int readHeadRight_x = (int)delayReadHeadRight;
        int readHeadRight_x1 = (readHeadRight_x + 1) % mCircularBufferLength; //wrap around if not in interval
        float readHeadFloatRight = delayReadHeadRight - readHeadRight_x;

        float delay_sample_left = lin_interp(mCircularBufferLeft[readHeadLeft_x], mCircularBufferLeft[readHeadLeft_x1], readHeadFloatLeft);
        float delay_sample_right = lin_interp(mCircularBufferRight[readHeadRight_x], mCircularBufferRight[readHeadRight_x1], readHeadFloatRight);

        mFeedbackLeft = delay_sample_left * *mFeedbackParameter;
        mFeedbackRight = delay_sample_right * *mFeedbackParameter;


        mCircularBufferWriteHead++;

        if (mCircularBufferWriteHead >= mCircularBufferLength)
            mCircularBufferWriteHead = 0;

        leftChannel[i] = leftChannel[i] * (1.0 - *mDryWetParameter) + delay_sample_left * *mDryWetParameter;
        rightChannel[i] = rightChannel[i] * (1.0 - *mDryWetParameter) + delay_sample_right * *mDryWetParameter;

        //buffer.setSample(0, i, buffer.getSample(0, i) * *mDryWetParameter + delay_sample_left * (1.0 - *mDryWetParameter));
        //buffer.setSample(1, i, buffer.getSample(1, i) * *mDryWetParameter + delay_sample_right * (1.0 - *mDryWetParameter));
    }
}

//==============================================================================
bool CoflangerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CoflangerAudioProcessor::createEditor()
{
    return new CoflangerAudioProcessorEditor (*this);
}

//==============================================================================
void CoflangerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    std::unique_ptr<juce::XmlElement> xml(new juce::XmlElement("Coflanger"));

    xml->setAttribute("DryWet", *mDryWetParameter);
    xml->setAttribute("Depth", *mDepthParameter);
    xml->setAttribute("Rate", *mRateParameter);
    xml->setAttribute("PhaseOffset", *mPhaseOffsetParameter);
    xml->setAttribute("Feedback", *mFeedbackParameter);
    xml->setAttribute("Type", *mTypeParameter);

    copyXmlToBinary(*xml, destData);
}

void CoflangerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

    if (xml.get() != nullptr && xml->hasTagName("Coflanger")) {
        *mDryWetParameter = xml->getDoubleAttribute("DryWet");
        *mDepthParameter = xml->getDoubleAttribute("Depth");
        *mRateParameter = xml->getDoubleAttribute("Rate");
        *mPhaseOffsetParameter = xml->getDoubleAttribute("PhaseOffset");
        *mFeedbackParameter = xml->getDoubleAttribute("Feedback");
        *mTypeParameter = xml->getDoubleAttribute("Type");
    }

}

float CoflangerAudioProcessor::lin_interp(float sample_x, float sample_x1, float inPhase)
{
    return (1.0 - inPhase) * sample_x + inPhase * sample_x1;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CoflangerAudioProcessor();
}
