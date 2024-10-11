#pragma once

#include <JuceHeader.h>

class PresetManager : juce::ValueTree::Listener
{
public:
    const juce::File defaultDir{ juce::File::getSpecialLocation(
        PRESET_FOLDER)
        .getChildFile(ProjectInfo::companyName)
        .getChildFile(ProjectInfo::projectName) };


    const juce::String ext{ "spchili" };
    const juce::String presetNameProperty{ "presetName" };

    PresetManager(juce::AudioProcessorValueTreeState& state) : apvts(state) {
        if (!defaultDir.exists()) {
            const auto hasCreatedDirectory = defaultDir.createDirectory();
            if (hasCreatedDirectory.failed()) {
                DBG("Could not create directory: " + hasCreatedDirectory.getErrorMessage());
                jassertfalse;
            }
        }

        apvts.state.addListener(this);
        current = apvts.state.getPropertyAsValue("presetName", nullptr).toString();

        // The program will crash if we get a preset from a saved state that has been deleted.
        // In that case, just assing current to an empty preset.
        auto srcFile = defaultDir.getChildFile(current + "." + ext);
        if (!srcFile.existsAsFile()) {
            current = "";
        }
    }

    void savePreset(const juce::String& name) {
        if (name.isEmpty()) return;

        current = name;
        const auto xml = apvts.copyState().createXml();
        auto destFile = defaultDir.getChildFile(name + "." + ext);
        if (!xml->writeTo(destFile)) {
            DBG("Could not create preset: " + destFile.getFullPathName());
            jassertfalse;
        }

    }

    void deletePreset(const juce::String& name) {
        if (name.isEmpty()) return;

        auto srcFile = defaultDir.getChildFile(name + "." + ext);
        if (!srcFile.existsAsFile()) {
            DBG("Could not open file: " + srcFile.getFullPathName());
            jassertfalse;
        }
        if (!srcFile.deleteFile()) {
            DBG("Could not delete file: " + srcFile.getFullPathName());
            jassertfalse;
        }
        current = "";
    }
    void loadPreset(const juce::String& name) {
        if (name.isEmpty()) return;

        auto srcFile = defaultDir.getChildFile(name + "." + ext);
        if (!srcFile.existsAsFile()) {
            DBG("Could not open file: " + srcFile.getFullPathName());
            jassertfalse;
        }

        juce::XmlDocument xmlDocument{ srcFile };
        const auto newValueTree = juce::ValueTree::fromXml(*xmlDocument.getDocumentElement());

        apvts.replaceState(newValueTree);
        current = name;
    }
    int next() {
        const auto allPresets = getPresetList();
        if (allPresets.isEmpty()) return -1;
        const auto currentIndex = allPresets.indexOf(current);
        const auto nextIndex = (currentIndex + 1) % allPresets.size();
        loadPreset(allPresets.getReference(nextIndex));
        return nextIndex;
    }
    int prev() {
        const auto allPresets = getPresetList();
        if (allPresets.isEmpty()) return -1;
        const auto currentIndex = allPresets.indexOf(current);
        const auto prevIndex = currentIndex - 1 < 0 ? allPresets.size() - 1 : currentIndex - 1;
        loadPreset(allPresets.getReference(prevIndex));
        return prevIndex;
    }

    juce::StringArray getPresetList() const {
        juce::StringArray presets;
        const auto files = defaultDir.findChildFiles(juce::File::TypesOfFileToFind::findFiles, false, "*." + ext);
        for (const auto& file : files) {
            presets.add(file.getFileNameWithoutExtension());
        }
        return presets;
    }
    juce::String getCurrent() const {
        return current;
    }

private:
    void valueTreeRedirected(juce::ValueTree& tree) override {
        current = tree.getPropertyAsValue("presetName", nullptr).toString();
        auto srcFile = defaultDir.getChildFile(current + "." + ext);
        if (!srcFile.existsAsFile()) {
            current = "";
        }
    }

    juce::AudioProcessorValueTreeState& apvts;
    juce::String current;
};
