
#pragma once

#include <vector>
using std::vector;

#include "Utils.h"
#include "DSPParameters.h"
#include "Filters.h"
#include "FilteredParameter.h"

#define DEFAULT_SR 44100.0f

float msToCoefficient(float sampleRate, float length) {
	return expf(-1.0f / lengthToSamples(sampleRate, length));
}

using std::array;

#define NOISE_THRESHOLD 0.01f

enum DistortionType { HARD, SIGMA, SQUARE, SINE };

class MultibandDistortion
{
	float sampleRate{ 44100.0f };
	float blockSize{ 0.0f };
	float nChannels{ 1.0f };

	FilteredParameter inputGain{};
	FilteredParameter outputGain{};
	FilteredParameter drive{};
	FilteredParameter mix{};
	int type{ 2 };

public:

	void prepare(DSPParameters<float>& params) {
		sampleRate = params["sampleRate"];
		blockSize = params["blockSize"];
		nChannels = params["nChannels"];

		inputGain.prepare(sampleRate);
		outputGain.prepare(sampleRate);
		drive.prepare(sampleRate);
		mix.prepare(sampleRate);

		update(params);
	}

	void update(DSPParameters<float>& params) {
		inputGain.update(dbToLinear(params["inputGain"]));
		outputGain.update(dbToLinear(params["outputGain"]));
		drive.update(params["drive"]);
		mix.update(params["mix"] * 0.01f);
		type = static_cast<int>(params["distortionType"]);
	}

	void processBlock(float* const* inputBuffer, int numChannels, int numSamples) {

		for (int ch = 0; ch < numChannels; ++ch) {
			for (auto s = 0; s < numSamples; ++s) {
				auto sample = inputBuffer[ch][s];

				float output;

				float currentInputGain = inputGain.next();
				float currentOutputGain = outputGain.next();
				float currentDrive = drive.next();

				switch (type) {
				case(DistortionType::HARD):
					output = hardClip(sample * currentInputGain, currentDrive);
					break;
				case(DistortionType::SIGMA):
					output = sigmaClip(sample * currentInputGain, currentDrive);
					break;
				case(DistortionType::SQUARE):
					output = squareClip(sample * currentInputGain);
					break;
				case(DistortionType::SINE):
					output = sineFoldover1(sample * currentInputGain, currentDrive);
					break;
				default:
					output = sample;
					break;
				}

				float dry = 1.0f - mix.next();
				inputBuffer[ch][s] = limit(output) * currentOutputGain;
			}
		}
	}

private:
	float hardClip(float sample, float drive) {
		sample *= drive;
		if (sample > 1.0f)  return 1.0f;
		if (sample < -1.0f) return -1.0f;
		return sample;
	}

	float sigmaClip(float sample, float drive) {
		return std::tanh(sample * drive);
	}

	float squareClip(float sample) {
		if (std::abs(sample) > NOISE_THRESHOLD) {
			return sample > 0.0f ? 1.0f : -1.0f;
		}
		return sample;
	}

	float sineFoldover1(float sample, float drive) {
		sample *= drive;
		return std::sin(sample);
	}

	float limit(float sample) {
		if (sample > 1.0f) return 1.0f;
		if (sample < -1.0f) return -1.0f;
		return sample;
	}
};