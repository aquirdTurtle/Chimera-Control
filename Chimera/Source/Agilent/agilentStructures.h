// created by Mark O. Brown
#pragma once

#include "scriptedAgilentWaveform.h"
#include "AgilentChannelMode.h"
#include "ParameterSystem/Expression.h"
#include "DigitalOutput/DioRows.h"
#include <string>
#include <array>
#include "afxwin.h"

class Agilent;

struct agilentSettings
{
	bool safemode;
	std::string address;
	UINT sampleRate;
	// "INT" or "USB"
	std::string memoryLocation;
	std::string deviceName;

	// button ids.
	ULONG chan1ButtonId;
	ULONG chan2ButtonId;
	ULONG syncButtonId;
	ULONG agilentComboId;
	ULONG functionComboId;
	ULONG editId;
	ULONG programButtonId;
	ULONG calibrationButtonId;
	
	DioRows::which triggerRow;
	ULONG triggerNumber;
	std::string configurationFileDelimiter;

	std::vector<double> calibrationCoeff;
	std::vector<std::string> setupCommands;
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
	AgilentChannelMode::which option = AgilentChannelMode::which::No_Control;
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

