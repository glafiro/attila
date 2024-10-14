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
#include "APVTSParameter.h"
#include "PresetManager.h"
#include "SpectrumAnalyzer.h"

#define MIN_DB  -60.0f
#define MAX_DB  6.0f
#define MAX_KNEE 24.0f


enum ParameterNames{
    INPUT_GAIN_1, OUTPUT_GAIN_1, 
    DRIVE_1, KNEE_1,
    BIT_1,
    BYPASS_1,

    INPUT_GAIN_2, OUTPUT_GAIN_2, 
    DRIVE_2, KNEE_2,
    BIT_2,
    BYPASS_2,

    INPUT_GAIN_3, OUTPUT_GAIN_3, 
    DRIVE_3, KNEE_3,
    BIT_3,
    BYPASS_3,

    MIX,
    INPUT_GLOBAL, OUTPUT_GLOBAL,
    BYPASS,

    LOW_MID_CUT, MID_HIGH_CUT,
    PARAMETER_COUNT
};

static std::array<std::unique_ptr<IAPVTSParameter>, ParameterNames::PARAMETER_COUNT> apvtsParameters{
    std::make_unique<APVTSParameterFloat> ("inputGain1",      "in gain",      0.0f),
    std::make_unique<APVTSParameterFloat> ("outputGain1",     "out gain",     0.0f),
    std::make_unique<APVTSParameterFloat> ("drive1",          "drive",        0.0f),
    std::make_unique<APVTSParameterFloat> ("knee1",           "knee",         1.0f),
    std::make_unique<APVTSParameterInt>   ("bit1",            "bit",          32),
    std::make_unique<APVTSParameterBool>  ("bypass1",         "LOW",          false),
    std::make_unique<APVTSParameterFloat> ("inputGain2",      "in gain",      0.0f),
    std::make_unique<APVTSParameterFloat> ("outputGain2",     "out gain",     0.0f),
    std::make_unique<APVTSParameterFloat> ("drive2",          "drive",        0.0f),
    std::make_unique<APVTSParameterFloat> ("knee2",           "knee",         1.0f),
    std::make_unique<APVTSParameterInt>   ("bit2",            "bit",          32),
    std::make_unique<APVTSParameterBool>  ("bypass2",         "MID",          false),
    std::make_unique<APVTSParameterFloat> ("inputGain3",      "in gain",      0.0f),
    std::make_unique<APVTSParameterFloat> ("outputGain3",     "out gain",     0.0f),
    std::make_unique<APVTSParameterFloat> ("drive3",          "drive",        0.0f),
    std::make_unique<APVTSParameterFloat> ("knee3",           "knee",         1.0f),
    std::make_unique<APVTSParameterInt>   ("bit3",            "bit",          32),
    std::make_unique<APVTSParameterBool>  ("bypass3",         "HIGH",         false),
    std::make_unique<APVTSParameterFloat> ("mix",             "mix",          100.0f),
    std::make_unique<APVTSParameterFloat> ("inputGain",       "input",        0.0f),
    std::make_unique<APVTSParameterFloat> ("outputGain",      "output",       0.0f),
    std::make_unique<APVTSParameterBool>  ("bypass",          "bypass",       false),
    std::make_unique<APVTSParameterFloat> ("lowMidCut",       "Low/Mid Cut",  440.0f),
    std::make_unique<APVTSParameterFloat> ("midHighCut",      "Mid/high Cut", 5000.0f)
};

class AttilaAudioProcessor  : 
    public juce::AudioProcessor,
    public ValueTree::Listener
{
public:
    //==============================================================================
    AttilaAudioProcessor();
    ~AttilaAudioProcessor() override;

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

    AudioProcessorValueTreeState     apvts;
    PresetManager& getPresetManager() { return *presetManager; }
    SpectrumAnalyzer spectrumAnalyzer;

    // Used for meters
    std::atomic<float> levelL, levelR;


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

    size_t oversampleFactor = 2;
    dsp::Oversampling<float> oversampling{ 2, oversampleFactor, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, false};

    std::unique_ptr<PresetManager>presetManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttilaAudioProcessor)
};
