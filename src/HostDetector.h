#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

// 実行中のホストDAWを判別し、keymap.json の daws キーに対応するidを返す
namespace HostDetector
{
    inline juce::String detectDawId()
    {
        const juce::PluginHostType host;

        if (host.isCubase() || host.isNuendo())     return "cubase";
        if (host.isAbletonLive())                   return "ableton_live";
        if (host.isFruityLoops())                   return "fl_studio";

        // 旧バージョンのStudio OneもFender Studio Proとして扱う
        if (host.isFenderStudioPro() || host.isStudioOne())
            return "fender_studio_pro";

        if (host.isLogic() || host.isGarageBand())  return "logic";
        if (host.isProTools())                      return "pro_tools";
        if (host.isReaper())                        return "reaper";
        if (host.isBitwigStudio())                  return "bitwig";

        return "unknown";
    }
}
