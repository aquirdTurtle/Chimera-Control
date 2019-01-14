#pragma once

#include "Control.h"
#include "myButton.h"
#include "VisaFlume.h"
#include "Version.h"

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


class TektronicsChannelControl
{
	public:
		void initialize(POINT loc, CWnd* parent, int& id, std::string channel1Text, LONG width, UINT control_id );
		tektronicsChannelOutputForm getTekChannelSettings();
		void setSettings(tektronicsChannelOutputForm info);
		void rearrange(int width, int height, fontMap fonts);
		void handleOnPress();
		void handleFskPress();
		void handleButton ( UINT indicator );
	private:
		Control<CStatic> channelLabel;
		Control<CleanCheck> controlButton;
		Control<CleanCheck> onOffButton;
		Control<CleanCheck> fskButton;
		Control<CEdit> power;
		Control<CEdit> mainFreq;
		Control<CEdit> fskFreq;
		tektronicsChannelOutputForm currentInfo;
};


class TektronicsAfgControl
{
	public:
		TektronicsAfgControl(bool safemode, std::string address, std::string configurationFileDelimiter);
		void handleNewConfig( std::ofstream& newFile );
		void handleSaveConfig(std::ofstream& saveFile);
		void handleOpenConfig(std::ifstream& configFile, Version ver );
		void initialize( POINT& loc, CWnd* parent, int& id, std::string headerText, std::string channel1Text,
						 std::string channel2Text, LONG width, UINT id_ );
		std::string queryIdentity();
		tektronicsInfo getTekSettings();
		void setSettings(tektronicsInfo info);
		void rearrange(int width, int height, fontMap fonts);
		void handleButtons(UINT indicator);
		HBRUSH handleColorMessage(CWnd* window, CDC* cDC);
		void interpretKey(std::vector<std::vector<parameterType>>& variables);
		void programMachine(UINT variation );
		void handleProgram();
		const std::string configDelim;
	private:
		Control<CStatic> header;
		Control<CleanButton> programNow;
		Control<CStatic> onOffLabel;
		Control<CStatic> controlLabel;
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

