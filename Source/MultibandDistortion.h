
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

#define NOISE_THRESHOLD 0.01f

enum DistortionType { HARD, TANH, SIGMOID, FUZZ_EXP, SINE };


class MultibandDistortion
{
	float sampleRate{ 44100.0f };
	float blockSize{ 0.0f };
	float nChannels{ 1.0f };

	FilteredParameter inputGain{};
	FilteredParameter outputGain{};
	FilteredParameter drive{};
	FilteredParameter mix{};
	
	int bitcrushBit{};
	bool bitcrushOn{};
	
	FilteredParameter sineFreq{};
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
		drive.update(dbToLinear(params["drive"]));
		mix.update(params["mix"] * 0.01f);
		type = static_cast<int>(params["distortionType"]);
		bitcrushBit = static_cast<int>(params["bitcrushBit"]);
		bitcrushOn = static_cast<bool>(params["bitcrushOn"]);
		sineFreq.update(params["sineFreq"]);
	}

	void processBlock(float* const* inputBuffer, int numChannels, int numSamples) {
		for (int ch = 0; ch < numChannels; ++ch) {
			for (auto s = 0; s < numSamples; ++s) {
				auto sample = inputBuffer[ch][s];

				float currentInputGain = inputGain.next();
				float currentOutputGain = outputGain.next();
				float currentDrive = drive.next();
				float currentMix = mix.next();

				float output;

				switch (type) {
				case(DistortionType::HARD):
					output = hardClip(sample * currentInputGain, currentDrive);
					break;
				case(DistortionType::TANH):
					output = tanhClip(sample * currentInputGain, currentDrive);
					break;
				case(DistortionType::SIGMOID):
					output = sigmoidClip(sample * currentInputGain, currentDrive);
					break;
				case(DistortionType::FUZZ_EXP):
					output = fuzzExponential(sample * currentInputGain, currentDrive);
					break;
				case(DistortionType::SINE):
					output = sineFoldover1(sample * currentInputGain, currentDrive, sineFreq.next());
					break;
				default:
					output = sample;
					break;
				}

				if (bitcrushOn) {
					output = bitcrush(output, bitcrushBit);
				}

				float dry = 1.0f - currentMix;
				inputBuffer[ch][s] = (sample * dry + limit(output) * currentMix) * currentOutputGain;
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

	float tanhClip(float sample, float drive) {
		return std::tanh(sample * drive);
	}

	float sigmoidClip(float sample, float drive) {
		return 2.0f * (1.0 / (1.0 + exp(-drive * sample))) - 1.0f;
	}

	float fuzzExponential(float sample, float drive) {
		float sign = sample >= 0.0f ? 1.0f : -1.0f;
		return sign * (1.0f - exp(-abs(drive * sample))) / (1.0 - exp(-drive));
	}


	float sineFoldover1(float sample, float drive, float freq) {
		sample *= drive;
		return std::sin(sample * freq);
	}

	float bitcrush(float sample, int bit) {

		float QL = 2.0 / (pow(2.0, bit) - 1.0);
		return QL * static_cast<int>(sample / QL);
	}

	float limit(float sample) {
		if (sample > 1.0f) return 1.0f;
		if (sample < -1.0f) return -1.0f;
		return sample;
	}
};