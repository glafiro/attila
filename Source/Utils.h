#pragma once

#include <JuceHeader.h>
#include <cmath>

float linearToDb(float input) {
    return 20.0f * log10f(fabsf(input) + 0.000001f);
}

float dbToLinear(float input) {
    return powf(10.0f, input / 20.0f);
}

template<typename T>
inline static void castParameter(AudioProcessorValueTreeState& apvts,
    const ParameterID& id, T& destination)
{
    destination = dynamic_cast<T>(apvts.getParameter(id.getParamID()));
    jassert(destination);
    // parameter does not exist or wrong type
}

// Utility functions
template<typename T>
T lengthToSamples(T sr, T n) noexcept {
    return sr * n * static_cast<T>(0.001);
}

inline int wrapNegative(int value, int maxValue) {
    if (value < 0) return value += maxValue;
    else return value;
}

inline unsigned int nearestPowerOfTwo(int n) {
    return pow(2, ceil(log(n) / log(2)));
}

template<typename T>
T lerp(T a, T b, T f) {
    return a * (1.0 - f) + b * f;
}

template<typename T>
T clamp(T val, T minVal, T maxVal) {
    val = fmin(val, maxVal);
    val = fmax(val, minVal);
    return val;
}

class SlewLimiter
{
public:
    SlewLimiter(float slew = 0.1f)
        : slewRate(slew), currentValue(0.0f), targetValue(0.0f)
    {
    }

    void prepare(float sr) {
        maxDelta = slewRate / sampleRate;
    }

    void setTarget(float target) {
        targetValue = target;
    }

    float getNextValue() {
        float delta = targetValue - currentValue;

        if (std::abs(delta) > maxDelta)  currentValue += maxDelta * (delta > 0 ? 1.0f : -1.0f);
        else currentValue = targetValue;

        return currentValue;
    }

private:
    float slewRate;
    float sampleRate;
    float currentValue;
    float targetValue;
    float maxDelta;
};

float sign(float x) {
    return x < 0.0f ? -1.0f : 1.0f;
}

// https://www.musicdsp.org/en/latest/Effects/104-variable-hardness-clipping-function.html

inline float fastatan(float x)
{
    return (x / (1.0f + 0.28f * (x * x)));
}