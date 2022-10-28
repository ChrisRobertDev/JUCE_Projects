/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DistortionAudioProcessorEditor::DistortionAudioProcessorEditor (DistortionAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible(_driveKnob = new juce::Slider("Drive"));
    _driveKnob->setSliderStyle(juce::Slider::Rotary);
    _driveKnob->setTextBoxStyle(juce::Slider::NoTextBox, false, 100, 100);

    addAndMakeVisible(_rangeKnob = new juce::Slider("Range"));
    _rangeKnob->setSliderStyle(juce::Slider::Rotary);
    _rangeKnob->setTextBoxStyle(juce::Slider::NoTextBox, false, 100, 100);

    addAndMakeVisible(_blendKnob = new juce::Slider("Blend"));
    _blendKnob->setSliderStyle(juce::Slider::Rotary);
    _blendKnob->setTextBoxStyle(juce::Slider::NoTextBox, false, 100, 100);

    addAndMakeVisible(_volumeKnob = new juce::Slider("Volume"));
    _volumeKnob->setSliderStyle(juce::Slider::Rotary);
    _volumeKnob->setTextBoxStyle(juce::Slider::NoTextBox, false, 100, 100);

    _driveAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment(p.getState(),"drive", *_driveKnob);
    _rangeAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment(p.getState(), "range", *_rangeKnob);
    _blendAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment(p.getState(), "blend", *_blendKnob);
    _volumeAttachment = new juce::AudioProcessorValueTreeState::SliderAttachment(p.getState(), "volume", *_volumeKnob);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (500, 200);
}

DistortionAudioProcessorEditor::~DistortionAudioProcessorEditor()
{
}

//==============================================================================
void DistortionAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);

    g.drawText("Drive", getWidth() / 5 - 100 / 2, getHeight() / 2 + 5, 100, 100, juce::Justification::centred, false);
    g.drawText("Range", getWidth() * 2 / 5 - 100 / 2, getHeight() / 2 + 5, 100, 100, juce::Justification::centred, false);
    g.drawText("Blend", getWidth() * 3 / 5 - 100 / 2, getHeight() / 2 + 5, 100, 100, juce::Justification::centred, false);
    g.drawText("Volume", getWidth() * 4 / 5 - 100 / 2, getHeight() / 2 + 5, 100, 100, juce::Justification::centred, false);
}

void DistortionAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    _driveKnob->setBounds(getWidth() / 5 - 100 / 2, getHeight() / 2 - 100 / 2, 100, 100);
    _rangeKnob->setBounds(getWidth() * 2 / 5 - 100 / 2, getHeight() / 2 - 100 / 2, 100, 100);
    _blendKnob->setBounds(getWidth() * 3 / 5 - 100 / 2, getHeight() / 2 - 100 / 2, 100, 100);
    _volumeKnob->setBounds(getWidth() * 4 / 5 - 100 / 2, getHeight() / 2 - 100 / 2, 100, 100);
}
