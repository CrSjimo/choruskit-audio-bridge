/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginAudioProcessor.h"

#include <RemoteEditorInterface.h>

#include "PluginAudioProcessorEditor.h"
#include "Bridge.h"

static const size_t MAX_MIDI_SIZE = 1048576;

//==============================================================================
PluginAudioProcessor::PluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
        : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                                 .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
#endif
                                 .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
)
#endif
{
    std::cerr << "PluginAudioProcessor: ctor" << std::endl;
    if (!ckBdg->initialize()) {
        std::cerr << ckBdg->getError() << std::endl;
    } else {
        ckBdg->getRemoteEditorInterface()->addListener(this);
    }
}

PluginAudioProcessor::~PluginAudioProcessor() {
    ckBdg->finalize();
    Bridge::deleteInstance();
}

//==============================================================================
const juce::String PluginAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool PluginAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool PluginAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool PluginAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double PluginAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int PluginAudioProcessor::getNumPrograms() {
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int PluginAudioProcessor::getCurrentProgram() {
    return 0;
}

void PluginAudioProcessor::setCurrentProgram(int index) {
}

const juce::String PluginAudioProcessor::getProgramName(int index) {
    return {};
}

void PluginAudioProcessor::changeProgramName(int index, const juce::String &newName) {
}

//==============================================================================
void PluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    if (ckBdg->getRemoteAudioSource())
        ckBdg->getRemoteAudioSource()->prepareToPlay(samplesPerBlock, sampleRate);
}

void PluginAudioProcessor::releaseResources() {
    if (ckBdg->getRemoteAudioSource())
        ckBdg->getRemoteAudioSource()->releaseResources();
}

#ifndef JucePlugin_PreferredChannelConfigurations

bool PluginAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
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
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

#endif

void PluginAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) {
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    if (ckBdg->getRemoteAudioSource() && ckBdg->getRemoteAudioSource()->processInfo()) {
        auto position = getPlayHead()->getPosition();
        if (!position.hasValue()) {
            ckBdg->m_remoteAudioSource->processInfo()->containsInfo = false;
        } else {
            bool isPlaying = position->getIsPlaying() || position->getIsRecording();
            *ckBdg->m_remoteAudioSource->processInfo() = {
                    true,
                    isPlaying ? (isNonRealtime() ? talcs::RemoteProcessInfo::Playing : talcs::RemoteProcessInfo::RealtimePlaying)
                              : talcs::RemoteProcessInfo::NotPlaying,
                    position->getTimeSignature().hasValue() ? position->getTimeSignature()->numerator : 0,
                    position->getTimeSignature().hasValue() ? position->getTimeSignature()->denominator : 0,
                    position->getBpm().orFallback(0.0),
                    position->getTimeInSamples().orFallback(0),
            };
            auto midiMessagesRawData = &ckBdg->m_remoteAudioSource->processInfo()->midiMessages;
            midiMessagesRawData->size = midiMessages.getNumEvents();
            auto midiMessagesRawDataPointer = reinterpret_cast<char *>(midiMessagesRawData->messages);
            for (const auto &metadata : midiMessages) {
                auto midiMessageRawData = reinterpret_cast<talcs::RemoteMidiMessage *>(midiMessagesRawDataPointer);
                midiMessageRawData->size = metadata.numBytes;
                midiMessageRawData->position = metadata.samplePosition;
                std::copy(metadata.data, metadata.data + metadata.numBytes, midiMessageRawData->data);
                midiMessagesRawDataPointer+= sizeof(talcs::RemoteMidiMessage) + metadata.numBytes - 1;
            }
        }
    }

    if (ckBdg->getRemoteAudioSource())
        ckBdg->getRemoteAudioSource()->getNextAudioBlock(juce::AudioSourceChannelInfo(buffer));
    else
        buffer.clear();
}

//==============================================================================
bool PluginAudioProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *PluginAudioProcessor::createEditor() {
    return new PluginAudioProcessorEditor(*this);
}

//==============================================================================
void PluginAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {
    if (ckBdg->getRemoteEditorInterface()) {
        bool ok;
        auto data = ckBdg->getRemoteEditorInterface()->getDataFromEditor(&ok);
        if (!ok) {
            std::cerr << "Cannot get data from editor." << std::endl;
            // TODO error message
        } else {
            destData = juce::MemoryBlock(data.data(), data.size());
        }
    }
}

void PluginAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {
    if (ckBdg->getRemoteEditorInterface()) {
        ckBdg->getRemoteEditorInterface()->putDataToEditor(std::vector(reinterpret_cast<const char *>(data),
                                                                       reinterpret_cast<const char *>(data) +
                                                                       sizeInBytes));
    }
}

void PluginAudioProcessor::markDirtyRequested() {
    updateHostDisplay(ChangeDetails().withNonParameterStateChanged(true));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
    return new PluginAudioProcessor();
}
