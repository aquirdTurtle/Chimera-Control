// created by Mark O. Brown
#pragma once

#include "TekCore.h"
#include "TektronixStructures.h"
#include "TektronixChannelControl.h"

#include "GeneralFlumes/VisaFlume.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "ParameterSystem/Expression.h"
#include "PrimaryWindows/IChimeraQtWindow.h"
#include <QLabel.h>
#include "QPushButton.h"

// QtWindow (e.g. QtMainWindow, QtAndorWindow)

// IChimeraSystem ("System" class)
// - responsible for gui stuff

// IDeviceCore ("Core" class)
// - actual programming

// "Flume" class 
//	- safemode
//	- errors


// "System class" but doesn't inherit from IChimeraSystem yet
class TektronixAfgControl {
	public:
		// THIS CLASS IS NOT COPYABLE.
		TektronixAfgControl& operator=(const TektronixAfgControl&) = delete;
		TektronixAfgControl (const TektronixAfgControl&) = delete;
		TektronixAfgControl(bool safemode, std::string address, std::string configurationFileDelimiter);

		void handleSaveConfig(ConfigStream& saveFile);
		void handleOpenConfig(ConfigStream& configFile);

		void initialize( POINT& loc, IChimeraQtWindow* parent, std::string headerText, std::string channel1Text,
						 std::string channel2Text, LONG width);
		std::string queryIdentity();
		tektronixInfo getTekSettings();
		void setSettings(tektronixInfo info);
		void handleProgram(std::vector<parameterType> constants);
		std::string getDelim ();
		TekCore& getCore ();
	private:
		QLabel* header;
		QPushButton* programNow;
		QPushButton* newButton;
		QLabel* onOffLabel;
		QLabel* controlLabel;
		QLabel* fskLabel;
		QLabel* mainPowerLabel;
		QLabel* mainFreqLabel;
		QLabel* fskFreqLabel;

		TektronixChannelControl channel1;
		TektronixChannelControl channel2;
		TekCore core;
		tektronixInfo currentInfo;
};

