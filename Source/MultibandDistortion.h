
#pragma once

#include <vector>
using std::vector;

#include "Utils.h"
#include "DSPParameters.h"
#include "Filters.h"
#include "FilteredParameter.h"

#define DEFAULT_SR 44100.0f

class Distortion
{
	float sampleRate{ 44100.0f };
	float blockSize{ 0.0f };
	float nChannels{ 2.0f };

	FilteredParameter inputGain{};
	FilteredParameter outputGain{};
	FilteredParameter drive{};
	FilteredParameter mix{};
	FilteredParameter knee{};
	int bit{};

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
	}

	void update(DSPParameters<float>& params) {
		inputGain.update(dbToLinear(params["inputGain"]));
		outputGain.update(dbToLinear(params["outputGain"]));
		drive.update(dbToLinear(params["drive"]));
		knee.update(params["knee"]);
		bit = static_cast<int>(params["bit"]);
		mix.update(params["mix"] * 0.01f);
	}

	void processBlock(float* const* inputBuffer, int numChannels, int numSamples) {
		for (int ch = 0; ch < numChannels; ++ch) {
			for (auto s = 0; s < numSamples; ++s) {
				auto sample = inputBuffer[ch][s];
				inputBuffer[ch][s] = processSample(sample);
			}
		}
	}

	float processSample(float sample) {

		float currentInputGain = inputGain.next();
		float currentOutputGain = outputGain.next();
		float currentDrive = drive.next();
		float currentMix = mix.next();
		float currentKnee = knee.next();

		float output = clip(sample * currentInputGain, currentDrive, currentKnee);
		output = bitcrush(output, bit);

		return limit(output) * currentOutputGain;
	}

private:

	float bitcrush(float sample, int bit) {
		float QL = 2.0 / (pow(2.0, bit) - 1.0);
		return QL * static_cast<int>(sample / QL);

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

class MultibandDistortion {
public:
	Distortion lowDist;
	Distortion midDist;
	Distortion highDist;

	DSPParameters<float> lowParams;
	DSPParameters<float> midParams;
	DSPParameters<float> highParams;
	
	float sampleRate{ 44100.0f };
	float blockSize{ 0.0f };
	float nChannels{ 1.0f };

	FilteredParameter inputGain{};
	FilteredParameter outputGain{};
	FilteredParameter mix{1.0f};
	bool bypass{ false };

	LRFilter<float> lowMidFilter;
	LRFilter<float> midHighFilter;
	FilteredParameter lowMidCut{};
	FilteredParameter midHighCut{};

	SmoothLogParameter lowEnabled;
	SmoothLogParameter midEnabled;
	SmoothLogParameter highEnabled;
	SmoothLogParameter allEnabled;

	void prepare(DSPParameters<float>& params) {
		sampleRate = params["sampleRate"];
		blockSize = params["blockSize"];
		nChannels = params["nChannels"];

		lowDist.prepare(params);
		midDist.prepare(params);
		highDist.prepare(params);

		lowMidFilter.prepare(sampleRate, blockSize, nChannels);
		midHighFilter.prepare(sampleRate, blockSize, nChannels);
		lowMidCut.update(params["lowMidCut"]);
		midHighCut.update(params["midHighCut"]);

		update(params);
	}

	void update(DSPParameters<float>& params) {
		// Band specific parameters
		lowParams.set("inputGain", params["inputGain1"]);
		lowParams.set("outputGain", params["outputGain1"]);
		lowParams.set("drive", params["drive1"]);
		lowParams.set("knee", params["knee1"]);
		lowParams.set("bit", params["bit1"]);
		lowParams.set("bypass", params["bypass1"]);
		lowDist.update(lowParams);
		midParams.set("inputGain", params["inputGain2"]);
		midParams.set("outputGain", params["outputGain2"]);
		midParams.set("drive", params["drive2"]);
		midParams.set("knee", params["knee2"]);
		midParams.set("bit", params["bit2"]);
		midParams.set("bypass", params["bypass2"]);
		midDist.update(midParams);
		highParams.set("inputGain", params["inputGain3"]);
		highParams.set("outputGain", params["outputGain3"]);
		highParams.set("drive", params["drive3"]);
		highParams.set("knee", params["knee3"]);
		highParams.set("bit", params["bit3"]);
		highParams.set("bypass", params["bypass3"]);
		highDist.update(highParams);

		// Global 
		lowEnabled.setValue(1.0f - params["bypass1"]);
		midEnabled.setValue(1.0f - params["bypass2"]);
		highEnabled.setValue(1.0f - params["bypass3"]);
		allEnabled.setValue(1.0f - params["bypass"]);
		inputGain.update(dbToLinear(params["inputGain"]));
		outputGain.update(dbToLinear(params["outputGain"]));
		bypass = static_cast<bool>(params["bypass"]);
		mix.update(params["mix"] * 0.01f);
		lowMidCut.update(params["lowMidCut"]);
		midHighCut.update(params["midHighCut"]);
	}

	void processBlock(float* const* inputBuffer, int numChannels, int numSamples) {
		for (int ch = 0; ch < numChannels; ++ch) {
			for (auto s = 0; s < numSamples; ++s) {
				auto sample = inputGain.next() * inputBuffer[ch][s];
				
				float lowBandFiltered, midBandFiltered, highBandFiltered;
				float lowBandDistorted, midBandDistorted, highBandDistorted;

				lowMidFilter.setFrequency(lowMidCut.next());
				midHighFilter.setFrequency(midHighCut.next());

				lowMidFilter.processSample(ch, sample, lowBandFiltered, midBandFiltered);
				midHighFilter.processSample(ch, midBandFiltered, midBandFiltered, highBandFiltered);

				lowBandDistorted = lowDist.processSample(lowBandFiltered) * lowEnabled.next();
				midBandDistorted = midDist.processSample(midBandFiltered) * midEnabled.next();
				highBandDistorted = highDist.processSample(highBandFiltered) * highEnabled.next();

				float wet = mix.next() * (lowBandDistorted + midBandDistorted + highBandDistorted);
				float dry = (1.0f - mix.read()) * sample;
				float amplitude = allEnabled.next();

				inputBuffer[ch][s] =
					sample * (1.0f - amplitude) + ((wet + dry) * amplitude * outputGain.next());
			}
		}
	}



};