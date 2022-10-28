/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class DistortionAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    DistortionAudioProcessorEditor (DistortionAudioProcessor&);
    ~DistortionAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::ScopedPointer<juce::Slider> _driveKnob;
    juce::ScopedPointer<juce::Slider> _rangeKnob;
    juce::ScopedPointer<juce::Slider> _blendKnob;
    juce::ScopedPointer<juce::Slider> _volumeKnob;

    juce::ScopedPointer<juce::AudioProcessorValueTreeState::SliderAttachment> _driveAttachment;
    juce::ScopedPointer<juce::AudioProcessorValueTreeState::SliderAttachment> _rangeAttachment;
    juce::ScopedPointer<juce::AudioProcessorValueTreeState::SliderAttachment> _blendAttachment;
    juce::ScopedPointer<juce::AudioProcessorValueTreeState::SliderAttachment> _volumeAttachment;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    DistortionAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistortionAudioProcessorEditor)
};
