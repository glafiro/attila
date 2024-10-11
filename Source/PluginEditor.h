#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GuiComponents.h"

#define WIDTH 1024.0f
#define HEIGHT 768.0f

#define KNOB_W 70
#define KNOB_H 68

//==============================================================================
/**
*/
class AttilaAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    AttilaAudioProcessorEditor (AttilaAudioProcessor&);
    ~AttilaAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AttilaAudioProcessor& audioProcessor;

    // GUI Components
    Knob lowInputGain   { apvtsParameters[INPUT_GAIN_1].get(),  KNOB_W, KNOB_H, audioProcessor.apvts};
    Knob lowOutputGain { apvtsParameters[OUTPUT_GAIN_1].get(),  KNOB_W, KNOB_H, audioProcessor.apvts};
    Knob lowDrive       { apvtsParameters[DRIVE_1].get(),       KNOB_W, KNOB_H, audioProcessor.apvts};
    Knob lowKnee        { apvtsParameters[KNEE_1].get(),        KNOB_W, KNOB_H, audioProcessor.apvts};
    Knob lowBit         { apvtsParameters[BIT_1].get(),         KNOB_W, KNOB_H, audioProcessor.apvts};
    
    Knob midInputGain   { apvtsParameters[INPUT_GAIN_2].get(),  KNOB_W, KNOB_H, audioProcessor.apvts};
    Knob midOutputGain { apvtsParameters[OUTPUT_GAIN_2].get(),  KNOB_W, KNOB_H, audioProcessor.apvts};
    Knob midDrive       { apvtsParameters[DRIVE_2].get(),       KNOB_W, KNOB_H, audioProcessor.apvts};
    Knob midKnee        { apvtsParameters[KNEE_2].get(),        KNOB_W, KNOB_H, audioProcessor.apvts};
    Knob midBit         { apvtsParameters[BIT_2].get(),         KNOB_W, KNOB_H, audioProcessor.apvts};    
    
    Knob highInputGain   { apvtsParameters[INPUT_GAIN_3].get(), KNOB_W, KNOB_H, audioProcessor.apvts};
    Knob highOutputGain { apvtsParameters[OUTPUT_GAIN_3].get(), KNOB_W, KNOB_H, audioProcessor.apvts};
    Knob highDrive       { apvtsParameters[DRIVE_3].get(),      KNOB_W, KNOB_H, audioProcessor.apvts};
    Knob highKnee        { apvtsParameters[KNEE_3].get(),       KNOB_W, KNOB_H, audioProcessor.apvts};
    Knob highBit         { apvtsParameters[BIT_3].get(),        KNOB_W, KNOB_H, audioProcessor.apvts};    
    
    Knob globalInputGain   { apvtsParameters[INPUT_GLOBAL].get(),   KNOB_W, KNOB_H, audioProcessor.apvts};
    Knob globalOutputGain  { apvtsParameters[OUTPUT_GLOBAL].get(),  KNOB_W, KNOB_H, audioProcessor.apvts};
    Knob mix               { apvtsParameters[MIX].get(),            KNOB_W, KNOB_H, audioProcessor.apvts};

    PresetMenu presetMenu{ {0, 0, getLocalBounds().getWidth() * 0.5f, getLocalBounds().getHeight() * 0.06f}, audioProcessor.getPresetManager()};
    GroupComponent lowBandGroup, midBandGroup, highBandGroup, globalGroup, presetManagerGroup, spectrumAnalyzerGroup;

    ToggleButton lowBypass, midBypass, highBypass, globalBypass;
    Label        lowBypassLabel, midBypassLabel, highBypassLabel, globalBypassLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttilaAudioProcessorEditor)
};
