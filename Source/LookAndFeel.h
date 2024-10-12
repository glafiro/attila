#pragma once

#include <JuceHeader.h>
#include <array>
using std::array;

enum Band { LOW, MID, HIGH, GLOBAL};
enum PresetBtnType { DELETE, PREV, NEXT, SAVE};

namespace Colors
{	
    const Colour pitchBlack{ 0, 0, 0 };
	const Colour black{43, 40, 40};
	const Colour veryDarkGrey{63, 61, 62};
    const Colour gradientTop{ 70, 70, 70 };
	const Colour darkGrey{87, 84, 85};
    const Colour grey{114, 110, 111};
    const Colour lightGrey{120, 121, 124};
    const Colour cream{ 239, 218, 187 };
	
    const Colour red{212, 64, 9};
    const Colour yellow{229, 179, 74};
    const Colour green{118, 175, 158};
    const Colour blue{95, 140, 176};
}

static array<Colour, 4> primaryColors{ Colors::red, Colors::yellow, Colors::green, Colors::blue };

class GroupComponentLookAndFeel : public LookAndFeel_V4
{
    Font mainFont;
public:

	GroupComponentLookAndFeel(float w, float h) : screenWidth(w), screenHeight(h) {
        mainFont = Font(Typeface::createSystemTypefaceFor(BinaryData::coolvetica_otf, BinaryData::coolvetica_otfSize));
    }

private:
	float screenWidth, screenHeight;

	void drawGroupComponentOutline(Graphics& g, int	w, int h, const String& text, const Justification& just, GroupComponent& group) override {
        int cornerSize = w * 0.03f;

		auto padding = screenWidth * 0.005f;
		auto rectBounds = group.getLocalBounds().reduced(padding);
		DropShadow shadow{ Colors::black, cornerSize, Point{0, 0}};
		shadow.drawForRectangle(g, rectBounds);
		g.setColour(Colors::darkGrey);
		g.fillRoundedRectangle(rectBounds.toFloat(), w * 0.03f);
        
        int type = group.getProperties().getWithDefault("type", Band::GLOBAL);
        bool state = group.isEnabled();

        if (type != Band::GLOBAL) {
            Colour mainColor = primaryColors[static_cast<int>(group.getProperties().getWithDefault("type", Band::GLOBAL))];
            Path topBandPath;
            auto topBandBounds = rectBounds.removeFromTop(h * 0.12f);
            topBandPath.addRoundedRectangle(
                topBandBounds.getX(), topBandBounds.getY(), 
                topBandBounds.getWidth(), topBandBounds.getHeight(), 
                cornerSize, cornerSize, true, true, false, false
            );
            if (state) {
                g.setColour(mainColor);
            }
            else {
                g.setColour(Colors::grey);
            }
            g.fillPath(topBandPath);

            auto fontSize = topBandBounds.getHeight() * 0.7f;
            g.setColour(Colors::veryDarkGrey);
            g.setFont(mainFont.withHeight(fontSize));
            g.drawText(text, topBandBounds.removeFromRight(w * 0.8f), Justification::left, false);
        }


	}
};


class KnobLookAndFeel : public LookAndFeel_V4
{
    float rotaryStart, rotaryEnd;
    Font labelFont, textBoxFont;

public:
    KnobLookAndFeel() {
        // Shorten the ends of the rotary slider
        float pi = MathConstants<float>::pi;
        rotaryStart = 1.25f * pi;
        rotaryEnd = 2.75f * pi;

        labelFont = Font(Typeface::createSystemTypefaceFor(BinaryData::coolvetica_otf, BinaryData::coolvetica_otfSize));
        textBoxFont = Font(Typeface::createSystemTypefaceFor(BinaryData::hack_ttf, BinaryData::hack_ttfSize));
    }

    static KnobLookAndFeel* get() {
        static KnobLookAndFeel inst;
        return &inst;
    }

    void drawLabel(Graphics& g, Label& l ) override {
        auto bounds = l.getLocalBounds();
        auto fontSize = bounds.getHeight() * 0.9f;
        g.setColour(Colors::cream);
        g.setFont(labelFont.withHeight(fontSize));
        g.drawText(l.getText(), bounds, Justification::centred, false);
    }

    void drawRotarySlider(Graphics& g, int x, int y, int w, int h, float pos, float startAngle, float endAngle, Slider& slider) override {

        slider.setRotaryParameters(rotaryStart, rotaryEnd, true);

        // The slider colors are:
        // Red - low band parameters
        // Yellow - mid band parameters
        // Green/Turquoise - high band parameters
        // Blue - global parameters
        Colour mainColor = primaryColors[static_cast<int>(slider.getProperties().getWithDefault("type", Band::GLOBAL))];
        Colour dialColor = Colors::cream;

        auto bounds = slider.getLocalBounds().reduced(w * 0.04f);

        auto center = Point{bounds.getX() + bounds.getWidth() / 2, bounds.getY() + bounds.getWidth() / 2};
        auto radius = bounds.getWidth() / 2.0f;

        // dialRadius is the size of the circle that appears on the knob.
        // bottomLineWidth is the line width of the bottom (dark) arc around the knob.
        // The colored part of the arc is always a multiple of the dark line.
        float dialRadius, bottomLineWidth;
        dialRadius = w * 0.07f;
        auto halfDialRadius = dialRadius / 2.0f;
        bottomLineWidth = w * 0.06f;

        auto knobRadius = radius * 0.75f;
        auto innerKnobRadius = knobRadius * 0.9f;

        // --- Draw the dial
        auto dialBounds = bounds.reduced(dialRadius * 2.0f);
        // innerRadius goes from the center to the end of the dial.
        auto innerRadius = dialBounds.getWidth() / 2.0f - halfDialRadius;
        auto toAngle = startAngle + pos * (endAngle - startAngle);

        // The dial circle
        Point<float> dialPos(
            center.getX() + innerKnobRadius * 0.8f * std::sin(toAngle),
            center.getY() - innerKnobRadius * 0.8f * std::cos(toAngle)
        );


        auto arcRadius = radius * 0.95f;

        // --- Draw the background arc
        Path backgroundArc;
        backgroundArc.addCentredArc(center.x,
            center.y,
            arcRadius,
            arcRadius,
            0.0f,
            startAngle,
            endAngle,
            true);
        auto strokeType = PathStrokeType(
            bottomLineWidth, PathStrokeType::curved, PathStrokeType::rounded);
        g.setColour(Colors::grey);
        g.strokePath(backgroundArc, strokeType);

        // --- Draw the foreground (colored) arc
        float topLineWidth = bottomLineWidth + 1;

        strokeType = PathStrokeType(
            topLineWidth, PathStrokeType::curved, PathStrokeType::rounded);

        if (slider.isEnabled()) {
            Path valueArc;
            valueArc.addCentredArc(center.x,
                center.y,
                arcRadius,
                arcRadius,
                0.0f,
                startAngle,
                toAngle,
                true);
            g.setColour(mainColor);
            g.strokePath(valueArc, strokeType);
        }

        // The knob
        Path knobPath, innerKnobPath;
        int shadowOffset = h * 0.1f;
        DropShadow knobShadow{ Colors::black, static_cast<int>(w * 0.35f), Point{0, shadowOffset} };
        knobPath.addEllipse(center.getX() - knobRadius, center.getY() - knobRadius, knobRadius * 2, knobRadius * 2);
        innerKnobPath.addEllipse(center.getX() - innerKnobRadius, center.getY() - innerKnobRadius, innerKnobRadius * 2, innerKnobRadius * 2);

        auto backGradient = ColourGradient(
            Colors::black, 0.0f, center.getY() + knobRadius,
            Colors::veryDarkGrey, 0.0f, center.getY() - knobRadius , false);
        auto topGradient = ColourGradient(
            Colors::black, center.getX(), center.getY(),
            Colors::gradientTop, 0.0f, center.getY() - knobRadius * 2.0f , true);

        knobShadow.drawForPath(g, knobPath);
        g.setGradientFill(backGradient);
        g.fillPath(knobPath);
        
        g.setGradientFill(topGradient);
        g.fillPath(innerKnobPath);

    
        // The dial circle
        Path knobDialPath;
        knobDialPath.addEllipse(dialPos.getX() - halfDialRadius, dialPos.getY() - halfDialRadius, dialRadius, dialRadius);
        g.setColour(dialColor);
        g.fillPath(knobDialPath);

        float textBoxHeight = bounds.getHeight() * 0.25f;
        auto textBoxBounds = Rectangle<float>(bounds.getX(), bounds.getY() + bounds.getHeight() - textBoxHeight, bounds.getWidth(), textBoxHeight).reduced(0.9f);
        auto textBoxCenter = textBoxBounds.getCentre();

        g.setColour(Colors::gradientTop);
        g.fillRoundedRectangle(textBoxBounds, w * 0.06f);

        auto fontSize = textBoxBounds.getHeight() * 0.75f;
        g.setColour(Colors::cream);
        g.setFont(textBoxFont.withHeight(fontSize));
        g.drawText(slider.getTextFromValue(slider.getValue()), textBoxBounds, Justification::centred, false);

    }

    Slider::SliderLayout getSliderLayout(Slider& slider) override
    {
        Slider::SliderLayout layout;
        auto bounds = slider.getLocalBounds();
        auto sliderBounds = Rectangle<int>(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight() * 0.75f);
        layout.sliderBounds = sliderBounds;
        auto textBoxBounds = Rectangle<int>(bounds.getX(), sliderBounds.getY() + sliderBounds.getHeight(), bounds.getWidth(), bounds.getHeight() * 0.25f);
        layout.textBoxBounds = textBoxBounds;
        return layout;
    }
};


class SwitchLookAndFeel : public LookAndFeel_V4
{
public:
    SwitchLookAndFeel() {

    }

    static SwitchLookAndFeel* get() {
        static SwitchLookAndFeel inst;
        return &inst;
    }

    void drawToggleButton(Graphics& g, ToggleButton& btn, bool highlighted, bool down) override {
        auto bounds = btn.getLocalBounds().reduced(btn.getLocalBounds().getWidth() * 0.05f);
        auto iconBounds = bounds.reduced(bounds.getWidth() * 0.18f);
        auto toggle = btn.getToggleState();
        int type = btn.getProperties().getWithDefault("type", Band::GLOBAL);
        Colour mainColor = primaryColors[type];

        if (type != Band::GLOBAL) {
            g.setColour(Colors::darkGrey);
            Path switchPath;
            switchPath.addEllipse(bounds.toFloat());
            g.fillEllipse(bounds.toFloat());

            int shadowRadius = jmin(1, static_cast<int>(bounds.getWidth() * 0.1f));
            DropShadow shadow{ Colors::black, shadowRadius, Point{0, 0}};
            shadow.drawForPath(g, switchPath);
        }

        // Draw icon
        auto icon = Drawable::createFromImageData(BinaryData::switchIcon_svg, BinaryData::switchIcon_svgSize);
        Colour iconColor;
        bool state = !btn.getToggleState();
        if (highlighted) {
            iconColor = state ? Colour{ 200, 200, 200 } : Colors::grey;
        }
        else {
            iconColor = state ? Colors::cream : Colors::darkGrey;
        }
        icon->replaceColour(Colour{ 0, 0, 0 }, iconColor);
        icon->drawWithin(g, iconBounds.toFloat(), RectanglePlacement::centred, 1.0f);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SwitchLookAndFeel)
};


class PresetMenuLookAndFeel : public LookAndFeel_V4
{
public:
    PresetMenuLookAndFeel() {
        //btnFont = Font(Typeface::createSystemTypefaceFor(BinaryData::arial_narrow_7_ttf, BinaryData::arial_narrow_7_ttfSize));
        //presetFont = Font(Typeface::createSystemTypefaceFor(BinaryData::game_over_ttf, BinaryData::game_over_ttfSize));
        menuFont = Font(Typeface::createSystemTypefaceFor(BinaryData::hack_ttf, BinaryData::hack_ttfSize));

    }

    static PresetMenuLookAndFeel* get() {
        static PresetMenuLookAndFeel inst;
        return &inst;
    }

    void drawButtonBackground(Graphics& g, Button& btn, const Colour& bg, bool highlight, bool down) override {
        auto bounds = btn.getLocalBounds().reduced(btn.getLocalBounds().getHeight() * 0.1f);
        int cornerSize = bounds.getHeight() * 0.16f;
        int shadowRadius = (btn.getLocalBounds().getWidth() - bounds.getWidth()) * 2;

        Path btnPath;
        btnPath.addRoundedRectangle(bounds, cornerSize);
        DropShadow btnShadow{ Colors::black, shadowRadius, {0, 0} };
        btnShadow.drawForPath(g, btnPath);
        g.setColour(highlight ? Colors::cream : Colors::lightGrey);
        g.fillRoundedRectangle(bounds.toFloat(), cornerSize);
    }

    void drawButtonText(Graphics& g, TextButton& btn, bool highlight, bool down) override {
        auto bounds = btn.getLocalBounds().reduced(btn.getLocalBounds().getHeight() * 0.2f);

        auto trashIcon = Drawable::createFromImageData(BinaryData::trashIcon_svg, BinaryData::trashIcon_svgSize);
        auto floppyIcon = Drawable::createFromImageData(BinaryData::floppyIcon_svg, BinaryData::floppyIcon_svgSize);

        int type = btn.getProperties().getWithDefault("type", PresetBtnType::DELETE);

        auto iconColor = highlight ? Colors::grey : Colors::cream;

        if (type == PresetBtnType::DELETE) {
            trashIcon->replaceColour(Colour{ 0, 0, 0 }, iconColor);
            trashIcon->drawWithin(g, bounds.toFloat(), RectanglePlacement::centred, 1.0f);
        }
        else if (type == PresetBtnType::SAVE) {
            floppyIcon->replaceColour(Colour{ 0, 0, 0 }, iconColor);
            floppyIcon->drawWithin(g, bounds.toFloat(), RectanglePlacement::centred, 1.0f);
        }
        else {
            float fontSize = btn.getLocalBounds().getHeight();
            g.setColour(iconColor);
            g.setFont(menuFont.withHeight(fontSize));
            g.drawFittedText(btn.getButtonText(), bounds, juce::Justification::centred, 1);
        }

    }

    Font getComboBoxFont(ComboBox& comboBox) override
    {
        return presetFont;
    }

    void positionComboBoxText(ComboBox&, Label& labelToPosition) override {}

    void drawComboBox(Graphics& g, int w, int h, bool down, int buttonX, int buttonY, int buttonW, int buttonH, ComboBox& box) override {
        const float textAreaWProportion = 0.88f;

        auto bounds = box.getLocalBounds().reduced(box.getLocalBounds().getHeight() * 0.1f);
        const auto textArea = box.getLocalBounds().reduced(box.getLocalBounds().proportionOfHeight(0.3f));
        const auto text = box.getText();

        g.setColour(Colors::lightGrey);
        int cornerSize = bounds.getHeight() * 0.16f;
        g.fillRoundedRectangle(bounds.toFloat(), cornerSize);

        g.setColour(Colors::cream);
        g.setFont(presetFont);
        g.setFont(0.6f * h);
        g.drawText(text, textArea, Justification::left);


        const auto arrowArea = Rectangle<float>(w * textAreaWProportion, h * 0.38f, w * 0.065f, h * 0.25f);
        Path arrow;
        arrow.addTriangle(arrowArea.getX(), arrowArea.getY(),
            arrowArea.getX() + arrowArea.getWidth() / 2.0f, arrowArea.getY() + arrowArea.getHeight(),
            arrowArea.getX() + arrowArea.getWidth(), arrowArea.getY());

        PathStrokeType stroke{ 2.0f };

        g.setColour(Colors::cream);
        g.strokePath(arrow, stroke);
    }

    void drawComboBoxTextWhenNothingSelected(Graphics& g, ComboBox& box, Label&) override {
        const auto textArea = box.getLocalBounds().reduced(box.getLocalBounds().proportionOfHeight(0.3f));
        const auto fontSize = box.getLocalBounds().getHeight() * 0.6f;
        const auto text = box.getTextWhenNothingSelected();
        g.setColour(Colors::darkGrey);
        g.setFont(presetFont);
        g.setFont(fontSize);
        g.drawText(text, textArea, Justification::left);
    }

    void drawPopupMenuBackground(Graphics& g, int width, int height) override
    {
        g.fillAll(Colors::lightGrey);
    }

    void drawPopupMenuItem(Graphics& g, const Rectangle<int>& area, bool isSeparator, bool isActive,
        bool isHighlighted, bool isTicked, bool hasSubMenu, const String& text,
        const String& shortcutKeyText, const Drawable* icon, const Colour* textColour) override
    {

        g.setColour(isTicked ? Colors::cream : Colour{200, 200, 200}); // Text color
        g.setFont(menuFont);
        g.drawFittedText(text, area.reduced(5), Justification::centredLeft, 1);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetMenuLookAndFeel)

        Font btnFont;
    Font presetFont;
    Font menuFont;
};
