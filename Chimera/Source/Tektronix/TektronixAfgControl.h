// created by Mark O. Brown
#pragma once

#include "TekCore.h"
#include "TektronixStructures.h"
#include "TektronixChannelControl.h"

#include "CustomMfcControlWrappers/Control.h"
#include "CustomMfcControlWrappers/myButton.h"
#include "GeneralFlumes/VisaFlume.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "ParameterSystem/Expression.h"


class TektronixAfgControl
{
	public:
		// THIS CLASS IS NOT COPYABLE.
		TektronixAfgControl& operator=(const TektronixAfgControl&) = delete;
		TektronixAfgControl (const TektronixAfgControl&) = delete;

		TektronixAfgControl(bool safemode, std::string address, std::string configurationFileDelimiter);
		void handleSaveConfig(ConfigStream& saveFile);
		void handleOpenConfig(ConfigStream& configFile, Version ver );
		void initialize( POINT& loc, CWnd* parent, int& id, std::string headerText, std::string channel1Text,
						 std::string channel2Text, LONG width, UINT id_ );
		std::string queryIdentity();
		tektronixInfo getTekSettings();
		void setSettings(tektronixInfo info);
		void rearrange(int width, int height, fontMap fonts);
		void handleButtons(UINT indicator);
		HBRUSH handleColorMessage(CWnd* window, CDC* cDC);
		void handleProgram(std::vector<parameterType> constants);
		std::string getDelim ();
		TekCore& getCore ();
	private:
		Control<CStatic> header;
		Control<CleanPush> programNow;
		Control<CStatic> onOffLabel;
		Control<CStatic> controlLabel;
		Control<CStatic> fskLabel;
		Control<CStatic> mainPowerLabel;
		Control<CStatic> mainFreqLabel;
		Control<CStatic> fskFreqLabel;
		TektronixChannelControl channel1;
		TektronixChannelControl channel2;
		TekCore core;
		tektronixInfo currentInfo;
};

