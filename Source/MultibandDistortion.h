#pragma once

#include "Utils.h"
#include "DSPParameters.h"
#include "Filters.h"
#include "FilteredParameter.h"

#define DEFAULT_SR 44100.0f

class Distortion
{
	float sampleRate{ DEFAULT_SR };
	int blockSize{ 0 };
	float nChannels{ 2.0f };

	FilteredParameter inputGain{};
	FilteredParameter outputGain{};
	FilteredParameter drive{};
	FilteredParameter mix{};
	FilteredParameter knee{};
	int bit{};

	float bitcrush(float sample, int bit);
	float clip(float input, float drive, float knee);
	float limit(float sample);

public:

	void prepare(DSPParameters<float>& params);
	void update(DSPParameters<float>& params);
	void processBlock(float* const* inputBuffer, int numChannels, int numSamples);
	float processSample(float sample);

};

class MultibandDistortion {
	float sampleRate{ DEFAULT_SR };
	int blockSize{ 0 };
	float nChannels{ 1.0f };

	Distortion lowDist;
	Distortion midDist;
	Distortion highDist;

	DSPParameters<float> lowParams;
	DSPParameters<float> midParams;
	DSPParameters<float> highParams;
	
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

public:

	void prepare(DSPParameters<float>& params);
	void update(DSPParameters<float>& params);
	void processBlock(float* const* inputBuffer, int numChannels, int numSamples);

};