/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AttilaAudioProcessor::AttilaAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    apvts(*this, nullptr, "Parameters", createParameterLayout()),
    distortion()
#endif
{
    apvts.state.addListener(this);
    for (auto& param : apvtsParameters) {
        param->castParameter(apvts);
    }
}
AttilaAudioProcessor::~AttilaAudioProcessor()
{
    apvts.state.removeListener(this);
}

//==============================================================================
const juce::String AttilaAudioProcessor::getName() const
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

const juce::String AttilaAudioProcessor::getProgramName (int index)
{
    return {};
}

void AttilaAudioProcessor::changeProgramName (int index, const juce::String& newName)
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
}

void AttilaAudioProcessor::updateDSP()
{
    DBG(apvtsParameters[ParameterNames::TYPE]->get());
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
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
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

void AttilaAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
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

}

//==============================================================================
bool AttilaAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AttilaAudioProcessor::createEditor()
{
    //return new AttilaAudioProcessorEditor (*this);
    return new GenericAudioProcessorEditor(*this);
}


void AttilaAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
//    copyXmlToBinary(*apvts.copyState().createXml(), destData);
}

void AttilaAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    //std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    //if (xml.get() != nullptr && xml->hasTagName(apvts.state.getType())) {
    //    apvts.replaceState(juce::ValueTree::fromXml(*xml));
    //    parametersChanged.store(true);
    //}
}


//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AttilaAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout AttilaAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique <juce::AudioParameterFloat>(
        apvtsParameters[ParameterNames::INPUT_GAIN]->id,
        apvtsParameters[ParameterNames::INPUT_GAIN]->displayValue,
        juce::NormalisableRange<float>{ MIN_DB, MAX_DB, 1.0f },
        apvtsParameters[ParameterNames::INPUT_GAIN]->getDefault()
    ));
    
    layout.add(std::make_unique <juce::AudioParameterFloat>(
        apvtsParameters[ParameterNames::OUTPUT_GAIN]->id,
        apvtsParameters[ParameterNames::OUTPUT_GAIN]->displayValue,
        juce::NormalisableRange<float>{ MIN_DB, MAX_DB, 1.0f },
        apvtsParameters[ParameterNames::OUTPUT_GAIN]->getDefault()
    ));    
    
    layout.add(std::make_unique <juce::AudioParameterFloat>(
        apvtsParameters[ParameterNames::DRIVE]->id,
        apvtsParameters[ParameterNames::DRIVE]->displayValue,
        juce::NormalisableRange<float>{ -36.0f, 36.0f, 0.01f },
        apvtsParameters[ParameterNames::DRIVE]->getDefault()
    ));    
    
    layout.add(std::make_unique <juce::AudioParameterFloat>(
        apvtsParameters[ParameterNames::MIX]->id,
        apvtsParameters[ParameterNames::MIX]->displayValue,
        juce::NormalisableRange<float>{ 0.0f, 100.0f, 0.01f },
        apvtsParameters[ParameterNames::MIX]->getDefault()
    ));
    
    layout.add(std::make_unique <juce::AudioParameterChoice>(
        apvtsParameters[ParameterNames::TYPE]->id,
        apvtsParameters[ParameterNames::TYPE]->displayValue,
        StringArray{"Hard Clip", "Tanh", "Sigmoid", "Fuzz Exponential", "Sine Fold"},
        apvtsParameters[ParameterNames::TYPE]->getDefault()
    ));

    layout.add(std::make_unique <juce::AudioParameterInt>(
        apvtsParameters[ParameterNames::BITCRUSH_BIT]->id,
        apvtsParameters[ParameterNames::BITCRUSH_BIT]->displayValue,
        1, 16,
        apvtsParameters[ParameterNames::BITCRUSH_BIT]->getDefault()
    ));

    layout.add(std::make_unique <juce::AudioParameterBool>(
        apvtsParameters[ParameterNames::BITCRUSH_ON]->id,
        apvtsParameters[ParameterNames::BITCRUSH_ON]->displayValue,
        apvtsParameters[ParameterNames::BITCRUSH_ON]->getDefault()
    ));


    layout.add(std::make_unique <juce::AudioParameterFloat>(
        apvtsParameters[ParameterNames::SINE_FREQ]->id,
        apvtsParameters[ParameterNames::SINE_FREQ]->displayValue,
        juce::NormalisableRange<float>{ 0.1f, 2.0f, 0.0001f },
        apvtsParameters[ParameterNames::SINE_FREQ]->getDefault()
    ));
    return layout;
}

