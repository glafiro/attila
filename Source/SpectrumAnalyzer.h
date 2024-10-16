#pragma once

#include <JuceHeader.h>
#include "Utils.h"
#include "LookAndFeel.h"

class SpectrumAnalyzer : public Component, private Timer
{
    enum FFTParams {
        FFT_ORDER = 11,
        FFT_SIZE = 1 << FFT_ORDER,
        SCOPE_SIZE = 2048
    };

    dsp::FFT forwardFFT;                      
    dsp::WindowingFunction<float> window;     

    array<float, 2 * FFTParams::FFT_SIZE>  fftData{};
    array<float, 2 * FFTParams::FFT_SIZE>  prev{};
    array<float, FFTParams::SCOPE_SIZE> scopeData{};
    array<float, FFTParams::FFT_SIZE> fifo{};
    int fifoIndex{};
    bool nextFFTBlockReady{ false };

    float decay{};
    int refreshRate{ 60 };
    float sampleRate;

    Path fftPath;
    float drive1{};
    float drive2{};
    float drive3{};

public:
    float lowMidCut{};
    float midHighCut{};


    SpectrumAnalyzer(float sr, float lm, float mh)
        : sampleRate(sr), forwardFFT(FFTParams::FFT_ORDER), window(FFTParams::FFT_SIZE, dsp::WindowingFunction<float>::hann),
        lowMidCut(lm) , midHighCut(mh)
    {
        startTimerHz(refreshRate);
        decay = 1.0f - std::exp(-1.0f / (float(refreshRate) * 0.2f));

    }

    ~SpectrumAnalyzer() {}

    void pushNextSampleIntoFifo(float sample) noexcept {
        if (fifoIndex == FFTParams::FFT_SIZE) {
            if (!nextFFTBlockReady) {
                std::copy_n(fifo.begin(), fifo.size(), fftData.begin());
                nextFFTBlockReady = true;
            }

            fifoIndex = 0;
        }

        fifo[fifoIndex++] = sample;
    }

    void setSampleRate(float sr) {
        sampleRate = sr;
    }

    void updateDriveValues(float d1, float d2, float d3) {
        drive1 = d1;
        drive2 = d2;
        drive3 = d3;
    }

    void getNextFFTData() {
        window.multiplyWithWindowingTable(fftData.data(), FFTParams::FFT_SIZE);
        forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());
    }

    void timerCallback() override {
        if (nextFFTBlockReady) {
            getNextFFTData();
            nextFFTBlockReady = false;
            repaint();
        }
    }

    void paint(Graphics& g) override {
        auto bounds = getLocalBounds().reduced(2.0f);
        auto top = bounds.getY();
        auto bottom = bounds.getHeight() + top;
        auto width = bounds.getWidth();
        auto height = bounds.getHeight();

        int lowMidPos = mapFromLog10(lowMidCut, 20.f, 20000.f) * width;
        int midHighPos = mapFromLog10(midHighCut, 20.f, 20000.f) * width;

        g.setColour(Colors::black);
        g.fillRoundedRectangle(bounds.toFloat(), 5);
                
        g.setColour(Colors::grey);
        g.drawRoundedRectangle(bounds.toFloat(), 5, 3.0f);
                
        drawFrame(g);

        
        if (drive1 > 0.0f) {
            auto bandArea = Rectangle<float>(bounds.getX(), bottom - drive1 * height, lowMidPos, height * drive1);
            g.setColour(Colors::red.withAlpha(0.3f * drive1));       
            g.fillRect(bandArea);
            g.setColour(Colors::red);       
            g.fillRect(bandArea.withHeight(3));
        }        
        
        if (drive2 > 0.0f) {
            auto bandArea = Rectangle<float>(lowMidPos, bottom - drive2 * height, midHighPos - lowMidPos, height * drive2);
            g.setColour(Colors::yellow.withAlpha(0.3f * drive2));       
            g.fillRect(bandArea);
            g.setColour(Colors::yellow);       
            g.fillRect(bandArea.withHeight(3));
        }
        
        if (drive3 > 0.0f) {
            auto bandArea = Rectangle<float>(midHighPos, bottom - drive3 * height, lowMidPos, height * drive3);
            g.setColour(Colors::green.withAlpha(0.3f * drive3));       
            g.fillRect(bandArea);
            g.setColour(Colors::green);       
            g.fillRect(bandArea.withHeight(3));
        }

        g.setColour(Colors::cream);
        g.fillRect(lowMidPos, bounds.getY(), 3, bounds.getHeight());
        g.fillRect(midHighPos, bounds.getY(), 3, bounds.getHeight());
    }

    void drawFrame(Graphics& g) {
        auto bounds = getLocalBounds();
        auto width = bounds.getWidth();
        auto height = bounds.getHeight();
        auto top = bounds.getY();
        auto minDb = -60.0f;
        auto maxDb = 6.0f;

        const auto binW = sampleRate / FFTParams::FFT_SIZE;
        const auto bins = FFTParams::FFT_SIZE * 0.5f;

        Path spectrumPath;
        spectrumPath.preallocateSpace(3 * width);

        float y = top + height;

        float lastX = 0.0f;
        float lastY = y;
        spectrumPath.startNewSubPath(0, y);

        for (int i = 1; i < bins; ++i) {
            if (fftData[i] > prev[i]) prev[i] = fftData[i];
            else prev[i] += (fftData[i] - prev[i]) * decay;
            
            auto level = prev[i];

            auto levelDb = linearToDb(level) - linearToDb(float(FFTParams::FFT_SIZE));
            auto y = jmap(levelDb, minDb, 6.f, float(top + height), float(top));

            if (!std::isnan(y) && !std::isinf(y)) {
                auto freq = i * binW;
                auto x = mapFromLog10(freq, 20.f, 20000.f) * width;

                if (i > 1) {
                    float midX = (lastX + x) * 0.5f;
                    float midY = (lastY + y) * 0.5f;

                    spectrumPath.quadraticTo(lastX, lastY, midX, midY);
                }

                lastX = x;
                lastY = y;
            }
        }

        spectrumPath.lineTo(lastX, float(top + height));  

        g.setColour(Colors::cream.withAlpha(0.3f)); 
        g.fillPath(spectrumPath);

        g.setColour(Colors::cream);
        g.strokePath(spectrumPath, PathStrokeType(2.0f, PathStrokeType::curved, PathStrokeType::rounded));

    }

};