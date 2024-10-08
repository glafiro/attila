#pragma once

#include <JuceHeader.h>

struct IAPVTSParameter
{
    juce::ParameterID id{ "" };
    juce::String displayValue{ "" };
    float defaultValue{ 0.0f };

    IAPVTSParameter(const juce::String& stringID = "", const juce::String& val = "", float def = 0.0f)
        : id(stringID, 1), displayValue(val), defaultValue(def) {}

    virtual ~IAPVTSParameter() = default;

    virtual void castParameter(juce::AudioProcessorValueTreeState& apvts) = 0;
    virtual float get() const = 0;
    virtual float getDefault() { return defaultValue; }
};


struct APVTSParameterFloat : public IAPVTSParameter
{
    juce::AudioParameterFloat* paramPointer = nullptr;

    APVTSParameterFloat(const juce::String& stringID = "", const juce::String& val = "", float def = 0.0f)
        : IAPVTSParameter(stringID, val, def)
    {
    }

    void castParameter(juce::AudioProcessorValueTreeState& apvts) override {
        paramPointer = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(id.getParamID()));
        jassert(paramPointer);
    }

    float get() const override {
        return paramPointer->get();
    }
};


struct APVTSParameterInt : public IAPVTSParameter
{
    juce::AudioParameterInt* paramPointer = nullptr;

    APVTSParameterInt(const juce::String& stringID, const juce::String& val, float def)
        : IAPVTSParameter(stringID, val, def)
    {
    }

    void castParameter(juce::AudioProcessorValueTreeState& apvts) override {
        paramPointer = dynamic_cast<juce::AudioParameterInt*>(apvts.getParameter(id.getParamID()));
        jassert(paramPointer);
    }

    float get() const override {
        return static_cast<float>(paramPointer->get());
    }
};

struct APVTSParameterBool : public IAPVTSParameter
{
    juce::AudioParameterBool* paramPointer = nullptr;

    APVTSParameterBool(const juce::String& stringID, const juce::String& val, float def)
        : IAPVTSParameter(stringID, val, def)
    {
    }

    void castParameter(juce::AudioProcessorValueTreeState& apvts) override {
        paramPointer = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(id.getParamID()));
        jassert(paramPointer);
    }

    float get() const override {
        return static_cast<float>(paramPointer->get());
    }
};

struct APVTSParameterChoice : public IAPVTSParameter
{
    juce::AudioParameterChoice* paramPointer = nullptr;

    APVTSParameterChoice(const juce::String& stringID, const juce::String& val, float def)
        : IAPVTSParameter(stringID, val, def)
    {
    }

    void castParameter(juce::AudioProcessorValueTreeState& apvts) override {
        paramPointer = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(id.getParamID()));
        jassert(paramPointer);
    }

    float get() const override {
        return paramPointer->getCurrentChoiceName().getFloatValue();
    }
};

