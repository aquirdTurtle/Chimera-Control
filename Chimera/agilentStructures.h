#pragma once

#include "scriptedAgilentWaveform.h"
#include "Expression.h"

#include <string>
#include <array>
#include "Windows.h"

class Agilent;

struct minMaxDoublet
{
	double min;
	double max;
};


struct generalAgilentOutputInfo
{
	std::string load;
	double sampleRate;
	bool useCalibration;
};


struct dcInfo : public generalAgilentOutputInfo
{
	Expression dcLevelInput;
	double dcLevel;
};


struct scriptedArbInfo : public generalAgilentOutputInfo
{
	std::string fileAddress;
	ScriptedAgilentWaveform wave;
};


struct squareInfo : public generalAgilentOutputInfo
{
	Expression frequencyInput;
	double frequency;
	Expression amplitudeInput;
	double amplitude;
	Expression offsetInput;
	double offset;
};


struct sineInfo : public generalAgilentOutputInfo
{
	Expression frequencyInput;
	double frequency;
	Expression amplitudeInput;
	double amplitude;
};


struct preloadedArbInfo : public generalAgilentOutputInfo
{
	std::string address;
	// could add burst settings options, impedance options, etc.
};


struct channelInfo
{
	int option;
	dcInfo dc;
	sineInfo sine;
	squareInfo square;
	preloadedArbInfo preloadedArb;
	scriptedArbInfo scriptedArb;
};


struct deviceOutputInfo
{
	// first ([0]) is channel 1, second ([1]) is channel 2.
	std::array<channelInfo, 2> channel;
	bool synced;
};

