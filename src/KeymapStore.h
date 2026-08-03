#pragma once

#include <juce_core/juce_core.h>
#include "Settings.h"

struct KeymapLoadResult
{
    juce::String json;        // 有効なJSONテキスト(カスタム or 同梱デフォルト)
    juce::String customPath;  // 設定中のカスタムファイルパス(空=未指定)
    bool loadError = false;   // カスタムファイルの読み込みに失敗しフォールバックした
};

// keymap.json の読み込み: 設定でカスタムパスが指定されていればそちらを優先し、
// 無指定・読込失敗時は同梱デフォルト(BinaryData)を返す
class KeymapStore
{
public:
    explicit KeymapStore (Settings& settingsToUse) : settings (settingsToUse) {}

    KeymapLoadResult load() const;

private:
    static juce::String defaultJson();

    Settings& settings;
};
