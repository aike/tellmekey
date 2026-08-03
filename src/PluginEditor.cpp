#include "PluginEditor.h"
#include "HostDetector.h"
#include "BinaryData.h"

TellMeKeyAudioProcessorEditor::TellMeKeyAudioProcessorEditor (TellMeKeyAudioProcessor& p)
    : AudioProcessorEditor (&p),
      processorRef (p),
      webView (makeWebViewOptions())
{
    addAndMakeVisible (webView);
    webView.goToURL (juce::WebBrowserComponent::getResourceProviderRoot());

    // JUCE標準の右下グリップはWebViewの下に隠れて操作できないため使わない。
    // リサイズはWeb UI内のハンドル(setEditorSizeネイティブ関数)と、
    // ホストによるウィンドウ枠ドラッグの2経路で行う。
    setResizable (true, false);
    setResizeLimits (380, 400, 1400, 1800);
    setSize (processorRef.editorWidth.load(), processorRef.editorHeight.load());
}

void TellMeKeyAudioProcessorEditor::resized()
{
    webView.setBounds (getLocalBounds());
    processorRef.editorWidth  = getWidth();
    processorRef.editorHeight = getHeight();
}

juce::var TellMeKeyAudioProcessorEditor::makeKeymapVar (const KeymapLoadResult& result) const
{
    auto* obj = new juce::DynamicObject();
    obj->setProperty ("keymapJson", result.json);
    obj->setProperty ("customKeymapPath", result.customPath);
    obj->setProperty ("keymapLoadError", result.loadError);
    return juce::var (obj);
}

juce::WebBrowserComponent::Options TellMeKeyAudioProcessorEditor::makeWebViewOptions()
{
    auto options =
        juce::WebBrowserComponent::Options {}
            .withNativeIntegrationEnabled()
            .withKeepPageLoadedWhenBrowserIsHidden()
            .withResourceProvider ([this] (const juce::String& url) { return getResource (url); })
            .withNativeFunction ("getInitData",
                [this] (const juce::Array<juce::var>&, juce::WebBrowserComponent::NativeFunctionCompletion complete)
                {
                    auto result = makeKeymapVar (keymapStore.load());

                    if (auto* obj = result.getDynamicObject())
                    {
                        obj->setProperty ("detectedDawId", HostDetector::detectDawId());
                        obj->setProperty ("compareDawId", processorRef.getCompareDawId());
#if JUCE_MAC
                        obj->setProperty ("isMac", true);
#else
                        obj->setProperty ("isMac", false);
#endif
                    }

                    complete (result);
                })
            .withNativeFunction ("setEditorSize",
                [this] (const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion complete)
                {
                    if (args.size() >= 2)
                    {
                        auto w = (int) args[0];
                        auto h = (int) args[1];

                        if (auto* c = getConstrainer())
                        {
                            w = juce::jlimit (c->getMinimumWidth(),  c->getMaximumWidth(),  w);
                            h = juce::jlimit (c->getMinimumHeight(), c->getMaximumHeight(), h);
                        }

                        setSize (w, h);
                    }

                    complete ({});
                })
            .withNativeFunction ("setCompareDaw",
                [this] (const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion complete)
                {
                    if (! args.isEmpty())
                        processorRef.setCompareDawId (args[0].toString());

                    complete ({});
                })
            .withNativeFunction ("reloadKeymap",
                [this] (const juce::Array<juce::var>&, juce::WebBrowserComponent::NativeFunctionCompletion complete)
                {
                    complete (makeKeymapVar (keymapStore.load()));
                })
            .withNativeFunction ("clearKeymapFile",
                [this] (const juce::Array<juce::var>&, juce::WebBrowserComponent::NativeFunctionCompletion complete)
                {
                    settings.setCustomKeymapPath ({});
                    complete (makeKeymapVar (keymapStore.load()));
                })
            .withNativeFunction ("chooseKeymapFile",
                [this] (const juce::Array<juce::var>&, juce::WebBrowserComponent::NativeFunctionCompletion complete)
                {
                    fileChooser = std::make_unique<juce::FileChooser> (
                        juce::String::fromUTF8 (u8"キーマップJSONファイルを選択"), juce::File(), "*.json");

                    fileChooser->launchAsync (
                        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                        [this, complete] (const juce::FileChooser& fc)
                        {
                            const auto file = fc.getResult();

                            if (file.existsAsFile())
                                settings.setCustomKeymapPath (file.getFullPathName());

                            complete (makeKeymapVar (keymapStore.load()));
                        });
                });

#if JUCE_WINDOWS
    auto userDataFolder = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                              .getChildFile ("TellMeKey")
                              .getChildFile ("webview2");
    userDataFolder.createDirectory();

    options = options.withBackend (juce::WebBrowserComponent::Options::Backend::webview2)
                     .withWinWebView2Options (
                         juce::WebBrowserComponent::Options::WinWebView2 {}
                             .withUserDataFolder (userDataFolder));
#endif

    return options;
}

std::optional<juce::WebBrowserComponent::Resource>
TellMeKeyAudioProcessorEditor::getResource (const juce::String& url) const
{
    auto makeResource = [] (const void* data, size_t size, const char* mimeType)
    {
        const auto* bytes = static_cast<const std::byte*> (data);
        return juce::WebBrowserComponent::Resource { std::vector<std::byte> (bytes, bytes + size),
                                                     juce::String (mimeType) };
    };

    if (url == "/" || url == "/index.html")
        return makeResource (BinaryData::index_html, (size_t) BinaryData::index_htmlSize, "text/html");

    if (url == "/style.css")
        return makeResource (BinaryData::style_css, (size_t) BinaryData::style_cssSize, "text/css");

    if (url == "/app.js")
        return makeResource (BinaryData::app_js, (size_t) BinaryData::app_jsSize, "text/javascript");

    return std::nullopt;
}
