#pragma once

#include <JuceHeader.h>
#include "PresetManager.h"

class Knob : public Component
{
public:
    Slider slider;

    Knob(IAPVTSParameter* param, int w, int h, AudioProcessorValueTreeState& apvts) :
        width(w), height(h), state(apvts)
    {
        
        slider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
        slider.setTextBoxStyle(Slider::TextBoxBelow, false, w, h * 0.186);
        slider.setBounds(0, 0, w, h);
        addAndMakeVisible(slider);

        label.setText(param->displayValue, NotificationType::dontSendNotification);
        label.setJustificationType(Justification::horizontallyCentred);
        label.setBorderSize(BorderSize<int>(0));
        label.attachToComponent(&slider, false);
        addAndMakeVisible(label);

        setSize(w, h + label.getHeight());
        //setLookAndFeel(KnobLookAndFeel::get());

        attachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(
            state, param->id.getParamID(), slider
        );
    }

    ~Knob() {}

    void resized() {
        slider.setTopLeftPosition(0, label.getHeight());
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
public:
    Switch(IAPVTSParameter* param, int w, int h, AudioProcessorValueTreeState& apvts, bool hasLabel=false) :
        width(w), height(h), state(apvts)
    {
        btn.setBounds(0, 0, w, h);
        addAndMakeVisible(btn);

        setSize(w, h);

        attachment = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(
            state, param->id.getParamID(), btn
        );
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

        createButton(saveBtn, "SAVE");
        createButton(deleteBtn, " DELETE"); // hackiest hack
        createButton(nextBtn, ">");
        createButton(prevBtn, "<");

        presetList.setTextWhenNothingSelected("No preset selected");
        addAndMakeVisible(presetList);
        presetList.addListener(this);

        loadPresetList();

        setBounds(area.toNearestInt());
        setSize(area.getWidth(), area.getHeight());

        //setLookAndFeel(PresetMenuLookAndFeel::get());

    }

    ~PresetMenu() {
        saveBtn.removeListener(this);
        deleteBtn.removeListener(this);
        nextBtn.removeListener(this);
        prevBtn.removeListener(this);
    }

    void resized() override {

        auto bounds = getLocalBounds();

        auto buttonWidth = bounds.getWidth() * 0.075f;
        auto presetListWidth = bounds.getWidth() * 0.7f;

        deleteBtn.setBounds(bounds.getX(), bounds.getY(), buttonWidth, bounds.getHeight());
        prevBtn.setBounds(bounds.getX() + buttonWidth, bounds.getY(), buttonWidth, bounds.getHeight());
        presetList.setBounds(bounds.getX() + buttonWidth * 2, bounds.getY(), presetListWidth, bounds.getHeight());
        nextBtn.setBounds(bounds.getX() + buttonWidth * 2 + presetListWidth, bounds.getY(), buttonWidth, bounds.getHeight());
        saveBtn.setBounds(bounds.getX() + buttonWidth * 3 + presetListWidth, bounds.getY(), buttonWidth, bounds.getHeight());
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

    void createButton(Button& btn, const String& text) {

        btn.setButtonText(text);
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
