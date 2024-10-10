
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

class MultibandDistortion
{
	float sampleRate{ 44100.0f };
	float blockSize{ 0.0f };
	float nChannels{ 1.0f };

	FilteredParameter inputGain{};
	FilteredParameter outputGain{};
	FilteredParameter drive{};
	FilteredParameter mix{};
	FilteredParameter knee{};
	FilteredParameter bitcrushShape{};
	
	int bitcrushBit{};

public:

	void prepare(DSPParameters<float>& params) {
		sampleRate = params["sampleRate"];
		blockSize = params["blockSize"];
		nChannels = params["nChannels"];

		inputGain.prepare(sampleRate);
		outputGain.prepare(sampleRate);
		drive.prepare(sampleRate);
		mix.prepare(sampleRate);
		knee.prepare(sampleRate);
		bitcrushShape.prepare(sampleRate);

		update(params);
	}

	void update(DSPParameters<float>& params) {
		inputGain.update(dbToLinear(params["inputGain"]));
		outputGain.update(dbToLinear(params["outputGain"]));
		drive.update(dbToLinear(params["drive"]));
		mix.update(params["mix"] * 0.01f);
		bitcrushBit = static_cast<int>(params["bitcrushBit"]);
		knee.update(params["knee"]);
		bitcrushShape.update(params["bitcrushShape"] * 0.01f);
	}

	void processBlock(float* const* inputBuffer, int numChannels, int numSamples) {
		for (int ch = 0; ch < numChannels; ++ch) {
			for (auto s = 0; s < numSamples; ++s) {
				auto sample = inputBuffer[ch][s];

				float currentInputGain = inputGain.next();
				float currentOutputGain = outputGain.next();
				float currentDrive = drive.next();
				float currentMix = mix.next();
				float currentKnee = knee.next();

				float output = clip(sample * currentInputGain, currentDrive, currentKnee);

				output = bitcrush(output, bitcrushBit, bitcrushShape.next());

				float dry = 1.0f - currentMix;
				inputBuffer[ch][s] = (sample * dry + limit(output) * currentMix) * currentOutputGain;
			}
		}
	}

private:

	float bitcrush(float sample, int bit, float shape) {
		float shapedSample = copysign(pow(fabs(sample), shape), sample);

		float QL = 2.0 / (pow(2.0, bit) - 1.0);
		float quantized =  QL * static_cast<int>(shapedSample / QL);

		return copysignf(powf(fabs(quantized), 1.0f / shape), quantized);
	}

    // https://www.musicdsp.org/en/latest/Effects/104-variable-hardness-clipping-function.html
	float clip(float input, float drive, float knee) {
		input *= drive;
		return sign(input) * pow(fastatan(pow(fabs(input), knee)), (1.0f / knee));
	}

	float limit(float sample) {
		if (sample > 1.0f) return 1.0f;
		if (sample < -1.0f) return -1.0f;
		return sample;
	}
};