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

// https://www.earlevel.com/main/2012/12/15/a-one-pole-filter/

class OnePoleFilter
{
public:
	OnePoleFilter() : a0(1.0f), b1(0.0f), z1(0.0f) {}
	OnePoleFilter(float freq) : z1(0.0f) { }
	~OnePoleFilter() {};

	void setSampleRate(float sr) {
		sampleRate = sr;
	}

	void setFrequency(float freq) {
		b1 = exp(-2.0f * M_PI * (freq / sampleRate));
		a0 = 1.0f - b1;
	}

	void prepare(float sr, float f) {
		setSampleRate(sr);
		setFrequency(f);
	}


	float process(float in) {
		z1 = in * a0 + z1 * b1;
		return z1;
	}

	float updateAndProcess(float freq, float in) {
		setFrequency(freq);
		return process(in);
	}

protected:
	float a0{ 1.0 }, b1{ 0.0 }, z1{ 0.0 };
	float sampleRate{ DEFAULT_SR };
};


#undef DEFAULT_SR
#undef M_PI