#pragma once

#include <JuceHeader.h>
#include "PresetManager.h"
#include "LookAndFeel.h"
#include "Utils.h"

#include <array>
using std::array;

class Knob : public Component
{
public:
    Slider slider;

    Knob(IAPVTSParameter* param, int w, int h, AudioProcessorValueTreeState& apvts, Band type) :
        width(w), height(h), state(apvts)
    {
        
        slider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
        slider.setTextBoxStyle(Slider::NoTextBox, false, w, h * 0.186);
        slider.setBounds(0, 0, w, h);
        slider.getProperties().set("type", type);
        addAndMakeVisible(slider);

        label.setText(param->displayValue, NotificationType::dontSendNotification);
        label.setJustificationType(Justification::horizontallyCentred);
        label.setBorderSize(BorderSize<int>(0));
        label.attachToComponent(&slider, false);
        addAndMakeVisible(label);

        setSize(w, h * 2.0f);
        setLookAndFeel(KnobLookAndFeel::get());

        attachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(
            state, param->id.getParamID(), slider
        );
    }

    void resize(int w, int h) {
        width = w;
        height = h;
    }

    ~Knob() {}

    void resized() {
        slider.setBounds(0, label.getHeight(), width, height);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Knob);
    
    Label label;
    String labelText;
    int width, height;
    AudioProcessorValueTreeState& state;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> attachment;
};

class Switch : public Component
{
    GroupComponent& group;
public:
    
    Switch(IAPVTSParameter* param, AudioProcessorValueTreeState& apvts, Band type, GroupComponent& gr) :
        state(apvts), group(gr)
    {
        btn.setBounds(0, 0, 35.0f, 35.0f);

        addAndMakeVisible(btn);

        setSize(35.0f, 35.0f);

        setLookAndFeel(SwitchLookAndFeel::get());
        btn.getProperties().set("type", type);

        attachment = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(
            state, param->id.getParamID(), btn
        );

        btn.onClick = [this]() {buttonClicked(); };
        group.setEnabled(!btn.getToggleState());

    }

    void buttonClicked() {
        group.setEnabled(!btn.getToggleState());
    }

    void resized() {
        auto bounds = getLocalBounds();
        btn.setBounds(bounds);
    }

private:
    ToggleButton btn;
    int width, height;
    AudioProcessorValueTreeState& state;
    std::unique_ptr<AudioProcessorValueTreeState::ButtonAttachment> attachment;
};


class PresetMenu : public Component, public Button::Listener, public ComboBox::Listener
{
    TextButton saveBtn, deleteBtn, nextBtn, prevBtn;
    ComboBox presetList;
    PresetManager presetManager;
    std::unique_ptr<FileChooser> fileChooser;

public:
    Rectangle<float> area;
    
    PresetMenu(Rectangle<float> a, PresetManager& pm) : area(a), presetManager(pm) {

        createButton(saveBtn, "SAVE", PresetBtnType::SAVE);
        createButton(deleteBtn, " DELETE", PresetBtnType::DELETE); // hackiest hack
        createButton(nextBtn, ">", PresetBtnType::NEXT);
        createButton(prevBtn, "<", PresetBtnType::PREV);

        presetList.setTextWhenNothingSelected("No preset selected");
        addAndMakeVisible(presetList);
        presetList.addListener(this);

        loadPresetList();

        setBounds(area.toNearestInt());
        setSize(area.getWidth(), area.getHeight());

        setLookAndFeel(PresetMenuLookAndFeel::get());

    }

    ~PresetMenu() {
        saveBtn.removeListener(this);
        deleteBtn.removeListener(this);
        nextBtn.removeListener(this);
        prevBtn.removeListener(this);
    }

    void resized() override {

        auto bounds = getLocalBounds();

        auto buttonSize = bounds.getHeight();
        auto presetListWidth = bounds.getWidth() * 0.7f;

        deleteBtn.setBounds(bounds.getX(), bounds.getY(), buttonSize, buttonSize);
        prevBtn.setBounds(bounds.getX() + buttonSize, bounds.getY(), buttonSize, buttonSize);
        presetList.setBounds(bounds.getX() + buttonSize * 2, bounds.getY(), presetListWidth, bounds.getHeight());
        nextBtn.setBounds(bounds.getX() + buttonSize * 2 + presetListWidth, bounds.getY(), buttonSize, buttonSize);
        saveBtn.setBounds(bounds.getX() + buttonSize * 3 + presetListWidth, bounds.getY(), buttonSize, buttonSize);
    }

private:
    void buttonClicked(Button* btn) {
        if (btn == &saveBtn) {
            fileChooser = std::make_unique<FileChooser>(
                "Preset name:",
                presetManager.defaultDir,
                "*." + presetManager.ext
            );
            fileChooser->launchAsync(FileBrowserComponent::saveMode, [&](const FileChooser& chooser) {
                const auto result = chooser.getResult();
                presetManager.savePreset(result.getFileNameWithoutExtension());
                loadPresetList();
                }
            );
        }
        if (btn == &nextBtn) {
            const auto idx = presetManager.next();
            presetList.setSelectedItemIndex(idx, dontSendNotification);
        }
        if (btn == &prevBtn) {
            const auto idx = presetManager.prev();
            presetList.setSelectedItemIndex(idx, dontSendNotification);
        }

        if (btn == &deleteBtn) {

            NativeMessageBox::showYesNoCancelBox(
                MessageBoxIconType::QuestionIcon,
                "Delete Confirmation",
                "Are you sure you want to delete this item?",
                nullptr,
                ModalCallbackFunction::create([this](int result) {
                    if (result == 1) {
                        const auto text = presetManager.getCurrent();
                        presetManager.deletePreset(text);
                        loadPresetList();
                    }
                    })
            );
        }
    }

    void comboBoxChanged(ComboBox* box) override {

        if (box == &presetList) {
            presetManager.loadPreset(presetList.getItemText(presetList.getSelectedItemIndex()));
        }
    }

    void createButton(Button& btn, const String& text, PresetBtnType type) {

        btn.setButtonText(text);
        btn.getProperties().set("type", type);
        addAndMakeVisible(btn);
        btn.addListener(this);
    }

    void loadPresetList() {

        presetList.clear(dontSendNotification);
        const auto allPresets = presetManager.getPresetList();
        const auto currentPreset = presetManager.getCurrent();
        presetList.addItemList(presetManager.getPresetList(), 1);
        presetList.setSelectedItemIndex(allPresets.indexOf(currentPreset), dontSendNotification);

    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetMenu);
};

// Inspired from Holleman's audio Plug-in book
class LevelMeter : public Component, private Timer
{
public:
    LevelMeter(std::atomic<float>& measureL, std::atomic<float>& measureR) :
        linearLevelL(measureL), linearLevelR(measureR), dbLevelL(clampDB), dbLevelR(clampDB)
    {
        startTimerHz(refreshRate);
        decay = 1.0f - std::exp(-1.0f / (float(refreshRate) * 0.2f));
    }

    ~LevelMeter() {} 

    void paint(Graphics& g) override {
        auto bounds = getLocalBounds();
        auto padding = getHeight() * 0.02f;

        int lineWidth = bounds.getWidth() * 0.6f;
        int baselineOffset = bounds.getHeight() * 0.02f;
        int meterWidth = lineWidth * 0.4f;

        drawLevel(g, dbLevelL, 0, meterWidth);
        drawLevel(g, dbLevelR, meterWidth + lineWidth * 0.2f, meterWidth);

        int y = getYPosition(0.0f);
        g.setColour(Colors::darkGrey);
        g.fillRect(0, y, lineWidth, int(padding / 2.0f));

    }

    void resized() override {
        auto bounds = getLocalBounds();
        padding = getHeight() * 0.015f;

        maxPos = padding;
        minPos = bounds.getHeight() - padding;
    }
private:
    void timerCallback() override {
        updateLevel(linearLevelL.load(), levelL, dbLevelL);
        updateLevel(linearLevelR.load(), levelR, dbLevelR);

        repaint();
    }

    int getYPosition(float level) const noexcept
    {
        int val = int(std::round(jmap(level, maxdB, mindB, maxPos, minPos)));
        if (val < maxPos) val = maxPos;
        if (val > minPos) val = minPos;
        return val;
    }

    void updateLevel(float newLevel, float& smooth, float& leveldB) const {
        if (newLevel > smooth) smooth = newLevel;
        else smooth += (newLevel - smooth) * decay;
        if (smooth > clampLevel) leveldB = linearToDb(smooth);
        else leveldB = clampDB;
    }

    void drawLevel(juce::Graphics& g, float level, int x, int width)
    {

        ColourGradient gradient{
            Colors::blue, 0.0f, float(getHeight()), Colors::red, 0.0f, 0.0f, false
        };
        gradient.addColour(0.2, Colors::green);
        gradient.addColour(0.8, Colors::yellow);
        gradient.addColour(0.9, Colors::red);
        int y = getYPosition(level);
        int y0 = getYPosition(maxdB);
        g.setColour(Colors::veryDarkGrey);
        g.fillRect(x, y0, width, int(minPos) - y0);
        g.setGradientFill(gradient);
        g.fillRect(x, y, width, int(minPos) - y);
    }

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
    float dbLevelL{mindB};
    float dbLevelR{mindB};

    float padding{};

    float decay = 0.0f;
    float levelL = clampLevel;
    float levelR = clampLevel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};

class SpectrumAnalyzerGroup : public Component
{
    AudioProcessorValueTreeState& state;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> freq1Attachment;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> freq2Attachment;
    Slider lowMidSlider;
    Slider midHighSlider;
    SpectrumAnalyzer& spectrumAnalyzer;

public:
    SpectrumAnalyzerGroup(IAPVTSParameter* freq1Param, IAPVTSParameter* freq2Param, 
        AudioProcessorValueTreeState& apvts, SpectrumAnalyzer& analyzer
    ) :
        state(apvts), spectrumAnalyzer(analyzer)
    {
        lowMidSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
        lowMidSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
        lowMidSlider.getProperties().set("type", FreqKnobBand::LOWMID);
        lowMidSlider.onValueChange = [this]() {lowMidSliderChanged(); };

        addAndMakeVisible(lowMidSlider);
        
        midHighSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
        midHighSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
        midHighSlider.getProperties().set("type", FreqKnobBand::MIDHIGH);
        midHighSlider.onValueChange = [this]() {midHighSliderChanged(); };
        addAndMakeVisible(midHighSlider);

        addAndMakeVisible(spectrumAnalyzer);

        freq1Attachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(
            state, freq1Param->id.getParamID(), lowMidSlider
        );

        freq2Attachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(
            state, freq2Param->id.getParamID(), midHighSlider
        );

        setLookAndFeel(SpectrumAnalyzerGroupLookAndFeel::get());

        setSize(10, 10);
    }

    void lowMidSliderChanged() {
        if (lowMidSlider.getValue() > midHighSlider.getValue()) {
            midHighSlider.setValue(lowMidSlider.getValue());
        }

        spectrumAnalyzer.lowMidCut = lowMidSlider.getValue();
    }
    
    void midHighSliderChanged() {
        if (midHighSlider.getValue() < lowMidSlider.getValue()) {
            lowMidSlider.setValue(midHighSlider.getValue());
        }
        
        spectrumAnalyzer.midHighCut = midHighSlider.getValue();
    }

    void resized() override {
        auto bounds = getLocalBounds();
        float width = bounds.getWidth();
        float height = bounds.getHeight();
        float spectrumHeight = height * 0.865;
        float sliderSize = height - spectrumHeight;
        float textBoxWidth = width * 0.15f;

        spectrumAnalyzer.setBounds(bounds.getX(), bounds.getY(), width, spectrumHeight);

        lowMidSlider.setBounds(bounds.getX() + width * 0.1f, spectrumHeight, sliderSize + textBoxWidth * 2, sliderSize);
        midHighSlider.setBounds(bounds.getX() + width * 0.5f, spectrumHeight, sliderSize + textBoxWidth * 2, sliderSize);
    }
};