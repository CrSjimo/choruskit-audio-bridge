function(ck_add_audio_plugin target)
    set(options ENABLE_ARA)
    set(oneValueArgs VERSION PLUGIN_NAME VENDOR)
    set(multiValueArgs FORMATS)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    find_package(Boost REQUIRED)

    juce_add_plugin(${target}
        VERSION ${FUNC_VERSION}
        COMPANY_NAME ${FUNC_VENDOR}
        IS_ARA_EFFECT ${FUNC_ENABLE_ARA}
        IS_SYNTH TRUE                       # Is this a synth or an effect?
        NEEDS_MIDI_INPUT TRUE               # Does the plugin need midi input?
        NEEDS_MIDI_OUTPUT FALSE              # Does the plugin need midi output?
        PLUGIN_MANUFACTURER_CODE Juce               # A four-character manufacturer id with at least one upper-case character
        PLUGIN_CODE Dem0                            # A unique four-character plugin id with exactly one upper-case character
        # GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case
        FORMATS ${FUNC_FORMATS}                # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
        PRODUCT_NAME ${FUNC_PLUGIN_NAME}
    )
    set_target_properties(${target} PROPERTIES CXX_STANDARD 20)
    target_compile_definitions(${target}
        PUBLIC
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            JUCE_DISPLAY_SPLASH_SCREEN=0
            JUCE_VST3_CAN_REPLACE_VST2=0
    )
    target_link_libraries(${target}
        PRIVATE
        # AudioPluginData           # If we'd created a binary data target, we'd link to it here
        juce::juce_audio_utils
        TalcsRemote
        Boost::boost
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
    )
    file(GLOB _src ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../Source/*)
    target_sources(${target} PRIVATE ${_src})
endfunction()

function(ck_target_plugin_config target)
    set(options)
    set(oneValueArgs
        PLUGIN_EDITOR_NAME
        PLUGIN_CONFIG_PATH
        COLOR_FOREGROUND
        COLOR_FOREGROUND_BORDER
        COLOR_FOREGROUND_LABEL
        COLOR_BACKGROUND_MAIN
        COLOR_BACKGROUND_SIDEBAR
        COLOR_BACKGROUND_MESSAGE
        COLOR_BACKGROUND_ERROR
    )
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    target_compile_definitions(${target}
        PUBLIC
            ChorusKit_PluginEditorName="${FUNC_PLUGIN_EDITOR_NAME}"
            ChorusKit_PluginConfigPath="${FUNC_PLUGIN_CONFIG_PATH}"
            ChorusKit_ForegroundColor=0xff${FUNC_COLOR_FOREGROUND}
            ChorusKit_ForegroundBorderColor=0xff${FUNC_COLOR_FOREGROUND_BORDER}
            ChorusKit_ForegroundLabelColor=0xff${FUNC_COLOR_FOREGROUND_LABEL}
            ChorusKit_BackgroundMainColor=0xff${FUNC_COLOR_BACKGROUND_MAIN}
            ChorusKit_BackgroundSidebarColor=0xff${FUNC_COLOR_BACKGROUND_SIDEBAR}
            ChorusKit_BackgroundMessageColor=0xff${FUNC_COLOR_BACKGROUND_MESSAGE}
            ChorusKit_BackgroundErrorColor=0xff${FUNC_COLOR_BACKGROUND_ERROR}
    )
endfunction()
