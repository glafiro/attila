#pragma once

#include <JuceHeader.h>
#include <array>
using std::array;

enum Band { LOW, MID, HIGH, GLOBAL};

namespace Colors
{	
    const Colour pitchBlack{ 0, 0, 0 };
	const Colour black{43, 40, 40};
	const Colour veryDarkGrey{63, 61, 62};
    const Colour gradientTop{ 70, 70, 70 };
	const Colour darkGrey{87, 84, 85};
    const Colour grey{114, 110, 111};
    const Colour white{ 239, 218, 187 };
	
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
        auto fontSize = bounds.getHeight();
        g.setColour(Colors::white);
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
        Colour dialColor = Colors::white;

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
        g.setColour(Colors::white);
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


class SwitchLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SwitchLookAndFeel() {

    }

    static SwitchLookAndFeel* get() {
        static SwitchLookAndFeel inst;
        return &inst;
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& btn, bool highlighted, bool down) override {
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
            iconColor = state ? Colors::white : Colors::darkGrey;
        }
        icon->replaceColour(Colour{ 0, 0, 0 }, iconColor);
        icon->drawWithin(g, iconBounds.toFloat(), RectanglePlacement::centred, 1.0f);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SwitchLookAndFeel)
};
