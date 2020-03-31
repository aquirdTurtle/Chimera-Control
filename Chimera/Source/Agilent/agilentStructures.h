// created by Mark O. Brown
#pragma once

#include "scriptedAgilentWaveform.h"
#include "AgilentChannelMode.h"
#include "ParameterSystem/Expression.h"
#include "DigitalOutput/DoRows.h"
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
	
	DoRows::which triggerRow;
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
	bool useCal = false;
};


struct dcInfo : public generalAgilentOutputInfo
{
	Expression dcLevel;
};


struct scriptedArbInfo : public generalAgilentOutputInfo
{
	Expression fileAddress = "";
	ScriptedAgilentWaveform wave;
};


struct squareInfo : public generalAgilentOutputInfo
{
	Expression frequency;
	Expression amplitude;
	Expression offset;
	// not used yet // ?!?!
	Expression dutyCycle;
	Expression phase;
};


struct sineInfo : public generalAgilentOutputInfo
{
	Expression frequency;
	Expression amplitude;
};


struct preloadedArbInfo : public generalAgilentOutputInfo
{
	// The only reason at this time to make this an expression instead of a normal string is to make sure it gets 
	// outputted to the config file correctly in case it's empty. 
	Expression address = "";
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

