/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CoflangerAudioProcessorEditor::CoflangerAudioProcessorEditor (CoflangerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    auto& params = processor.getParameters();

    juce::AudioParameterFloat* dryWetParameter = (juce::AudioParameterFloat*)params.getUnchecked(0);

    mDryWetSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    mDryWetSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mDryWetSlider.setRange(dryWetParameter->range.start, dryWetParameter->range.end);
    mDryWetSlider.setValue(*dryWetParameter);
    addAndMakeVisible(mDryWetSlider);

    mDryWetSlider.onValueChange = [this, dryWetParameter] {*dryWetParameter = mDryWetSlider.getValue(); };
    mDryWetSlider.onDragStart = [dryWetParameter] {dryWetParameter->beginChangeGesture(); };
    mDryWetSlider.onDragEnd = [dryWetParameter] {dryWetParameter->endChangeGesture(); };
    //asd
    juce::AudioParameterFloat* depthParameter = (juce::AudioParameterFloat*)params.getUnchecked(1);

    mDepthSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    mDepthSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mDepthSlider.setRange(depthParameter->range.start, depthParameter->range.end);
    mDepthSlider.setValue(*depthParameter);
    addAndMakeVisible(mDepthSlider);

    mDepthSlider.onValueChange = [this, depthParameter] {*depthParameter = mDepthSlider.getValue(); };
    mDepthSlider.onDragStart = [depthParameter] {depthParameter->beginChangeGesture(); };
    mDepthSlider.onDragEnd = [depthParameter] {depthParameter->endChangeGesture(); };
    //asd
    juce::AudioParameterFloat* rateParameter = (juce::AudioParameterFloat*)params.getUnchecked(2);

    mRateSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    mRateSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mRateSlider.setRange(rateParameter->range.start, rateParameter->range.end);
    mRateSlider.setValue(*rateParameter);
    addAndMakeVisible(mRateSlider);

    mRateSlider.onValueChange = [this, rateParameter] {*rateParameter = mRateSlider.getValue(); };
    mRateSlider.onDragStart = [rateParameter] {rateParameter->beginChangeGesture(); };
    mRateSlider.onDragEnd = [rateParameter] {rateParameter->endChangeGesture(); };
    //asd
    juce::AudioParameterFloat* phaseOffsetParameter = (juce::AudioParameterFloat*)params.getUnchecked(3);

    mPhaseOffsetSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    mPhaseOffsetSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mPhaseOffsetSlider.setRange(phaseOffsetParameter->range.start, phaseOffsetParameter->range.end);
    mPhaseOffsetSlider.setValue(*phaseOffsetParameter);
    addAndMakeVisible(mPhaseOffsetSlider);

    mPhaseOffsetSlider.onValueChange = [this, phaseOffsetParameter] {*phaseOffsetParameter = mPhaseOffsetSlider.getValue(); };
    mPhaseOffsetSlider.onDragStart = [phaseOffsetParameter] {phaseOffsetParameter->beginChangeGesture(); };
    mPhaseOffsetSlider.onDragEnd = [phaseOffsetParameter] {phaseOffsetParameter->endChangeGesture(); };
    //asd
    juce::AudioParameterFloat* feedbackParameter = (juce::AudioParameterFloat*)params.getUnchecked(4);

    mFeedbackSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    mFeedbackSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mFeedbackSlider.setRange(feedbackParameter->range.start, feedbackParameter->range.end);
    mFeedbackSlider.setValue(*feedbackParameter);
    addAndMakeVisible(mFeedbackSlider);

    mFeedbackSlider.onValueChange = [this, feedbackParameter] {*feedbackParameter = mFeedbackSlider.getValue(); };
    mFeedbackSlider.onDragStart = [feedbackParameter] {feedbackParameter->beginChangeGesture(); };
    mFeedbackSlider.onDragEnd = [feedbackParameter] {feedbackParameter->endChangeGesture(); };
    //asd
    juce::AudioParameterInt* typeParameter = (juce::AudioParameterInt*)params.getUnchecked(5);

    mType.addItem("Chorus", 1);
    mType.addItem("Flanger", 2);
    mType.onChange = [this, typeParameter] {
        typeParameter->beginChangeGesture();
        *typeParameter = mType.getSelectedItemIndex();
        typeParameter->endChangeGesture();
    };
    addAndMakeVisible(mType);


    setSize (400, 300);
}

CoflangerAudioProcessorEditor::~CoflangerAudioProcessorEditor()
{
}

//==============================================================================
void CoflangerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);

    g.drawText("Wet/Dry", 0, 50, 100, 100, juce::Justification::centred, false);
    g.drawText("Depth", 100, 50, 100, 100, juce::Justification::centred, false);
    g.drawText("Rate", 200, 50, 100, 100, juce::Justification::centred, false);
    g.drawText("Phase Offset", 300, 50, 100, 100, juce::Justification::centred, false);
    g.drawText("Feedback", 0, 150, 100, 100, juce::Justification::centred, false);
}

void CoflangerAudioProcessorEditor::resized()
{
    mDryWetSlider.setBounds(0, 0, 100, 100);
    mDepthSlider.setBounds(100, 0, 100, 100);
    mRateSlider.setBounds(200, 0, 100, 100);
    mPhaseOffsetSlider.setBounds(300, 0, 100, 100);
    mFeedbackSlider.setBounds(0, 100, 100, 100);
    mType.setBounds(250, 150, 80, 30);
}
