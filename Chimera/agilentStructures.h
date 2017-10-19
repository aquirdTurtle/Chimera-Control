#pragma once

#include "scriptedAgilentWaveform.h"
#include "Expression.h"

#include <string>
#include <array>
#include "Windows.h"

class Agilent;

struct agilentSettings
{
	bool safemode;
	std::string address;
	ULONG sampleRate;
	std::string outputImpedance;
	std::string filterState;
	// "INT" or "USB"
	std::string memoryLocation;
	// button ids.
	ULONG chan1ButtonId;
	ULONG chan2ButtonId;
	ULONG syncButtonId;
	ULONG agilentComboId;
	ULONG functionComboId;
	ULONG editId;
	ULONG programButtonId;
	ULONG calibrationButtonId;

	ULONG triggerRow;
	ULONG triggerNumber;
};

struct minMaxDoublet
{
	double min;
	double max;
};


struct generalAgilentOutputInfo
{
	//std::string load;
	bool useCalibration = false;
};


struct dcInfo : public generalAgilentOutputInfo
{
	Expression dcLevelInput;
	double dcLevel = 0;
};


struct scriptedArbInfo : public generalAgilentOutputInfo
{
	std::string fileAddress = "";
	ScriptedAgilentWaveform wave;
};


struct squareInfo : public generalAgilentOutputInfo
{
	Expression frequencyInput;
	double frequency = 1;
	Expression amplitudeInput;
	double amplitude = 0;
	Expression offsetInput;
	double offset = 0;
	// not used yet
	Expression dutyCycleInput;
	double dutyCycle = 0;
	Expression phaseInput;
	double phase = 0;
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
	std::string address = "";
	// could add burst settings options, impedance options, etc.
};


struct channelInfo
{
	int option = -2;
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
	bool synced = false;
};

