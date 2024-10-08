
#pragma once

#include "Filters.h"

#define DEFAULT_FILTER_FREQ 0.5f
#define DEFAULT_SR          44100.0f

class FilteredParameter
{
    OnePoleFilter filter{ DEFAULT_FILTER_FREQ };
    float sampleRate{ DEFAULT_SR };
    float value{ 0.0f };
    float frequency{ DEFAULT_FILTER_FREQ };

public:

    FilteredParameter(float sr = DEFAULT_SR) : sampleRate(sr) {}

    void prepare(float sr, float v) {
        filter.setSampleRate(sr);
        filter.setFrequency(DEFAULT_FILTER_FREQ);
        value = v;
    }

    // Filter then return current value
    float next() {
        value = filter.process(value);
        return value;
    }

    // Just return current value
    float read() {
        return value;
    }


    void setValue(float v) {
        value = v;
    }
};

#undef DEFAULT_SR