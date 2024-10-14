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
        auto bounds = getLocalBounds();
        auto width = bounds.getWidth();

        auto lowMidPos = mapFromLog10(lowMidCut, 20.f, 20000.f) * width;
                
        drawFrame(g);

        g.setColour(Colors::cream);
        g.drawVerticalLine(lowMidPos, bounds.getY(), bounds.getHeight());
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
        spectrumPath.lineTo(0, float(top + height));      

        g.setColour(Colors::cream.withAlpha(0.3f)); 
        g.fillPath(spectrumPath);

        g.setColour(Colors::cream);
        g.strokePath(spectrumPath, PathStrokeType(2.0f, PathStrokeType::curved, PathStrokeType::rounded));

    }

};