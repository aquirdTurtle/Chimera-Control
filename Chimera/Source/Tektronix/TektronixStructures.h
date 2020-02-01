#pragma once

struct tektronicsChannelOutputForm
{
	bool control;
	bool on;
	bool fsk;
	Expression power;
	Expression mainFreq;
	Expression fskFreq;
};


struct tektronicsChannelOutput
{
	// eventually these double values take on the actual values being programmed, like so:
	// info.channels.first.mainFreqVal = std::stod(info.channels.first.mainFreq);
	// then if failing checking a key for variable values.
	double powerVal;
	double mainFreqVal;
	double fskFreqVal;
};


struct tektronicsInfo
{
	// add any other settings for the whole machine here. 
	std::pair<tektronicsChannelOutputForm, tektronicsChannelOutputForm> channels;
	int machineAddress;
};


struct tektronicsNums
{
	std::pair<tektronicsChannelOutput, tektronicsChannelOutput> channels;
};