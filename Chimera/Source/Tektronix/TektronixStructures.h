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

struct tektronicsInfo
{
	// add any other settings for the whole machine here. 
	std::array<tektronicsChannelOutputForm, 2> channels;
	int machineAddress;
};
