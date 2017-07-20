#pragma once

#include "Control.h"
#include "KeyHandler.h"


struct tektronicsChannelInfo
{
	bool on;
	bool fsk;

	std::string power;
	std::string mainFreq;
	std::string fskFreq;
	
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
	std::pair<tektronicsChannelInfo, tektronicsChannelInfo> channels;
	int machineAddress;
};

class TektronicsChannelControl
{
	public:
		void initialize(POINT loc, CWnd* parent, int& id, std::string channel1Text, LONG width);
		tektronicsChannelInfo getSettings();
		void setSettings(tektronicsChannelInfo info);
		void rearrange(int width, int height, std::unordered_map<std::string, CFont*> fonts);
		void handleOnPress();
		void handleFskPress();
	private:
		Control<CStatic> channelLabel;
		Control<CButton> onOffButton;
		Control<CButton> fskButton;
		Control<CEdit> power;
		Control<CEdit> mainFreq;
		Control<CEdit> fskFreq;
		tektronicsChannelInfo currentInfo;
};

class TektronicsControl
{
	public:
		TektronicsControl(int address); 

		void initialize(POINT& loc, CWnd* parent, int& id, std::string headerText, std::string channel1Text,
						std::string channel2Text, LONG width);
		tektronicsInfo getSettings();
		void setSettings(tektronicsInfo info);
		void rearrange(int width, int height, std::unordered_map<std::string, CFont*> fonts);
		void handleButtons(UINT indicator);

	private:
		Control<CStatic> header;
		Control<CStatic> onOffLabel;
		Control<CStatic> fskLabel;
		Control<CStatic> mainPowerLabel;
		Control<CStatic> mainFreqLabel;
		Control<CStatic> fskFreqLabel;
		const int machineAddress;
		TektronicsChannelControl channel1;
		TektronicsChannelControl channel2;

		tektronicsInfo currentInfo;
};

