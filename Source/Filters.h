#pragma once

#include <JuceHeader.h>

#define M_PI 3.14159265358979323846
#define DEFAULT_SR 44100.0f

template <typename T>
struct LRFilter
{
	juce::dsp::LinkwitzRileyFilter<float> filter;
	int type{ 0 };
	T frequency{ 0.0f };

	float sampleRate{ DEFAULT_SR };
	float blockSize{ 0.0f };
	int   nChannels{ 1 };

	LRFilter() {
		filter.setType(juce::dsp::LinkwitzRileyFilterType::allpass);
	}

	void setFrequency(T f) {
		frequency = f;
		filter.setCutoffFrequency(frequency);
	}

	void prepare(float sr, float numSamples, int numChannels) {
		sampleRate = sr;
		blockSize = numSamples;
		nChannels = numChannels;
		juce::dsp::ProcessSpec spec;
		spec.sampleRate = sampleRate;
		spec.maximumBlockSize = blockSize;
		spec.numChannels = nChannels;

		filter.prepare(spec);
	}

	void processSample(int ch, T sample, T& sampleOutLow, T& sampleOutHigh) {
		filter.processSample(ch, sample, sampleOutLow, sampleOutHigh);
	}
};

#undef DEFAULT_SR
#undef M_PI