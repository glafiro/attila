#pragma once

#include <JuceHeader.h>

#include <vector>
#include <array>
#include <unordered_map>
using std::vector;
using std::array;
using std::unordered_map;

#include "DSPParameters.h"
#include "MultibandDistortion.h"
#include "Utils.h"
#include "APVTSParameter.h"

#define MIN_DB  -60.0f
#define MAX_DB  12.0f

enum ParameterNames{INPUT_GAIN, OUTPUT_GAIN, 
    DRIVE, MIX, TYPE, 
    PARAMETER_COUNT
};

static std::array<std::unique_ptr<IAPVTSParameter>, ParameterNames::PARAMETER_COUNT> apvtsParameters{
    std::make_unique<APVTSParameterFloat>("inputGain",      "Input Gain",        0.0f),
    std::make_unique<APVTSParameterFloat>("outputGain",     "Output Gain",       -12.0f),
    std::make_unique<APVTSParameterFloat>("drive",          "Drive",             1.0f),
    std::make_unique<APVTSParameterFloat>("mix",            "Mix",               100.0f),
    std::make_unique<APVTSParameterInt>  ("distortionType", "Distortion Type",   0)
};

class ConanAudioProcessor  : 
    public juce::AudioProcessor,
    public ValueTree::Listener
{
public:
    //==============================================================================
    ConanAudioProcessor();
    ~ConanAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    AudioProcessorValueTreeState apvts;

private:
    //==============================================================================
    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    std::atomic<bool> parametersChanged{ false };

    void valueTreePropertyChanged(ValueTree&, const Identifier&) override {
        parametersChanged.store(true);
    }

    void updateDSP();
    DSPParameters<float> distortionParameters;

    MultibandDistortion distortion;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConanAudioProcessor)
};
