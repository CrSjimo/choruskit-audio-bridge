/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <memory>

#include <RemoteSocket.h>

#include "PluginAudioProcessor.h"

//==============================================================================
/**
*/
class PluginAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Button::Listener, public talcs::RemoteSocket::Listener
{
public:
    PluginAudioProcessorEditor (PluginAudioProcessor&);
    ~PluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void buttonClicked(juce::Button *) override;
    void socketStatusChanged(int newStatus, int oldStatus) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginAudioProcessor& audioProcessor;
    juce::TextButton mainButton;
    juce::Label statusLabel;
    juce::Label errorLabel;
    juce::Label versionLabel;
    std::unique_ptr<juce::LookAndFeel> buttonLookAndFeel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginAudioProcessorEditor)
};
