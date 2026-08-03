#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

class TellMeKeyAudioProcessor : public juce::AudioProcessor
{
public:
    TellMeKeyAudioProcessor();

    void prepareToPlay (double, int) override {}
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override                              { return true; }

    const juce::String getName() const override                  { return JucePlugin_Name; }
    bool acceptsMidi() const override                            { return false; }
    bool producesMidi() const override                           { return false; }
    bool isMidiEffect() const override                           { return false; }
    double getTailLengthSeconds() const override                 { return 0.0; }

    int getNumPrograms() override                                { return 1; }
    int getCurrentProgram() override                             { return 0; }
    void setCurrentProgram (int) override                        {}
    const juce::String getProgramName (int) override             { return {}; }
    void changeProgramName (int, const juce::String&) override   {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::String getCompareDawId() const;
    void setCompareDawId (const juce::String& dawId);

    std::atomic<int> editorWidth  { 520 };
    std::atomic<int> editorHeight { 640 };

private:
    mutable juce::CriticalSection stateLock;
    juce::String compareDawId { "none" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TellMeKeyAudioProcessor)
};
