// created by Mark O. Brown
#pragma once

#include "CustomMfcControlWrappers/Control.h"
#include "CustomMfcControlWrappers/myButton.h"
#include "GeneralFlumes/VisaFlume.h"
#include "ConfigurationSystems/Version.h"
#include "ParameterSystem/Expression.h"
#include "TektronixStructures.h"
#include "TektronixChannelControl.h"


class TektronixAfgControl
{
	public:
		TektronixAfgControl(bool safemode, std::string address, std::string configurationFileDelimiter);
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
		Control<CleanPush> programNow;
		Control<CStatic> onOffLabel;
		Control<CStatic> controlLabel;
		Control<CStatic> fskLabel;
		Control<CStatic> mainPowerLabel;
		Control<CStatic> mainFreqLabel;
		Control<CStatic> fskFreqLabel;
		VisaFlume visaFlume;
		TektronixChannelControl channel1;
		TektronixChannelControl channel2;

		tektronicsInfo currentInfo;
		std::vector<tektronicsNums> currentNums;
};

