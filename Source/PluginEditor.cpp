#pragma once

#include "PluginEditor.h"

using Track = Grid::TrackInfo;
using Fr = Grid::Fr;

//==============================================================================
AttilaAudioProcessorEditor::AttilaAudioProcessorEditor (AttilaAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    lowBandGroup.setText("LOW");
    lowBandGroup.setTextLabelPosition(Justification::horizontallyCentred);
    lowBandGroup.addAndMakeVisible(lowInputGain);
    lowBandGroup.addAndMakeVisible(lowOutputGain);
    lowBandGroup.addAndMakeVisible(lowDrive);
    lowBandGroup.addAndMakeVisible(lowKnee);
    lowBandGroup.addAndMakeVisible(lowBit);
    
    midBandGroup.setText("MID");
    midBandGroup.setTextLabelPosition(Justification::horizontallyCentred);
    midBandGroup.addAndMakeVisible(midInputGain);
    midBandGroup.addAndMakeVisible(midOutputGain);
    midBandGroup.addAndMakeVisible(midDrive);
    midBandGroup.addAndMakeVisible(midKnee);
    midBandGroup.addAndMakeVisible(midBit);
    highBandGroup.setText("HIGH");
    highBandGroup.setTextLabelPosition(Justification::horizontallyCentred);
    highBandGroup.addAndMakeVisible(highInputGain);
    highBandGroup.addAndMakeVisible(highOutputGain);
    highBandGroup.addAndMakeVisible(highDrive);
    highBandGroup.addAndMakeVisible(highKnee);
    highBandGroup.addAndMakeVisible(highBit);

    globalGroup.setText("GLOBAL");
    globalGroup.setTextLabelPosition(Justification::horizontallyCentred);
    globalGroup.addAndMakeVisible(globalInputGain);
    globalGroup.addAndMakeVisible(globalOutputGain);
    globalGroup.addAndMakeVisible(mix);

    addAndMakeVisible(lowBandGroup);
    addAndMakeVisible(midBandGroup);
    addAndMakeVisible(highBandGroup);
    addAndMakeVisible(globalGroup);
    addAndMakeVisible(globalBypass);
    addAndMakeVisible(presetMenu);
    
    addAndMakeVisible(lowBypass);
    addAndMakeVisible(midBypass);
    addAndMakeVisible(highBypass);

    lowBandGroup.getProperties().set("type", Band::LOW);
    lowBandGroup.setLookAndFeel(&groupComponentLookAndFeel);
    midBandGroup.getProperties().set("type", Band::MID);
    midBandGroup.setLookAndFeel(&groupComponentLookAndFeel);
    highBandGroup.getProperties().set("type", Band::HIGH);
    highBandGroup.setLookAndFeel(&groupComponentLookAndFeel);
    globalGroup.setLookAndFeel(&groupComponentLookAndFeel);

    logoImg = ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize);

    setSize (screenWidth, screenHeight);
}

AttilaAudioProcessorEditor::~AttilaAudioProcessorEditor()
{
    lowBandGroup.setLookAndFeel(nullptr);
    midBandGroup.setLookAndFeel(nullptr);
    highBandGroup.setLookAndFeel(nullptr);
    globalGroup.setLookAndFeel(nullptr);
}

//==============================================================================
void AttilaAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colors::veryDarkGrey);

    float logoHeight = screenHeight * 0.04f;
    float logoWidth = logoHeight * 7.58f;
    float margin = screenHeight * 0.01f;
    float xOffset = screenWidth - logoWidth - margin;
    float yOffset = margin;
    auto bounds = Rectangle<float>{ xOffset, yOffset, logoWidth, logoHeight };
    logo->drawWithin(g, bounds, RectanglePlacement::centred, 1.0f);
}

void AttilaAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(getWidth() * 0.005f);

    auto topRowHeight =    bounds.getHeight() * 0.06f;
    auto midRowHeight =    bounds.getHeight() * 0.52f;
    auto bottomRowHeight = bounds.getHeight() * 0.43f;

    auto bandGroupWidth = bounds.getWidth() * 0.333f;
    auto globalGroupWidth = bounds.getWidth() * 0.2f;
    auto padding = bandGroupWidth * 0.06f;
    auto bandTopHeight = bottomRowHeight * 0.18f;
    auto bandGroupHeight = bottomRowHeight - bandTopHeight;

    auto switchSize = bandTopHeight * 0.5f;

    lowBandGroup.setBounds(bounds.getX(), topRowHeight + midRowHeight, bandGroupWidth, bottomRowHeight);
    midBandGroup.setBounds(lowBandGroup.getX() + bandGroupWidth, topRowHeight + midRowHeight, bandGroupWidth, bottomRowHeight);
    highBandGroup.setBounds(midBandGroup.getX() + bandGroupWidth, topRowHeight + midRowHeight, bandGroupWidth, bottomRowHeight);

    globalGroup.setBounds(bounds.getX(), topRowHeight, globalGroupWidth, midRowHeight);
    presetMenu.setBounds(switchSize * 1.8f, bounds.getY(), bounds.getWidth() * 0.6f, topRowHeight - padding / 2.0f);

    Grid lowBandGrid;
    Grid midBandGrid;
    Grid highBandGrid;
    Grid globalGrid;

    lowBandGrid.templateRows     = { Track(Fr(1)), Track(Fr(1)) };
    lowBandGrid.templateColumns  = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
    midBandGrid.templateRows     = { Track(Fr(1)), Track(Fr(1)) };
    midBandGrid.templateColumns  = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
    highBandGrid.templateRows    = { Track(Fr(1)), Track(Fr(1)) };
    highBandGrid.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
    globalGrid.templateRows      = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1))};
    globalGrid.templateColumns   = { Track(Fr(1)) };

    lowBandGrid.items = {
        GridItem(lowDrive),
        GridItem(lowKnee),
        GridItem(lowBit),
        GridItem(lowInputGain),
        GridItem(lowOutputGain)
    };


    midBandGrid.items = {  
        GridItem(midDrive),
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

    lowBandGrid.performLayout(lowBandGroup.getLocalBounds().
        removeFromBottom(bandGroupHeight).
        removeFromRight(bandGroupWidth - padding));
    midBandGrid.performLayout(midBandGroup.getLocalBounds().
        removeFromBottom(bandGroupHeight).
        removeFromRight(bandGroupWidth - padding));
    highBandGrid.performLayout(highBandGroup.getLocalBounds().
        removeFromBottom(bandGroupHeight).
        removeFromRight(bandGroupWidth - padding));

    globalGrid.performLayout(globalGroup.getLocalBounds().reduced(padding));

    lowBypass.setBounds(padding, topRowHeight + midRowHeight + padding * 0.5f, switchSize, switchSize);
    midBypass.setBounds(padding + bandGroupWidth, topRowHeight + midRowHeight + padding * 0.5f, switchSize, switchSize);
    highBypass.setBounds(padding + bandGroupWidth * 2, topRowHeight + midRowHeight + padding * 0.5f, switchSize, switchSize);
    globalBypass.setBounds(bounds.getX(), 0.0f, switchSize * 1.5f, switchSize * 1.5f);
}
