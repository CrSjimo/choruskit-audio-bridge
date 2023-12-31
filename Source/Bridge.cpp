#include "Bridge.h"

#ifdef _WIN32
#include <shlobj.h>
#endif

#include <RemoteSocket.h>
#include <RemoteAudioSource.h>
#include <RemoteEditorInterface.h>

JUCE_IMPLEMENT_SINGLETON(Bridge)

Bridge::Bridge() = default;

Bridge::~Bridge() {
    finalize();
    clearSingletonInstance();
}

static juce::String getConfigPath(juce::StringRef path) {
#ifdef _WIN32
    PWSTR appDataPath;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &appDataPath))) {
        juce::String filePath = appDataPath;
        filePath += ("/" + path);
        CoTaskMemFree(appDataPath);
        return filePath;
    }
    return L"";
#else
    return "~/.config/" + path;
#endif
}

#define FIND_STRING_PROPERTY(name) \
if (!configObj->hasProperty(#name)) { \
    m_error = "Config file is missing field: '" #name "'"; \
    return false; \
} \
auto name##Var = configObj->getProperty(#name); \
    if (!name##Var.isString()) { \
    m_error = "Field '" #name "' should be 'String' in config file"; \
    return false; \
}                                  \

#define FIND_INT_PROPERTY(name, l, r) \
if (!configObj->hasProperty(#name)) { \
    m_error = "Config file is missing field: '" #name "'"; \
    return false; \
} \
auto name##Var = configObj->getProperty(#name); \
if (!name##Var.isInt() || (int)name##Var < l|| (int)name##Var > r) { \
    m_error = "Field '" #name "' should be 'Int' and within " #l " ~ " #r "in config file"; \
    return false; \
}

bool Bridge::initialize() {
    auto configPath = getConfigPath(ChorusKit_PluginConfigPath);
    juce::File configFile(configPath);
    if (!configFile.existsAsFile()) {
        m_error = "Cannot locate config file at '" + configPath + "'";
        return false;
    }
    auto configVar = juce::JSON::parse(configFile);
    if (!configVar.isObject()) {
        m_error = "Cannot parse config file at '" + configPath + "'";
        return false;
    }
    auto configObj = configVar.getDynamicObject();

    //======== editor ========//
    FIND_STRING_PROPERTY(editor)
    m_editorProgramPath = editorVar.toString();

    //======== pluginPort ========//
    FIND_INT_PROPERTY(pluginPort, 0, 65535);
    if (!juce::StreamingSocket().createListener(pluginPortVar, "127.0.0.1")) {
        m_error = "The socket is occupied by another program";
        return false;
    }

    //======== editorPort ========//
    FIND_INT_PROPERTY(editorPort, 0, 65535)

    //======== threadCount ========//
    FIND_INT_PROPERTY(threadCount, 1, 16)

    m_remoteSocket = new talcs::RemoteSocket((int)pluginPortVar, (int)editorPortVar);
    if (!m_remoteSocket->startServer(threadCountVar)) {
        m_error = "Remote socket cannot start server (port = " + juce::String((int)pluginPortVar) + ")";
        finalize();
        return false;
    }
    m_remoteAudioSource = new talcs::RemoteAudioSource(m_remoteSocket, 32, &m_bridgeProcessInfoContext);
    m_remoteEditorInterface = new talcs::RemoteEditorInterface(m_remoteSocket);
    if (!m_remoteSocket->startClient()) {
        m_error = "Remote socket cannot start client (port = " + juce::String((int)editorPortVar) + ")";
        finalize();
        return false;
    }
    startEditorProgram();
    m_error.clear();
    return true;
}

void Bridge::finalize() {
    delete m_remoteAudioSource;
    m_remoteAudioSource = nullptr;
    delete m_remoteEditorInterface;
    m_remoteEditorInterface = nullptr;
    delete m_remoteSocket;
    m_remoteSocket = nullptr;
    m_error.clear();
}

void Bridge::startEditorProgram() {
    juce::File(m_editorProgramPath).startAsProcess("-vst");
}

juce::String Bridge::getError() {
    return m_error;
}

talcs::RemoteSocket *Bridge::getRemoteSocket() {
    return m_remoteSocket;
}

talcs::RemoteAudioSource *Bridge::getRemoteAudioSource() {
    return m_remoteAudioSource;
}

talcs::RemoteEditorInterface *Bridge::getRemoteEditorInterface() {
    return m_remoteEditorInterface;
}
