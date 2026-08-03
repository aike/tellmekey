#pragma once

#include <juce_data_structures/juce_data_structures.h>

// グローバル設定 (%APPDATA%/TellMeKey/TellMeKey.xml 等) の読み書き
class Settings
{
public:
    Settings();

    juce::String getCustomKeymapPath() const;
    void setCustomKeymapPath (const juce::String& path);

private:
    std::unique_ptr<juce::PropertiesFile> props;

    JUCE_DECLARE_NON_COPYABLE (Settings)
};
