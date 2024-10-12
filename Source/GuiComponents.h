#pragma once

#include <JuceHeader.h>
#include "PresetManager.h"
#include "LookAndFeel.h"

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