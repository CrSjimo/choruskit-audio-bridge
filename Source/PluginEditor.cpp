/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ChorusKitAudioProcessorEditor::ChorusKitAudioProcessorEditor(ChorusKitAudioProcessor &p)
        : AudioProcessorEditor(&p), audioProcessor(p) {
    mainButton.setButtonText(juce::String("Show ") + ChorusKit_PluginEditorName);
    mainButton.setColour(juce::TextButton::buttonColourId, juce::Colour(ChorusKit_ForegroundColor));
    mainButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(ChorusKit_ForegroundOnColor));
    mainButton.setColour(juce::TextButton::textColourOnId, juce::Colour(ChorusKit_ForegroundLabelColor));
    mainButton.setColour(juce::TextButton::textColourOffId, juce::Colour(ChorusKit_ForegroundLabelColor));
    mainButton.addListener(this);

    statusLabel.setColour(juce::Label::textColourId, juce::Colour(ChorusKit_BackgroundMessageColor));
    versionLabel.setColour(juce::Label::textColourId, juce::Colour(ChorusKit_BackgroundMessageColor));
    errorLabel.setColour(juce::Label::textColourId, juce::Colour(ChorusKit_BackgroundErrorColor));
    errorLabel.setJustificationType(juce::Justification::bottom);
    versionLabel.setText(juce::String("Version ") + JucePlugin_VersionString, juce::dontSendNotification);
    versionLabel.setJustificationType(juce::Justification::right);

    addAndMakeVisible(statusLabel);
    addAndMakeVisible(errorLabel);
    addAndMakeVisible(versionLabel);
    addAndMakeVisible(mainButton);
    setSize(400, 200);
    std::cerr << "Initialized: Editor" << std::endl;
}

ChorusKitAudioProcessorEditor::~ChorusKitAudioProcessorEditor() {
}

//==============================================================================
void ChorusKitAudioProcessorEditor::paint(juce::Graphics &g) {
    g.fillAll(juce::Colour(ChorusKit_BackgroundMainColor));
    g.setColour(juce::Colour(ChorusKit_BackgroundSidebarColor));
    g.fillRect(0, 150, 400, 50);
}

void ChorusKitAudioProcessorEditor::resized() {
    mainButton.setBounds(100, 50, 200, 50);
    statusLabel.setBounds(8, 176, 384, 16);
    errorLabel.setBounds(8, 16, 384, 160);
    versionLabel.setBounds(8, 176, 384, 16);
}

void ChorusKitAudioProcessorEditor::buttonClicked(juce::Button *) {

}
