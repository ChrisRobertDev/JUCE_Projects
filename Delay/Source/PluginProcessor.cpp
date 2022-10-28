/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DelayKadenzeAudioProcessor::DelayKadenzeAudioProcessor()
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

    addParameter(mFeedbackParameter = new juce::AudioParameterFloat("feedback", "Feedback", 0, 0.98, 0.5));

    addParameter(mDelayTimeParameter = new juce::AudioParameterFloat("delaytime", "Delay Time", 0.01, MAX_DELAY_TIME, 0.5));

    mCircularBufferLeft = nullptr;
    mCircularBufferRight = nullptr;
    mCircularBufferWriteHead = 0;
    mCircularBufferLength = 0;
    mDelayTimeInSamples = 0;
    mDelayReadHead = 0;
    mFeedbackLeft = 0.0;
    mFeedbackRight = 0.0;
    mDelayTimeSmoothed = 0.0;
}

DelayKadenzeAudioProcessor::~DelayKadenzeAudioProcessor()
{
    if (mCircularBufferLeft != nullptr) {
        delete[] mCircularBufferLeft;
    }

    if (mCircularBufferRight != nullptr) {
        delete[] mCircularBufferRight;
    }
}

//==============================================================================
const juce::String DelayKadenzeAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DelayKadenzeAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DelayKadenzeAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DelayKadenzeAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DelayKadenzeAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DelayKadenzeAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DelayKadenzeAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DelayKadenzeAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DelayKadenzeAudioProcessor::getProgramName (int index)
{
    return {};
}

void DelayKadenzeAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DelayKadenzeAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mDelayTimeInSamples = *mDelayTimeParameter * sampleRate;

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

    mDelayTimeSmoothed = *mDelayTimeParameter;
}

void DelayKadenzeAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DelayKadenzeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void DelayKadenzeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    

    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);

    for (int i = 0; i < buffer.getNumSamples(); i++) {
        mDelayTimeSmoothed = mDelayTimeSmoothed - 0.001 * (mDelayTimeSmoothed - *mDelayTimeParameter);
        mDelayTimeInSamples = mDelayTimeSmoothed * getSampleRate();

        mCircularBufferLeft[mCircularBufferWriteHead] = leftChannel[i] + mFeedbackLeft;
        mCircularBufferRight[mCircularBufferWriteHead] = rightChannel[i] + mFeedbackRight;

        mDelayReadHead = mCircularBufferWriteHead - mDelayTimeInSamples;

        if (mDelayReadHead < 0) {
            mDelayReadHead += mCircularBufferLength;
        }

        //interpolation
        int readHead_x = (int)mDelayReadHead;
        int readHead_x1 = (readHead_x + 1) % mCircularBufferLength; //wrap around if not in interval

        float readHeadFloat = mDelayReadHead - readHead_x;

        float delay_sample_left = lin_interp(mCircularBufferLeft[readHead_x], mCircularBufferLeft[readHead_x1], readHeadFloat);
        float delay_sample_right = lin_interp(mCircularBufferRight[readHead_x], mCircularBufferRight[readHead_x1], readHeadFloat);
        
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
bool DelayKadenzeAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DelayKadenzeAudioProcessor::createEditor()
{
    return new DelayKadenzeAudioProcessorEditor (*this);
}

//==============================================================================
void DelayKadenzeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DelayKadenzeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

float DelayKadenzeAudioProcessor::lin_interp(float sample_x, float sample_x1, float inPhase)
{
    return (1.0 - inPhase) * sample_x + inPhase * sample_x1;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DelayKadenzeAudioProcessor();
}
