#pragma once

struct tektronixChannelOutput
{
	bool control;
	bool on;
	bool fsk;
	Expression power;
	Expression mainFreq;
	Expression fskFreq;
};

struct tektronixInfo
{
	// add any other settings for the whole machine here. 
	std::array<tektronixChannelOutput, 2> channels;
	int machineAddress;
};
