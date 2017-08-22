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
};


struct tektronicsChannelNums
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
	std::pair<tektronicsChannelInfo, tektronicsChannelInfo> channels;
	int machineAddress;
};


struct tektronicsNums
{
	std::pair<tektronicsChannelNums, tektronicsChannelNums> channels;
};


class TektronicsChannelControl
{
	public:
		void initialize(POINT loc, CWnd* parent, int& id, std::string channel1Text, LONG width, std::array<UINT, 2> ids );
		tektronicsChannelInfo getSettings();
		void setSettings(tektronicsChannelInfo info);
		void rearrange(int width, int height, fontMap fonts);
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
		TektronicsControl(bool safemode);
		void handleSaveConfig(std::ofstream& saveFile);
		void handleOpeningConfig(std::ifstream& configFile, double version);
		void initialize( POINT& loc, CWnd* parent, int& id, std::string headerText, std::string channel1Text,
						 std::string channel2Text, LONG width, std::string usbAddress, std::array<UINT, 5> ids );
		std::string queryIdentity();
		tektronicsInfo getSettings();
		void setSettings(tektronicsInfo info);
		void rearrange(int width, int height, fontMap fonts);
		void handleButtons(UINT indicator);
		HBRUSH handleColorMessage(CWnd* window, brushMap brushes, rgbMap rGBs, CDC* cDC);
		void interpretKey(key variationKey, std::vector<variable>& vars);
		void programMachine(UINT var);
		void handleProgram();
	private:
		Control<CStatic> header;
		Control<CButton> programNow;
		Control<CStatic> onOffLabel;
		Control<CStatic> fskLabel;
		Control<CStatic> mainPowerLabel;
		Control<CStatic> mainFreqLabel;
		Control<CStatic> fskFreqLabel;
		VisaFlume visaFlume;
		TektronicsChannelControl channel1;
		TektronicsChannelControl channel2;

		tektronicsInfo currentInfo;
		std::vector<tektronicsNums> currentNums;
};

