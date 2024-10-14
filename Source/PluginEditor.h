#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GuiComponents.h"
#include "LookAndFeel.h"

#define BTN_SIZE 15

//==============================================================================
/**
*/
class AttilaAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    AttilaAudioProcessorEditor (AttilaAudioProcessor&);
    ~AttilaAudioProcessorEditor() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    AttilaAudioProcessor& audioProcessor;

    Rectangle<int> screenSize = Desktop::getInstance().getDisplays().getMainDisplay().userArea;
    int screenHeight = screenSize.getHeight() * 0.75f;
    int screenWidth = screenHeight * 1.125f;

    int knobW = screenWidth * 0.089f;
    int knobH = knobW * 1.275;

    // GUI Components
    Knob lowInputGain   { apvtsParameters[INPUT_GAIN_1].get(),  knobW, knobH, audioProcessor.apvts, Band::LOW};
    Knob lowOutputGain { apvtsParameters[OUTPUT_GAIN_1].get(),  knobW, knobH, audioProcessor.apvts, Band::LOW};
    Knob lowDrive       { apvtsParameters[DRIVE_1].get(),       knobW, knobH, audioProcessor.apvts, Band::LOW};
    Knob lowKnee        { apvtsParameters[KNEE_1].get(),        knobW, knobH, audioProcessor.apvts, Band::LOW};
    Knob lowBit         { apvtsParameters[BIT_1].get(),         knobW, knobH, audioProcessor.apvts, Band::LOW};
    
    Knob midInputGain   { apvtsParameters[INPUT_GAIN_2].get(),  knobW, knobH, audioProcessor.apvts, Band::MID};
    Knob midOutputGain { apvtsParameters[OUTPUT_GAIN_2].get(),  knobW, knobH, audioProcessor.apvts, Band::MID};
    Knob midDrive       { apvtsParameters[DRIVE_2].get(),       knobW, knobH, audioProcessor.apvts, Band::MID};
    Knob midKnee        { apvtsParameters[KNEE_2].get(),        knobW, knobH, audioProcessor.apvts, Band::MID};
    Knob midBit         { apvtsParameters[BIT_2].get(),         knobW, knobH, audioProcessor.apvts, Band::MID};    
    
    Knob highInputGain   { apvtsParameters[INPUT_GAIN_3].get(), knobW, knobH, audioProcessor.apvts, Band::HIGH};
    Knob highOutputGain  { apvtsParameters[OUTPUT_GAIN_3].get(),knobW, knobH, audioProcessor.apvts, Band::HIGH};
    Knob highDrive       { apvtsParameters[DRIVE_3].get(),      knobW, knobH, audioProcessor.apvts, Band::HIGH};
    Knob highKnee        { apvtsParameters[KNEE_3].get(),       knobW, knobH, audioProcessor.apvts, Band::HIGH};
    Knob highBit         { apvtsParameters[BIT_3].get(),        knobW, knobH, audioProcessor.apvts, Band::HIGH};    
    
    Knob globalInputGain   { apvtsParameters[INPUT_GLOBAL].get(),   knobW, knobH, audioProcessor.apvts, Band::GLOBAL};
    Knob globalOutputGain  { apvtsParameters[OUTPUT_GLOBAL].get(),  knobW, knobH, audioProcessor.apvts, Band::GLOBAL};
    Knob mix               { apvtsParameters[MIX].get(),            knobW, knobH, audioProcessor.apvts, Band::GLOBAL};

    PresetMenu presetMenu{ {0, 0, getLocalBounds().getWidth() * 0.6f, getLocalBounds().getHeight() * 0.06f}, audioProcessor.getPresetManager()};
    
    GroupComponent lowBandGroup;
    GroupComponent midBandGroup;
    GroupComponent highBandGroup;
    GroupComponent globalGroup;

    GroupComponentLookAndFeel groupComponentLookAndFeel{ static_cast<float>(screenWidth), static_cast<float>(screenHeight) };

    Switch lowBypass    { apvtsParameters[BYPASS_1].get(), audioProcessor.apvts, Band::LOW, lowBandGroup};
    Switch midBypass    { apvtsParameters[BYPASS_2].get(), audioProcessor.apvts, Band::MID, midBandGroup};
    Switch highBypass   { apvtsParameters[BYPASS_3].get(), audioProcessor.apvts, Band::HIGH, highBandGroup};
    Switch globalBypass { apvtsParameters[BYPASS].get(),   audioProcessor.apvts, Band::GLOBAL, globalGroup};

    std::unique_ptr<Drawable> logo = Drawable::createFromImageData(BinaryData::logo_svg, BinaryData::logo_svgSize);

    LevelMeter levelMeter;

    SpectrumAnalyzerGroup analyzerGroup{ 
        apvtsParameters[LOW_MID_CUT].get() , 
        apvtsParameters[MID_HIGH_CUT].get(), 
        audioProcessor.apvts, 
        audioProcessor.spectrumAnalyzer
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttilaAudioProcessorEditor)
};
