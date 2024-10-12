#pragma once

#include <JuceHeader.h>

namespace Colors
{	
	const Colour black{43, 40, 40};
	const Colour veryDarkGrey{63, 61, 62};
	const Colour darkGrey{87, 84, 85};
}

class GroupComponentLookAndFeel : public LookAndFeel_V4
{
public:

	GroupComponentLookAndFeel(float w, float h) : screenWidth(w), screenHeight(h) {}

private:
	float screenWidth, screenHeight;

	void drawGroupComponentOutline(Graphics& g, int	w, int h, const String& text, const Justification& just, GroupComponent& group) override {
		auto padding = screenWidth * 0.005f;
		auto rectBounds = group.getLocalBounds().reduced(padding);
		DropShadow shadow{ Colors::black, static_cast<int>(w * 0.035f), Point{0, 0}};
		shadow.drawForRectangle(g, rectBounds);
		g.setColour(Colors::darkGrey);
		g.fillRoundedRectangle(rectBounds.toFloat(), w * 0.03f);
	}

};