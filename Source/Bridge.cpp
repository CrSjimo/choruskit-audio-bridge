#include "Bridge.h"

#include <Boost/process/spawn.hpp>
#include <Boost/filesystem/detail/path_traits.hpp>

#include "RemoteSocket.h"
#include "RemoteAudioSource.h"
#include "RemoteEditorInterface.h"

JUCE_IMPLEMENT_SINGLETON(Bridge)

Bridge::~Bridge() {
    finalize();
    clearSingletonInstance();
}

bool Bridge::initialize() {
    juce::String configPath = getenv(ChorusKit_PluginConfigEnv);
    if (configPath.isEmpty()) {
        m_error = "Environment variable '" + juce::String(ChorusKit_PluginConfigEnv) + "' does not exists";
        return false;
    }
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

    if (!configObj->hasProperty("editor")) {
        m_error = "Config file is missing field: 'editor'";
        return false;
    }
    auto editorProgramPathVar = configObj->getProperty("editor");
    if (!editorProgramPathVar.isString()) {
        m_error = "Field 'editor' should be 'String' in config file";
        return false;
    }
    m_editorProgramPath = editorProgramPathVar.toString();

    if (!configObj->hasProperty("pluginPort")) {
        m_error = "Config file is missing field: 'pluginPort'";
        return false;
    }
    auto pluginPortVar = configObj->getProperty("pluginPort");
    if (!pluginPortVar.isInt() || (int)pluginPortVar < 0 || (int)pluginPortVar > 65535) {
        m_error = "Field 'pluginPort' should be 'Int' and within 0 ~ 65536 in config file";
        return false;
    }
    juce::uint16 pluginPort = (int)pluginPortVar;

    if (!configObj->hasProperty("editorPort")) {
        m_error = "Config file is missing field: 'editorPort'";
        return false;
    }
    auto editorPortVar = configObj->getProperty("editorPort");
    if (!editorPortVar.isInt() || (int)editorPortVar < 0 || (int)editorPortVar > 65535) {
        m_error = "Field 'editorPort' should be 'Int' and within 0 ~ 65536 in config file";
        return false;
    }
    juce::uint16 editorPort = (int)editorPortVar;

    m_remoteSocket = new talcs::RemoteSocket(pluginPort, editorPort);
    if (!m_remoteSocket->startServer()) {
        m_error = "Remote socket cannot start server (port = " + juce::String(pluginPort) + ")";
        finalize();
        return false;
    }
    m_remoteAudioSource = new talcs::RemoteAudioSource(m_remoteSocket, 32, &m_bridgeProcessInfoContext);
    m_remoteEditorInterface = new talcs::RemoteEditorInterface(m_remoteSocket);
    if (!m_remoteSocket->startClient()) {
        m_error = "Remote socket cannot start client (port = " + juce::String(editorPort) + ")";
        finalize();
        return false;
    }
//    startEditorProgram();
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
    boost::process::spawn(m_editorProgramPath.toStdString(), "-vst");
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
