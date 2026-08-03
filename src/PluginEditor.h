#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include "Settings.h"
#include "KeymapStore.h"

class TellMeKeyAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit TellMeKeyAudioProcessorEditor (TellMeKeyAudioProcessor&);

    void resized() override;

private:
    juce::WebBrowserComponent::Options makeWebViewOptions();
    std::optional<juce::WebBrowserComponent::Resource> getResource (const juce::String& url) const;
    juce::var makeKeymapVar (const KeymapLoadResult& result) const;

    TellMeKeyAudioProcessor& processorRef;
    Settings settings;
    KeymapStore keymapStore { settings };
    std::unique_ptr<juce::FileChooser> fileChooser;

    // 上記メンバーをラムダで参照するため最後に宣言する(先に破棄される)
    juce::WebBrowserComponent webView;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TellMeKeyAudioProcessorEditor)
};
