#pragma once

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AttilaAudioProcessor::AttilaAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", AudioChannelSet::stereo(), true)
#endif
    ),
    apvts(*this, nullptr, "Parameters", createParameterLayout()),
    distortion()
#endif
{
    if (!apvts.state.isValid()) {
        DBG("Invalid value tree");
        jassertfalse;
    }
    apvts.state.setProperty("presetName", "", nullptr);
    apvts.state.addListener(this);
    presetManager = std::make_unique<PresetManager>(apvts);

    for (auto& param : apvtsParameters) {
        param->castParameter(apvts);
    }
}
AttilaAudioProcessor::~AttilaAudioProcessor()
{
    apvts.state.removeListener(this);
}

//==============================================================================
const String AttilaAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AttilaAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AttilaAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AttilaAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AttilaAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AttilaAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AttilaAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AttilaAudioProcessor::setCurrentProgram (int index)
{
}

const String AttilaAudioProcessor::getProgramName (int index)
{
    return {};
}

void AttilaAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void AttilaAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    int nChannels = getTotalNumInputChannels();

    oversampling.numChannels = getTotalNumInputChannels();
    oversampling.initProcessing(samplesPerBlock);
    oversampling.reset();

    setLatencySamples(oversampling.getLatencyInSamples());  

    distortionParameters.set("sampleRate", sampleRate * (pow(2, oversampleFactor)));
    distortionParameters.set("blockSize", samplesPerBlock);
    distortionParameters.set("nChannels", nChannels);

    for (auto& param : apvtsParameters) {
        distortionParameters.set(param->id.getParamID().toStdString(), param->getDefault());
    }

    distortion.prepare(distortionParameters);

    levelL.store(0.0f);
    levelR.store(0.0f);
}

void AttilaAudioProcessor::updateDSP()
{
    for (auto& param : apvtsParameters) {
        distortionParameters.set(param->id.getParamID().toStdString(), param->get());
    }

    distortion.update(distortionParameters);
}

void AttilaAudioProcessor::releaseResources()
{
    oversampling.reset();  // Make sure you reset oversampling
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AttilaAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void AttilaAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    bool expected = true;

    if (isNonRealtime() || parametersChanged.compare_exchange_strong(expected, false)) {
        updateDSP();
    }

    dsp::AudioBlock<float> block(buffer);
    auto oversampledBlock = oversampling.processSamplesUp(block);

    float* outputBuffers[2] = { nullptr, nullptr };
    outputBuffers[0] = oversampledBlock.getChannelPointer(0);
    if (totalNumOutputChannels > 1) outputBuffers[1] = oversampledBlock.getChannelPointer(1);

    distortion.processBlock(
        outputBuffers,
        buffer.getNumChannels(),
        oversampledBlock.getNumSamples()
    );

    oversampling.processSamplesDown(block);
    
    auto maxL = 0.0f, maxR = 0.0f;

    for (int s = 0; s < buffer.getNumSamples(); ++s) {
        maxL = std::max(maxL, block.getChannelPointer(0)[s]);
        if (buffer.getNumChannels() > 1) {
            maxR = std::max(maxR, block.getChannelPointer(1)[s]);
        }
    }

    levelL.store(maxL);
    levelR.store(maxR);

}

//==============================================================================
bool AttilaAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* AttilaAudioProcessor::createEditor()
{
    return new AttilaAudioProcessorEditor (*this);
    //return new GenericAudioProcessorEditor(*this);
}


void AttilaAudioProcessor::getStateInformation(MemoryBlock& destData)
{
    copyXmlToBinary(*apvts.copyState().createXml(), destData);
}

void AttilaAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml.get() != nullptr && xml->hasTagName(apvts.state.getType())) {
        apvts.replaceState(ValueTree::fromXml(*xml));
        parametersChanged.store(true);
    }
}


//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AttilaAudioProcessor();
}

AudioProcessorValueTreeState::ParameterLayout AttilaAudioProcessor::createParameterLayout()
{
    AudioProcessorValueTreeState::ParameterLayout layout;

    auto dbStringFromValue = [](float value, int) { return String(value) + " dB"; };
    auto truncateDecimals = [](float value, int) { return String(value, 2); };
    auto percentStringFromValue = [](float value, int) { 
        return value > 0.0f ? String(value, 1) + " %" : "OFF";
    };
    auto hzStringFromValue = [](float value, int) { return String(value) + " Hz"; };

    layout.add(std::make_unique <AudioParameterFloat>(
        apvtsParameters[ParameterNames::INPUT_GAIN_1]->id,
        apvtsParameters[ParameterNames::INPUT_GAIN_1]->displayValue,
        NormalisableRange<float>{ MIN_DB, MAX_DB, 0.02f },
        apvtsParameters[ParameterNames::INPUT_GAIN_1]->getDefault(),
        AudioParameterFloatAttributes().withStringFromValueFunction(dbStringFromValue)
    ));
    
    layout.add(std::make_unique <AudioParameterFloat>(
        apvtsParameters[ParameterNames::OUTPUT_GAIN_1]->id,
        apvtsParameters[ParameterNames::OUTPUT_GAIN_1]->displayValue,
        NormalisableRange<float>{ MIN_DB, MAX_DB, 0.02f },
        apvtsParameters[ParameterNames::OUTPUT_GAIN_1]->getDefault(),
        AudioParameterFloatAttributes().withStringFromValueFunction(dbStringFromValue)
    ));
    
    layout.add(std::make_unique <AudioParameterFloat>(
        apvtsParameters[ParameterNames::DRIVE_1]->id,
        apvtsParameters[ParameterNames::DRIVE_1]->displayValue,
        NormalisableRange<float>{ 0.0f, 36.0f, 0.01f },
        apvtsParameters[ParameterNames::DRIVE_1]->getDefault(),
        AudioParameterFloatAttributes().withStringFromValueFunction(dbStringFromValue)
    ));

    layout.add(std::make_unique <AudioParameterFloat>(
        apvtsParameters[ParameterNames::KNEE_1]->id,
        apvtsParameters[ParameterNames::KNEE_1]->displayValue,
        NormalisableRange<float>{ 1.0f, 48.0f, 0.01f },
        apvtsParameters[ParameterNames::KNEE_1]->getDefault(),
        AudioParameterFloatAttributes().withStringFromValueFunction(truncateDecimals)
    ));

    layout.add(std::make_unique <AudioParameterInt>(
        apvtsParameters[ParameterNames::BIT_1]->id,
        apvtsParameters[ParameterNames::BIT_1]->displayValue,
        1, 32,
        apvtsParameters[ParameterNames::BIT_1]->getDefault()
    ));

    layout.add(std::make_unique <AudioParameterBool>(
        apvtsParameters[ParameterNames::BYPASS_1]->id,
        apvtsParameters[ParameterNames::BYPASS_1]->displayValue,
        apvtsParameters[ParameterNames::BYPASS_1]->getDefault()
    ));

    layout.add(std::make_unique <AudioParameterFloat>(
        apvtsParameters[ParameterNames::INPUT_GAIN_2]->id,
        apvtsParameters[ParameterNames::INPUT_GAIN_2]->displayValue,
        NormalisableRange<float>{ MIN_DB, MAX_DB, 1.0f },
        apvtsParameters[ParameterNames::INPUT_GAIN_2]->getDefault(),
        AudioParameterFloatAttributes().withStringFromValueFunction(dbStringFromValue)
    ));

    layout.add(std::make_unique <AudioParameterFloat>(
        apvtsParameters[ParameterNames::OUTPUT_GAIN_2]->id,
        apvtsParameters[ParameterNames::OUTPUT_GAIN_2]->displayValue,
        NormalisableRange<float>{ MIN_DB, MAX_DB, 1.0f },
        apvtsParameters[ParameterNames::OUTPUT_GAIN_2]->getDefault(),
        AudioParameterFloatAttributes().withStringFromValueFunction(dbStringFromValue)
    ));

    layout.add(std::make_unique <AudioParameterFloat>(
        apvtsParameters[ParameterNames::DRIVE_2]->id,
        apvtsParameters[ParameterNames::DRIVE_2]->displayValue,
        NormalisableRange<float>{ 0.0f, 36.0f, 0.01f },
        apvtsParameters[ParameterNames::DRIVE_2]->getDefault(),
        AudioParameterFloatAttributes().withStringFromValueFunction(dbStringFromValue)
    ));

    layout.add(std::make_unique <AudioParameterFloat>(
        apvtsParameters[ParameterNames::KNEE_2]->id,
        apvtsParameters[ParameterNames::KNEE_2]->displayValue,
        NormalisableRange<float>{ 1.0f, 48.0f, 0.001f },
        apvtsParameters[ParameterNames::KNEE_2]->getDefault(),
        AudioParameterFloatAttributes().withStringFromValueFunction(truncateDecimals)

    ));

    layout.add(std::make_unique <AudioParameterInt>(
        apvtsParameters[ParameterNames::BIT_2]->id,
        apvtsParameters[ParameterNames::BIT_2]->displayValue,
        1, 32,
        apvtsParameters[ParameterNames::BIT_2]->getDefault()
    ));

    layout.add(std::make_unique <AudioParameterBool>(
        apvtsParameters[ParameterNames::BYPASS_2]->id,
        apvtsParameters[ParameterNames::BYPASS_2]->displayValue,
        apvtsParameters[ParameterNames::BYPASS_2]->getDefault()
    ));


    layout.add(std::make_unique <AudioParameterFloat>(
        apvtsParameters[ParameterNames::INPUT_GAIN_3]->id,
        apvtsParameters[ParameterNames::INPUT_GAIN_3]->displayValue,
        NormalisableRange<float>{ MIN_DB, MAX_DB, 1.0f },
        apvtsParameters[ParameterNames::INPUT_GAIN_3]->getDefault(),
        AudioParameterFloatAttributes().withStringFromValueFunction(dbStringFromValue)
    ));

    layout.add(std::make_unique <AudioParameterFloat>(
        apvtsParameters[ParameterNames::OUTPUT_GAIN_3]->id,
        apvtsParameters[ParameterNames::OUTPUT_GAIN_3]->displayValue,
        NormalisableRange<float>{ MIN_DB, MAX_DB, 1.0f },
        apvtsParameters[ParameterNames::OUTPUT_GAIN_3]->getDefault(),
        AudioParameterFloatAttributes().withStringFromValueFunction(dbStringFromValue)
    ));

    layout.add(std::make_unique <AudioParameterFloat>(
        apvtsParameters[ParameterNames::DRIVE_3]->id,
        apvtsParameters[ParameterNames::DRIVE_3]->displayValue,
        NormalisableRange<float>{ 0.0f, 36.0f, 0.01f },
        apvtsParameters[ParameterNames::DRIVE_3]->getDefault(),
        AudioParameterFloatAttributes().withStringFromValueFunction(dbStringFromValue)
    ));

    layout.add(std::make_unique <AudioParameterFloat>(
        apvtsParameters[ParameterNames::KNEE_3]->id,
        apvtsParameters[ParameterNames::KNEE_3]->displayValue,
        NormalisableRange<float>{ 1.0f, 48.0f, 0.001f },
        apvtsParameters[ParameterNames::KNEE_3]->getDefault(),
        AudioParameterFloatAttributes().withStringFromValueFunction(truncateDecimals)

    ));

    layout.add(std::make_unique <AudioParameterInt>(
        apvtsParameters[ParameterNames::BIT_3]->id,
        apvtsParameters[ParameterNames::BIT_3]->displayValue,
        1, 32,
        apvtsParameters[ParameterNames::BIT_3]->getDefault()
    ));

    layout.add(std::make_unique <AudioParameterBool>(
        apvtsParameters[ParameterNames::BYPASS_3]->id,
        apvtsParameters[ParameterNames::BYPASS_3]->displayValue,
        apvtsParameters[ParameterNames::BYPASS_3]->getDefault()
    ));
    
    layout.add(std::make_unique <AudioParameterFloat>(
        apvtsParameters[ParameterNames::MIX]->id,
        apvtsParameters[ParameterNames::MIX]->displayValue,
        NormalisableRange<float>{ 0.0f, 100.0f, 0.01f },
        apvtsParameters[ParameterNames::MIX]->getDefault(),
        AudioParameterFloatAttributes().withStringFromValueFunction(percentStringFromValue)
    ));

    layout.add(std::make_unique <AudioParameterFloat>(
        apvtsParameters[ParameterNames::INPUT_GLOBAL]->id,
        apvtsParameters[ParameterNames::INPUT_GLOBAL]->displayValue,
        NormalisableRange<float>{ MIN_DB, MAX_DB, 1.0f },
        apvtsParameters[ParameterNames::INPUT_GLOBAL]->getDefault(),
        AudioParameterFloatAttributes().withStringFromValueFunction(dbStringFromValue)
    ));
    
    layout.add(std::make_unique <AudioParameterFloat>(
        apvtsParameters[ParameterNames::OUTPUT_GLOBAL]->id,
        apvtsParameters[ParameterNames::OUTPUT_GLOBAL]->displayValue,
        NormalisableRange<float>{ MIN_DB, MAX_DB, 1.0f },
        apvtsParameters[ParameterNames::OUTPUT_GLOBAL]->getDefault(),
        AudioParameterFloatAttributes().withStringFromValueFunction(dbStringFromValue)
    ));
    
    layout.add(std::make_unique <AudioParameterBool>(
        apvtsParameters[ParameterNames::BYPASS]->id,
        apvtsParameters[ParameterNames::BYPASS]->displayValue,
        apvtsParameters[ParameterNames::BYPASS]->getDefault()
    ));

    layout.add(std::make_unique <AudioParameterFloat>(
        apvtsParameters[ParameterNames::LOW_MID_CUT]->id,
        apvtsParameters[ParameterNames::LOW_MID_CUT]->displayValue,
        NormalisableRange<float>{ 20.0f, 20000.0f, 1.0f, 0.3f },
        apvtsParameters[ParameterNames::LOW_MID_CUT]->getDefault(),
        AudioParameterFloatAttributes().withStringFromValueFunction(hzStringFromValue)
    ));

    layout.add(std::make_unique <AudioParameterFloat>(
        apvtsParameters[ParameterNames::MID_HIGH_CUT]->id,
        apvtsParameters[ParameterNames::MID_HIGH_CUT]->displayValue,
        NormalisableRange<float>{ 20.0f, 20000.0f, 1.0f, 0.3f },
        apvtsParameters[ParameterNames::MID_HIGH_CUT]->getDefault(),
        AudioParameterFloatAttributes().withStringFromValueFunction(hzStringFromValue)
    ));

    return layout;
}

