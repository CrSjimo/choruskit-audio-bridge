/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginAudioProcessor.h"
#include "PluginAudioProcessorEditor.h"

#include "Bridge.h"

static juce::String formatStatusString(juce::StringRef str, bool isARA) {
    return "Status: " + str + (isARA ? " (ARA)" : "");
}

class PluginAudioProcessorEditorLookAndFeel : public juce::LookAndFeel_V4 {
    void drawButtonBackground(juce::Graphics &g, juce::Button &button, const juce::Colour &backgroundColour, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override {
        auto cornerSize = 6.0f;
        auto bounds = button.getLocalBounds().toFloat().reduced (0.5f, 0.5f);

        auto baseColour = backgroundColour.withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f);

        if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
            baseColour = baseColour.contrasting (shouldDrawButtonAsDown ? 0.2f : 0.05f);

        g.setColour (baseColour);
        g.fillRoundedRectangle (bounds, cornerSize);
        g.setColour (button.findColour (juce::ComboBox::outlineColourId));
        g.drawRoundedRectangle (bounds, cornerSize, 1.0f);
    }
};

//==============================================================================
PluginAudioProcessorEditor::PluginAudioProcessorEditor(PluginAudioProcessor &p)
        : AudioProcessorEditor(&p), audioProcessor(p) {
    if(ckBdg->getRemoteSocket()) {
        ckBdg->getRemoteSocket()->addListener(this);
    }

    buttonLookAndFeel = std::make_unique<PluginAudioProcessorEditorLookAndFeel>();
    mainButton.setLookAndFeel(buttonLookAndFeel.get());
    mainButton.setButtonText(juce::StringRef("Show ") + ChorusKit_PluginEditorName);
    mainButton.setColour(juce::TextButton::buttonColourId, juce::Colour(static_cast<juce::uint32>(int(ckBdg->getTheme().getProperty("foreground", static_cast<int>(ChorusKit_ForegroundColor))))));
    mainButton.setColour(juce::ComboBox::outlineColourId, juce::Colour(static_cast<juce::uint32>(int(ckBdg->getTheme().getProperty("foregroundBorder", static_cast<int>(ChorusKit_ForegroundBorderColor))))));
    mainButton.setColour(juce::TextButton::textColourOnId, juce::Colour(static_cast<juce::uint32>(int(ckBdg->getTheme().getProperty("foregroundLabel", static_cast<int>(ChorusKit_ForegroundLabelColor))))));
    mainButton.setColour(juce::TextButton::textColourOffId, juce::Colour(static_cast<juce::uint32>(int(ckBdg->getTheme().getProperty("foregroundLabel", static_cast<int>(ChorusKit_ForegroundLabelColor))))));
    mainButton.addListener(this);

    statusLabel.setColour(juce::Label::textColourId, juce::Colour(static_cast<juce::uint32>(int(ckBdg->getTheme().getProperty("backgroundMessage", static_cast<int>(ChorusKit_BackgroundMessageColor))))));
    versionLabel.setColour(juce::Label::textColourId, juce::Colour(static_cast<juce::uint32>(int(ckBdg->getTheme().getProperty("backgroundMessage", static_cast<int>(ChorusKit_BackgroundMessageColor))))));
    errorLabel.setColour(juce::Label::textColourId, juce::Colour(static_cast<juce::uint32>(int(ckBdg->getTheme().getProperty("backgroundError", static_cast<int>(ChorusKit_BackgroundErrorColor))))));
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
    g.fillAll(juce::Colour(static_cast<juce::uint32>(int(ckBdg->getTheme().getProperty("backgroundMain", static_cast<int>(ChorusKit_BackgroundMainColor))))));
    g.setColour(juce::Colour(static_cast<juce::uint32>(int(ckBdg->getTheme().getProperty("backgroundSidebar", static_cast<int>(ChorusKit_BackgroundSidebarColor))))));
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
