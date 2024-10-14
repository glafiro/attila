#include "MultibandDistortion.h"


void Distortion::prepare(DSPParameters<float>& params) {
	sampleRate = params["sampleRate"];
	blockSize = params["blockSize"];
	nChannels = params["nChannels"];

	inputGain.prepare(sampleRate);
	outputGain.prepare(sampleRate);
	drive.prepare(sampleRate);
	mix.prepare(sampleRate);
	knee.prepare(sampleRate);
}

void Distortion::update(DSPParameters<float>& params) {
	inputGain.update(dbToLinear(params["inputGain"]));
	outputGain.update(dbToLinear(params["outputGain"]));
	drive.update(dbToLinear(params["drive"]));
	knee.update(params["knee"]);
	bit = static_cast<int>(params["bit"]);
	mix.update(params["mix"] * 0.01f);
}

void Distortion::processBlock(float* const* inputBuffer, int numChannels, int numSamples) {
	for (int ch = 0; ch < numChannels; ++ch) {
		for (auto s = 0; s < numSamples; ++s) {
			auto sample = inputBuffer[ch][s];
			inputBuffer[ch][s] = processSample(sample);
		}
	}
}

float Distortion::processSample(float sample) {
	float output = clip(sample * inputGain.next(), drive.next(), knee.next());
	output = bitcrush(output, bit);
	return limit(output) * outputGain.next();
}


float Distortion::bitcrush(float sample, int bit) {
	float QL = 2.0 / (pow(2.0, bit) - 1.0);
	return QL * static_cast<int>(sample / QL);

}

// https://www.musicdsp.org/en/latest/Effects/104-variable-hardness-clipping-function.html
float Distortion::clip(float input, float drive, float knee) {
	input *= drive;
	return sign(input) * pow(fastatan(pow(fabs(input), knee)), (1.0f / knee));
}

float Distortion::limit(float sample) {
	if (sample > 4.0f) return 4.0f;
	if (sample < -4.0f) return -4.0f;
	return sample;
}

void MultibandDistortion::prepare(DSPParameters<float>& params) {
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

void MultibandDistortion::update(DSPParameters<float>& params) {
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

void MultibandDistortion::processBlock(float* const* inputBuffer, int numChannels, int numSamples) {
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

			auto wet = mix.next() * (lowBandDistorted + midBandDistorted + highBandDistorted);
			auto dry = (1.0f - mix.read()) * sample;
			auto amplitude = allEnabled.next();

			auto output = sample * (1.0f - amplitude) + ((wet + dry) * amplitude * outputGain.next());
			inputBuffer[ch][s] = output;
		}
	}
}


