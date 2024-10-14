#include "GuiComponents.h"


Knob::Knob(IAPVTSParameter* param, int w, int h, AudioProcessorValueTreeState& apvts, Band type) :
    width(w), height(h), state(apvts)
{

    // Create the Slider component. Set TextBox style as "NoTextBox", as
    // it will be manually created later.
    slider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    slider.setTextBoxStyle(Slider::NoTextBox, false, w, h * 0.186);
    slider.setBounds(0, 0, w, h);
    // "type" identifies the frequency band the knob operates on, as 
    // each band has a different color.
    slider.getProperties().set("type", type);
    addAndMakeVisible(slider);

    label.setText(param->displayValue, NotificationType::dontSendNotification);
    label.setJustificationType(Justification::horizontallyCentred);
    label.setBorderSize(BorderSize<int>(0));
    label.attachToComponent(&slider, false);
    addAndMakeVisible(label);

    // Height is double to account for the label. Since width and height are usually
    // passed with the same value, the aspect ratio of this component is effectively 1:2.
    setSize(w, h * 2.0f);
    setLookAndFeel(KnobLookAndFeel::get());

    // Attach GUI component to AudioProcessorValueTreeState
    attachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(
        state, param->id.getParamID(), slider
    );
}

Knob::~Knob() {}

void Knob::resized()  {
    slider.setBounds(0, label.getHeight(), width, height);
}

Switch::Switch(IAPVTSParameter* param, AudioProcessorValueTreeState& apvts, Band type, GroupComponent& gr) :
    state(apvts), group(gr)
{
    btn.getProperties().set("type", type);
    btn.onClick = [this]() {buttonClicked(); };
    addAndMakeVisible(btn);

    setLookAndFeel(SwitchLookAndFeel::get());

    attachment = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment>(
        state, param->id.getParamID(), btn
    );

    // Set initial value for group
    group.setEnabled(!btn.getToggleState());

}

void Switch::buttonClicked() {
    group.setEnabled(!btn.getToggleState());
}

void Switch::resized() {
    auto bounds = getLocalBounds();
    btn.setBounds(bounds);
}

PresetMenu::PresetMenu(Rectangle<float> a, PresetManager& pm) : area(a), presetManager(pm) {

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

PresetMenu::~PresetMenu() {
    saveBtn.removeListener(this);
    deleteBtn.removeListener(this);
    nextBtn.removeListener(this);
    prevBtn.removeListener(this);
}

void PresetMenu::resized() {

    auto bounds = getLocalBounds();

    auto buttonSize = bounds.getHeight();
    auto presetListWidth = bounds.getWidth() * 0.7f;

    deleteBtn.setBounds(bounds.getX(), bounds.getY(), buttonSize, buttonSize);
    prevBtn.setBounds(bounds.getX() + buttonSize, bounds.getY(), buttonSize, buttonSize);
    presetList.setBounds(bounds.getX() + buttonSize * 2, bounds.getY(), presetListWidth, bounds.getHeight());
    nextBtn.setBounds(bounds.getX() + buttonSize * 2 + presetListWidth, bounds.getY(), buttonSize, buttonSize);
    saveBtn.setBounds(bounds.getX() + buttonSize * 3 + presetListWidth, bounds.getY(), buttonSize, buttonSize);
}

void PresetMenu::buttonClicked(Button* btn) {
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

void PresetMenu::comboBoxChanged(ComboBox* box) {

    if (box == &presetList) {
        presetManager.loadPreset(presetList.getItemText(presetList.getSelectedItemIndex()));
    }
}

void PresetMenu::createButton(Button& btn, const String& text, PresetBtnType type) {

    btn.setButtonText(text);
    btn.getProperties().set("type", type);
    addAndMakeVisible(btn);
    btn.addListener(this);
}

void PresetMenu::loadPresetList() {

    presetList.clear(dontSendNotification);
    const auto allPresets = presetManager.getPresetList();
    const auto currentPreset = presetManager.getCurrent();
    presetList.addItemList(presetManager.getPresetList(), 1);
    presetList.setSelectedItemIndex(allPresets.indexOf(currentPreset), dontSendNotification);

}


void LevelMeter::timerCallback() {
    updateLevel(linearLevelL.load(), levelL, dbLevelL);
    updateLevel(linearLevelR.load(), levelR, dbLevelR);

    repaint();
}

// Get the y position based on the db value.
int LevelMeter::getYPosition(float level) const noexcept {
    int val = int(std::round(jmap(level, maxdB, mindB, maxPos, minPos)));
    if (val < maxPos) val = maxPos;
    if (val > minPos) val = minPos;
    return val;
}

// If the new peak is higher than the oldest peak, then we update the
// new value immediately. Otherwise, we use a one-pole filter for a 
// smoother animation.
void LevelMeter::updateLevel(float newLevel, float& smooth, float& leveldB) const {
    if (newLevel > smooth) smooth = newLevel;
    else smooth += (newLevel - smooth) * decay;
    if (smooth > clampLevel) leveldB = linearToDb(smooth);
    else leveldB = clampDB;
}

void LevelMeter::drawLevel(juce::Graphics& g, float level, int x, int width)
{
    // Create color gradient
    ColourGradient gradient{
        Colors::blue, 0.0f, float(getHeight()),
        Colors::red, 0.0f, 0.0f, false
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

LevelMeter::LevelMeter(std::atomic<float>& measureL, std::atomic<float>& measureR) :
    linearLevelL(measureL), linearLevelR(measureR), dbLevelL(clampDB), dbLevelR(clampDB)
{
    startTimerHz(refreshRate);
    decay = 1.0f - std::exp(-1.0f / (float(refreshRate) * 0.2f));
}

LevelMeter::~LevelMeter() {}

void LevelMeter::paint(Graphics& g) {
    auto bounds = getLocalBounds();
    auto padding = getHeight() * 0.02f;

    int lineWidth = bounds.getWidth() * 0.6f;
    int meterWidth = lineWidth * 0.4f;

    drawLevel(g, dbLevelL, 0, meterWidth);
    drawLevel(g, dbLevelR, meterWidth + lineWidth * 0.2f, meterWidth);

    int y = getYPosition(0.0f);
    g.setColour(Colors::darkGrey);
    g.fillRect(0, y, lineWidth, int(padding / 2.0f));
}

void LevelMeter::resized() {
    auto bounds = getLocalBounds();
    padding = getHeight() * 0.015f;

    maxPos = padding;
    minPos = bounds.getHeight() - padding;
}

SpectrumAnalyzerGroup::SpectrumAnalyzerGroup(IAPVTSParameter* freq1Param, IAPVTSParameter* freq2Param,
    AudioProcessorValueTreeState& apvts, SpectrumAnalyzer& analyzer,
    Knob& k1, Knob& k2, Knob& k3
) :
    state(apvts), spectrumAnalyzer(analyzer),
    driveKnob1(k1), driveKnob2(k2), driveKnob3(k3)
{

    // The group also includes two knobs for controlling the bands.
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

    driveKnob1.slider.onValueChange = [this]() {driveChanged(); };
    driveKnob2.slider.onValueChange = [this]() {driveChanged(); };
    driveKnob3.slider.onValueChange = [this]() {driveChanged(); };

    addAndMakeVisible(spectrumAnalyzer);

    freq1Attachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(
        state, freq1Param->id.getParamID(), lowMidSlider
    );

    freq2Attachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(
        state, freq2Param->id.getParamID(), midHighSlider
    );

    setLookAndFeel(SpectrumAnalyzerGroupLookAndFeel::get());
}

void SpectrumAnalyzerGroup::driveChanged() {
    spectrumAnalyzer.updateDriveValues(
        jmap(float(driveKnob1.slider.getValue()), 0.0f, 36.0f, 0.0f, 1.0f),
        jmap(float(driveKnob2.slider.getValue()), 0.0f, 36.0f, 0.0f, 1.0f),
        jmap(float(driveKnob3.slider.getValue()), 0.0f, 36.0f, 0.0f, 1.0f)
    );
}

// Ensure low/mid cut frequency stays below the mid/high cut frequency
// to prevent overlap and unwanted interactions between the knobs.
// If low/mid > mid/high, then changing low/mid will also change mid/high
void SpectrumAnalyzerGroup::lowMidSliderChanged() {
    if (lowMidSlider.getValue() > midHighSlider.getValue()) {
        midHighSlider.setValue(lowMidSlider.getValue());
    }

    spectrumAnalyzer.lowMidCut = lowMidSlider.getValue();
}

// ...and vice versa.
void SpectrumAnalyzerGroup::midHighSliderChanged() {
    if (midHighSlider.getValue() < lowMidSlider.getValue()) {
        lowMidSlider.setValue(midHighSlider.getValue());
    }

    spectrumAnalyzer.midHighCut = midHighSlider.getValue();
}

void SpectrumAnalyzerGroup::resized() {
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
