#include "PluginProcessor.h"
#include "PluginEditor.h"

TellMeKeyAudioProcessor::TellMeKeyAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

bool TellMeKeyAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto mainOut = layouts.getMainOutputChannelSet();

    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;

    return mainOut == layouts.getMainInputChannelSet();
}

void TellMeKeyAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // 音声はパススルー。入力より多い出力チャンネルだけクリアする
    for (auto ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());
}

juce::AudioProcessorEditor* TellMeKeyAudioProcessor::createEditor()
{
    return new TellMeKeyAudioProcessorEditor (*this);
}

juce::String TellMeKeyAudioProcessor::getCompareDawId() const
{
    const juce::ScopedLock sl (stateLock);
    return compareDawId;
}

void TellMeKeyAudioProcessor::setCompareDawId (const juce::String& dawId)
{
    const juce::ScopedLock sl (stateLock);
    compareDawId = dawId;
}

void TellMeKeyAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::XmlElement xml ("TellMeKeyState");
    xml.setAttribute ("compareDawId", getCompareDawId());
    xml.setAttribute ("editorWidth", editorWidth.load());
    xml.setAttribute ("editorHeight", editorHeight.load());
    copyXmlToBinary (xml, destData);
}

void TellMeKeyAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (xml->hasTagName ("TellMeKeyState"))
        {
            setCompareDawId (xml->getStringAttribute ("compareDawId", "none"));
            editorWidth  = xml->getIntAttribute ("editorWidth", 520);
            editorHeight = xml->getIntAttribute ("editorHeight", 640);
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TellMeKeyAudioProcessor();
}
