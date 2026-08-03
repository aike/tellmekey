#include "KeymapStore.h"
#include "BinaryData.h"

juce::String KeymapStore::defaultJson()
{
    return juce::String::fromUTF8 (BinaryData::keymap_json, BinaryData::keymap_jsonSize);
}

KeymapLoadResult KeymapStore::load() const
{
    KeymapLoadResult result;
    result.customPath = settings.getCustomKeymapPath();

    if (result.customPath.isNotEmpty())
    {
        const juce::File file (result.customPath);

        if (file.existsAsFile())
        {
            const auto text = file.loadFileAsString();

            if (juce::JSON::parse (text).isObject())
            {
                result.json = text;
                return result;
            }
        }

        result.loadError = true;
    }

    result.json = defaultJson();
    return result;
}
