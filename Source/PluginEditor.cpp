#pragma once

#include "PluginEditor.h"

using Track = juce::Grid::TrackInfo;
using Fr = juce::Grid::Fr;

//==============================================================================
AttilaAudioProcessorEditor::AttilaAudioProcessorEditor (AttilaAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    lowBandGroup.setText("LOW");
    lowBandGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    lowBandGroup.addAndMakeVisible(lowInputGain);
    lowBandGroup.addAndMakeVisible(lowOutputGain);
    lowBandGroup.addAndMakeVisible(lowDrive);
    lowBandGroup.addAndMakeVisible(lowKnee);
    lowBandGroup.addAndMakeVisible(lowBit);
    addAndMakeVisible(lowBypass);
    
    midBandGroup.setText("MID");
    midBandGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    midBandGroup.addAndMakeVisible(midInputGain);
    midBandGroup.addAndMakeVisible(midOutputGain);
    midBandGroup.addAndMakeVisible(midDrive);
    midBandGroup.addAndMakeVisible(midKnee);
    midBandGroup.addAndMakeVisible(midBit);
    addAndMakeVisible(midBypass);
    
    highBandGroup.setText("HIGH");
    highBandGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    highBandGroup.addAndMakeVisible(highInputGain);
    highBandGroup.addAndMakeVisible(highOutputGain);
    highBandGroup.addAndMakeVisible(highDrive);
    highBandGroup.addAndMakeVisible(highKnee);
    highBandGroup.addAndMakeVisible(highBit);
    addAndMakeVisible(highBypass);

    globalGroup.setText("GLOBAL");
    globalGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    globalGroup.addAndMakeVisible(globalInputGain);
    globalGroup.addAndMakeVisible(globalOutputGain);
    globalGroup.addAndMakeVisible(mix);

    addAndMakeVisible(lowBandGroup);
    addAndMakeVisible(midBandGroup);
    addAndMakeVisible(highBandGroup);
    addAndMakeVisible(globalGroup);
    addAndMakeVisible(globalBypass);
    addAndMakeVisible(presetMenu);

    setSize (WIDTH, HEIGHT);
}

AttilaAudioProcessorEditor::~AttilaAudioProcessorEditor()
{
}

//==============================================================================
void AttilaAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void AttilaAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    auto topRowHeight =    bounds.getHeight() * 0.06f;
    auto midRowHeight =    bounds.getHeight() * 0.47;
    auto bottomRowHeight = bounds.getHeight() * 0.47;

    auto bandGroupWidth = bounds.getWidth() * 0.333f;
    auto globalGroupWidth = bounds.getWidth() * 0.2f;
    auto padding = bandGroupWidth * 0.06f;
    auto bandTopHeight = bottomRowHeight * 0.20f;
    auto bandGroupHeight = bottomRowHeight - bandTopHeight;

    auto switchSize = topRowHeight;
    auto bandSwitchOffset = bandTopHeight - switchSize;

    lowBandGroup.setBounds(0, topRowHeight + midRowHeight, bandGroupWidth, bottomRowHeight);
    midBandGroup.setBounds(bandGroupWidth, topRowHeight + midRowHeight, bandGroupWidth, bottomRowHeight);
    highBandGroup.setBounds(bandGroupWidth * 2, topRowHeight + midRowHeight, bandGroupWidth, bottomRowHeight);

    globalGroup.setBounds(0, topRowHeight, globalGroupWidth, midRowHeight);
    presetMenu.setBounds(switchSize, 0, bounds.getWidth() * 0.5f, topRowHeight);

    juce::Grid lowBandGrid;
    juce::Grid midBandGrid;
    juce::Grid highBandGrid;
    juce::Grid globalGrid;

    lowBandGrid.templateRows = { Track(Fr(1)), Track(Fr(1)) };
    lowBandGrid.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
    midBandGrid.templateRows = { Track(Fr(1)), Track(Fr(1)) };
    midBandGrid.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
    highBandGrid.templateRows = { Track(Fr(1)), Track(Fr(1)) };
    highBandGrid.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
    globalGrid.templateRows = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1))};
    globalGrid.templateColumns = { Track(Fr(1)) };

    lowBandGrid.items = {
        GridItem(lowDrive),
        GridItem(lowKnee),
        GridItem(lowBit),
        GridItem(lowInputGain),
        GridItem(lowOutputGain)
    };


    midBandGrid.items = {  
        GridItem(midDrive).withJustifySelf(GridItem::JustifySelf::center),
        GridItem(midKnee),
        GridItem(midBit),
        GridItem(midInputGain), 
        GridItem(midOutputGain)
    };    
    
    highBandGrid.items = {  
        GridItem(highDrive),
        GridItem(highKnee),
        GridItem(highBit),
        GridItem(highInputGain), 
        GridItem(highOutputGain)
    };
    

    globalGrid.items = {
        GridItem(globalInputGain),
        GridItem(globalOutputGain),
        GridItem(mix),
    };

    lowBandGrid.performLayout(lowBandGroup.getLocalBounds().removeFromBottom(bandGroupHeight).reduced(padding));
    midBandGrid.performLayout(midBandGroup.getLocalBounds().removeFromBottom(bandGroupHeight).reduced(padding));
    highBandGrid.performLayout(highBandGroup.getLocalBounds().removeFromBottom(bandGroupHeight).reduced(padding));
    globalGrid.performLayout(globalGroup.getLocalBounds().reduced(padding));

    lowBypass.setBounds(padding, topRowHeight + midRowHeight + bandSwitchOffset, switchSize, switchSize);
    midBypass.setBounds(padding+ bandGroupWidth, topRowHeight + midRowHeight + bandSwitchOffset, switchSize, switchSize);
    highBypass.setBounds(padding + bandGroupWidth * 2, topRowHeight + midRowHeight + bandSwitchOffset, switchSize, switchSize);
    globalBypass.setBounds(0, 0, switchSize, switchSize);
    
    
}
