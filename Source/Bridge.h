#ifndef CHORUSKITAUDIOBRIDGE_BRIDGE_H
#define CHORUSKITAUDIOBRIDGE_BRIDGE_H

#include <juce_core/juce_core.h>

#include <RemoteAudioSource.h>

#define ckBdg (Bridge::getInstance())

namespace talcs {
    class RemoteSocket;
    class RemoteAudioSource;
    class RemoteEditorInterface;
}

class PluginAudioProcessor;
class SingleInstanceGuard;

class Bridge {
public:
    ~Bridge();

    bool initialize();
    void finalize();

    void startEditorProgram();

    juce::String getError();

    talcs::RemoteSocket *getRemoteSocket();
    talcs::RemoteAudioSource *getRemoteAudioSource();
    talcs::RemoteEditorInterface *getRemoteEditorInterface();

    JUCE_DECLARE_SINGLETON(Bridge, false)

private:
    friend class PluginAudioProcessor;
    Bridge();
    juce::String m_error;
    juce::String m_editorProgramPath;
    talcs::RemoteSocket *m_remoteSocket = nullptr;
    talcs::RemoteAudioSource *m_remoteAudioSource = nullptr;
    talcs::RemoteEditorInterface *m_remoteEditorInterface = nullptr;

    class : public talcs::RemoteAudioSource::ProcessInfoContext {
        friend class PluginAudioProcessor;
        talcs::RemoteAudioSource::ProcessInfo thisBlockProcessInfo{};
    public:
        talcs::RemoteAudioSource::ProcessInfo getThisBlockProcessInfo() const override {
            return thisBlockProcessInfo;
        }
    } m_bridgeProcessInfoContext = {};

};


#endif //CHORUSKITAUDIOBRIDGE_BRIDGE_H
