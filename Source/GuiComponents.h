#pragma once

#include <JuceHeader.h>
#include "APVTSParameter.h"
#include "SpectrumAnalyzer.h"
#include "PresetManager.h"
#include "LookAndFeel.h"
#include "Utils.h"

#include <array>
using std::array;

#ifndef MAX_DB 
#define MAX_DB 6.0f
#endif

#ifndef MIN_DB
#define MIN_DB -60.0f
#endif

class Knob : public Component
{
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Knob);

    Label label;
    String labelText;
    int width, height;
    AudioProcessorValueTreeState& state;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> attachment;

public:
    Slider slider;
    Knob(IAPVTSParameter* param, int w, int h, AudioProcessorValueTreeState& apvts, Band type);
    ~Knob();
    void resized();
};

class Switch : public Component
{
private:
    ToggleButton btn;
    int width, height;
    AudioProcessorValueTreeState& state;
    std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> attachment;

    // Since the only use for switches in these plug-in is for bypass buttons,
    // we take a reference to a group (which correspond to a band in the multiband processor)
    // so that we can mute a whole group.
    GroupComponent& group;

public:
    
    Switch(IAPVTSParameter* param, AudioProcessorValueTreeState& apvts, Band type, GroupComponent& gr);
    void buttonClicked();
    void resized() override;
};


class PresetMenu : public Component, public Button::Listener, public ComboBox::Listener
{
    TextButton saveBtn, deleteBtn, nextBtn, prevBtn;
    ComboBox presetList;
    PresetManager presetManager;
    std::unique_ptr<FileChooser> fileChooser;

    void buttonClicked(Button* btn);
    void comboBoxChanged(ComboBox* box) override;
    void createButton(Button& btn, const String& text, PresetBtnType type);
    void loadPresetList();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetMenu);
public:

    Rectangle<float> area;
    PresetMenu(Rectangle<float> a, PresetManager& pm);
    ~PresetMenu();
    void resized() override;

};

// Inspired from Holleman's audio Plug-in book
class LevelMeter : public Component, private Timer
{
private:
    std::atomic<float>& linearLevelL;
    std::atomic<float>& linearLevelR;

    static constexpr float maxdB = MAX_DB;
    static constexpr float mindB = MIN_DB;
    static constexpr float stepdB = 6.0f;
    static constexpr float clampDB = -120.0f;
    static constexpr float clampLevel = 0.000001f;
    static constexpr int refreshRate = 60;

    float maxPos = 0.0f;
    float minPos = 0.0f;
    float dbLevelL{ mindB };
    float dbLevelR{ mindB };

    float padding{};

    float decay = 0.0f;
    float levelL = clampLevel;
    float levelR = clampLevel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)

    void timerCallback() override;
    int getYPosition(float level) const noexcept;
    void updateLevel(float newLevel, float& smooth, float& leveldB) const;
    void drawLevel(juce::Graphics& g, float level, int x, int width);

public:
    LevelMeter(std::atomic<float>& measureL, std::atomic<float>& measureR);
    ~LevelMeter();
    void paint(Graphics& g) override;
    void resized() override;
};

class SpectrumAnalyzerGroup : public Component
{
    AudioProcessorValueTreeState& state;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> freq1Attachment;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> freq2Attachment;
    Slider lowMidSlider;
    Slider midHighSlider;
    SpectrumAnalyzer& spectrumAnalyzer;

    // We need a reference to the drive knobs, so that we can update
    // the visual response in the analyzer. 
    // In the analyzer, the drive will be represented by a horizontal line
    // of the chosen band color, which will map [0..maxDrive] to [bottom..top]
    Knob& driveKnob1;
    Knob& driveKnob2;
    Knob& driveKnob3;
    
public:
    SpectrumAnalyzerGroup(
        IAPVTSParameter* freq1Param, IAPVTSParameter* freq2Param,
        AudioProcessorValueTreeState& apvts, SpectrumAnalyzer& analyzer,
        Knob& k1, Knob& k2, Knob& k3
    );
    void driveChanged();
    
    void lowMidSliderChanged();
    void midHighSliderChanged();

    void resized() override;
};