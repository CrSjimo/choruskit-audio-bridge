/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginAudioProcessor.h"
#include "PluginAudioProcessorEditor.h"

#include "Bridge.h"

#include <RemoteEditorInterface.h>

static juce::String formatStatusString(juce::StringRef str, bool isARA) {
    return "Status: " + str + (isARA ? " (ARA)" : "");
}

//==============================================================================
PluginAudioProcessorEditor::PluginAudioProcessorEditor(PluginAudioProcessor &p)
        : AudioProcessorEditor(&p), audioProcessor(p) {
    if(ckBdg->getRemoteSocket()) {
        ckBdg->getRemoteSocket()->addListener(this);
    }

    mainButton.setButtonText(juce::StringRef("Show ") + ChorusKit_PluginEditorName);
    mainButton.setColour(juce::TextButton::buttonColourId, juce::Colour(ChorusKit_ForegroundColor));
    mainButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(ChorusKit_ForegroundOnColor));
    mainButton.setColour(juce::TextButton::textColourOnId, juce::Colour(ChorusKit_ForegroundLabelColor));
    mainButton.setColour(juce::TextButton::textColourOffId, juce::Colour(ChorusKit_ForegroundLabelColor));
    mainButton.addListener(this);

    statusLabel.setColour(juce::Label::textColourId, juce::Colour(ChorusKit_BackgroundMessageColor));
    versionLabel.setColour(juce::Label::textColourId, juce::Colour(ChorusKit_BackgroundMessageColor));
    errorLabel.setColour(juce::Label::textColourId, juce::Colour(ChorusKit_BackgroundErrorColor));
    errorLabel.setJustificationType(juce::Justification::bottom);
    versionLabel.setJustificationType(juce::Justification::right);

    if (ckBdg->getRemoteSocket() && ckBdg->getRemoteSocket()->status() == talcs::RemoteSocket::Connected)
        statusLabel.setText(formatStatusString("Connected", audioProcessor.isBoundToARA()), juce::dontSendNotification);
    else
        statusLabel.setText(formatStatusString("Not Connected", audioProcessor.isBoundToARA()), juce::dontSendNotification);
    errorLabel.setText(ckBdg->getError(), juce::dontSendNotification);
    versionLabel.setText(juce::StringRef("Version ") + JucePlugin_VersionString, juce::dontSendNotification);

    if(!ckBdg->getError().isEmpty()) {
        mainButton.setEnabled(false);
    }

    addAndMakeVisible(statusLabel);
    addAndMakeVisible(errorLabel);
    addAndMakeVisible(versionLabel);
    addAndMakeVisible(mainButton);
    setSize(400, 200);

    if (ckBdg->getRemoteSocket() && ckBdg->getRemoteEditorInterface()) {
        if (ckBdg->getRemoteSocket()->status() == talcs::RemoteSocket::Connected)
            ckBdg->getRemoteEditorInterface()->showEditor();
    }

    std::cerr << "Initialized: Editor" << std::endl;
}

PluginAudioProcessorEditor::~PluginAudioProcessorEditor() {
    if (ckBdg->getRemoteSocket())
        ckBdg->getRemoteSocket()->removeListener(this);

    if (ckBdg->getRemoteEditorInterface())
        ckBdg->getRemoteEditorInterface()->hideEditor();
}

//==============================================================================
void PluginAudioProcessorEditor::paint(juce::Graphics &g) {
    g.fillAll(juce::Colour(ChorusKit_BackgroundMainColor));
    g.setColour(juce::Colour(ChorusKit_BackgroundSidebarColor));
    g.fillRect(0, 150, 400, 50);
}

void PluginAudioProcessorEditor::resized() {
    mainButton.setBounds(100, 50, 200, 50);
    statusLabel.setBounds(8, 176, 384, 16);
    errorLabel.setBounds(8, 16, 384, 160);
    versionLabel.setBounds(8, 176, 384, 16);
}

void PluginAudioProcessorEditor::buttonClicked(juce::Button *) {
    if (!ckBdg->getRemoteSocket() || !ckBdg->getRemoteEditorInterface())
        return;
    if (ckBdg->getRemoteSocket()->status() == talcs::RemoteSocket::Connected)
        ckBdg->getRemoteEditorInterface()->showEditor();
    else
        ckBdg->startEditorProgram();
}

void PluginAudioProcessorEditor::socketStatusChanged(int newStatus, int oldStatus) {
    if (newStatus == talcs::RemoteSocket::Connected) {
        juce::MessageManagerLock mmLock;
        statusLabel.setText(formatStatusString("Connected", audioProcessor.isBoundToARA()), juce::dontSendNotification);
    } else if (oldStatus == talcs::RemoteSocket::Connected) {
        juce::MessageManagerLock mmLock;
        statusLabel.setText(formatStatusString("Not Connected", audioProcessor.isBoundToARA()), juce::dontSendNotification);
    }
}
