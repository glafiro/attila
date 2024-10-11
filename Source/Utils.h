#pragma once

#include <JuceHeader.h>
#include <cmath>

template<typename T>
inline static void castParameter(AudioProcessorValueTreeState& apvts,
    const ParameterID& id, T& destination)
{
    destination = dynamic_cast<T>(apvts.getParameter(id.getParamID()));
    jassert(destination);
    // parameter does not exist or wrong type
}

inline float linearToDb(float input) {
    return 20.0f * log10f(fabsf(input) + 0.000001f);
}

inline float dbToLinear(float input) {
    return powf(10.0f, input / 20.0f);
}

inline float sign(float x) {
    return x < 0.0f ? -1.0f : 1.0f;
}

// https://www.musicdsp.org/en/latest/Effects/104-variable-hardness-clipping-function.html

inline float fastatan(float x)
{
    return (x / (1.0f + 0.28f * (x * x)));
}