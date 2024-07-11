#ifndef CHORUSKITAUDIOBRIDGE_BRIDGE_H
#define CHORUSKITAUDIOBRIDGE_BRIDGE_H

#include <juce_core/juce_core.h>

#include <RemoteAudioSource.h>

#define ckBdg (Bridge::getInstance())

namespace talcs {
    class RemoteEditorInterface;
}

class PluginAudioProcessor;

class Bridge : public talcs::RemoteSocket::Listener {
public:
    ~Bridge();

    bool initialize();
    void finalize();

    void startEditorProgram();

    juce::String getError() const;

    talcs::RemoteSocket *getRemoteSocket() const;
    talcs::RemoteAudioSource *getRemoteAudioSource() const;
    talcs::RemoteEditorInterface *getRemoteEditorInterface() const;

    juce::var getTheme() const;

    void socketStatusChanged(int newStatus, int oldStatus) override;

    JUCE_DECLARE_SINGLETON(Bridge, false)

private:
    friend class PluginAudioProcessor;
    Bridge();
    juce::String m_error;
    juce::String m_editorProgramPath;
    talcs::RemoteSocket *m_remoteSocket = nullptr;
    talcs::RemoteAudioSource *m_remoteAudioSource = nullptr;
    talcs::RemoteEditorInterface *m_remoteEditorInterface = nullptr;
    juce::var m_theme;
    juce::ChildProcess m_editorProcess;

};


#endif //CHORUSKITAUDIOBRIDGE_BRIDGE_H
