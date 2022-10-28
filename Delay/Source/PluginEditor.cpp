/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DelayKadenzeAudioProcessorEditor::DelayKadenzeAudioProcessorEditor (DelayKadenzeAudioProcessor& p)
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

    juce::AudioParameterFloat* feedbackParameter = (juce::AudioParameterFloat*)params.getUnchecked(1);

    mFeedbackSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    mFeedbackSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mFeedbackSlider.setRange(feedbackParameter->range.start, feedbackParameter->range.end);
    mFeedbackSlider.setValue(*feedbackParameter);
    addAndMakeVisible(mFeedbackSlider);

    mFeedbackSlider.onValueChange = [this, feedbackParameter] {*feedbackParameter = mFeedbackSlider.getValue(); };
    mFeedbackSlider.onDragStart = [feedbackParameter] {feedbackParameter->beginChangeGesture(); };
    mFeedbackSlider.onDragEnd = [feedbackParameter] {feedbackParameter->endChangeGesture(); };

    juce::AudioParameterFloat* DelayTimeParameter = (juce::AudioParameterFloat*)params.getUnchecked(2);

    mDelayTimeSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    mDelayTimeSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    mDelayTimeSlider.setRange(DelayTimeParameter->range.start, DelayTimeParameter->range.end);
    mDelayTimeSlider.setValue(*DelayTimeParameter);
    addAndMakeVisible(mDelayTimeSlider);

    mDelayTimeSlider.onValueChange = [this, DelayTimeParameter] {*DelayTimeParameter = mDelayTimeSlider.getValue(); };
    mDelayTimeSlider.onDragStart = [DelayTimeParameter] {DelayTimeParameter->beginChangeGesture(); };
    mDelayTimeSlider.onDragEnd = [DelayTimeParameter] {DelayTimeParameter->endChangeGesture(); };

    setSize (400, 300);
}

DelayKadenzeAudioProcessorEditor::~DelayKadenzeAudioProcessorEditor()
{
}

//==============================================================================
void DelayKadenzeAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);

    g.drawText("Delay", 200, 50, 100, 100, juce::Justification::centred, false);
    g.drawText("Feedback", 100, 50, 100, 100, juce::Justification::centred, false);
    g.drawText("Wet/Dry", 0, 50, 100, 100, juce::Justification::centred, false);
}

void DelayKadenzeAudioProcessorEditor::resized()
{
    mDryWetSlider.setBounds(0, 0, 100, 100);
    mFeedbackSlider.setBounds(100, 0, 100, 100);

    mDelayTimeSlider.setBounds(200, 0, 100, 100);

    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
