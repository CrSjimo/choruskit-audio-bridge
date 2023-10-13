/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "PluginProcessor.h"

//==============================================================================
/**
*/
class ChorusKitAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Button::Listener
{
public:
    ChorusKitAudioProcessorEditor (ChorusKitAudioProcessor&);
    ~ChorusKitAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void buttonClicked(juce::Button *) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ChorusKitAudioProcessor& audioProcessor;
    juce::TextButton mainButton;
    juce::Label statusLabel;
    juce::Label errorLabel;
    juce::Label versionLabel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChorusKitAudioProcessorEditor)
};
