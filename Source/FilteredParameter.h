
#pragma once

#define DEFAULT_FILTER_FREQ 0.5f
#define DEFAULT_SR          44100.0f
#define M_PI 3.14159265358979323846
#include <cmath>

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
		b1 = exp(-2.0f * M_PI * (freq));
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

class FilteredParameter
{
    OnePoleFilter filter{ DEFAULT_FILTER_FREQ };
    float sampleRate{ DEFAULT_SR };
    float value{ 0.0f };
    float frequency{ DEFAULT_FILTER_FREQ };

public:

    FilteredParameter(float sr = DEFAULT_SR) : sampleRate(sr) {
    }

    void prepare(float sr) {
        sampleRate = sr;
        filter.prepare(sr, DEFAULT_FILTER_FREQ / sr);
    }

    void update(float v) {
        value = v;
    }

    // Filter then return current value
    float next() {
        return filter.process(value);
    }

    // Just return current value
    float read() {
        return value;
    }

};

#undef DEFAULT_SR
#undef M_PI