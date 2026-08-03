cmake_minimum_required(VERSION 3.22)

project(TellMeKey VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# WebView2 SDK (NuGetパッケージ) の場所。external/webview2 に同梱している
if(WIN32 AND NOT JUCE_WEBVIEW2_PACKAGE_LOCATION)
    set(JUCE_WEBVIEW2_PACKAGE_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/external/webview2")
endif()

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        9.0.0
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(TellMeKey
    COMPANY_NAME "aikelab"
    PRODUCT_NAME "TellMeKey"
    PLUGIN_MANUFACTURER_CODE aike
    PLUGIN_CODE Tmk1
    FORMATS AU VST3
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    NEEDS_WEB_BROWSER TRUE
    NEEDS_WEBVIEW2 TRUE
    VST3_CATEGORIES "Fx Tools")

target_sources(TellMeKey PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/KeymapStore.cpp
    src/Settings.cpp)

juce_add_binary_data(TellMeKeyAssets SOURCES
    assets/keymap.json
    assets/web/index.html
    assets/web/style.css
    assets/web/app.js)

target_compile_definitions(TellMeKey PUBLIC
    JUCE_WEB_BROWSER=1
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

if(MSVC)
    # WebView2Loader.dll を同梱せずに済む静的リンクモード
    target_compile_definitions(TellMeKey PUBLIC JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1)
    # 日本語コメント入りUTF-8ソースをCP932環境でも正しく解釈させる
    target_compile_options(TellMeKey PRIVATE /utf-8)
endif()

target_link_libraries(TellMeKey
    PRIVATE
        TellMeKeyAssets
        juce::juce_audio_utils
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)
