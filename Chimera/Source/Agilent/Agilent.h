// created by Mark O. Brown
#pragma once
#include "AgilentCore.h"
#include "Scripts/ScriptStream.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "GeneralFlumes/VisaFlume.h"
#include "Agilent/agilentStructures.h"
#include "Scripts/Script.h"
#include "DigitalOutput/DoRows.h"
#include <vector>
#include <array>
#include <qlabel.h>
#include <CustomQtControls/AutoNotifyCtrls.h>

class IChimeraWindowWidget;

// A class for programming agilent arbitrary waveform generators.
// in essense this includes a wrapper around agilent's implementation of the VISA protocol. 
class Agilent {
	public:
		// THIS CLASS IS NOT COPYABLE.
		Agilent& operator=(const Agilent&) = delete;
		Agilent (const Agilent&) = delete;

		Agilent( const agilentSettings & settings );
		void initialize( POINT& loc, std::string header, UINT editHeight, IChimeraWindowWidget* qtp,
			UINT width = 480);
		void updateButtonDisplay( int chan );
		void checkSave( std::string configPath, RunInfo info );
		void handleChannelPress( int chan, std::string configPath, RunInfo currentRunInfo );
		void handleModeCombo();

		void readGuiSettings();
		void readGuiSettings (int chan);
		bool scriptingModeIsSelected( );
		bool getSavedStatus ();
		void updateSavedStatus (bool isSaved);
		void handleSavingConfig( ConfigStream& saveFile, std::string configPath, RunInfo info );
		std::string getDeviceIdentity();
		void handleOpenConfig(ConfigStream& file);
		void updateSettingsDisplay( int chan, std::string configPath, RunInfo currentRunInfo );
		void updateSettingsDisplay( std::string configPath, RunInfo currentRunInfo );
		deviceOutputInfo getOutputInfo();
/*		void handleScriptVariation( UINT variation, scriptedArbInfo& scriptInfo, UINT channel, 
									std::vector<parameterType>& variables );*/
		// making the script public greatly simplifies opening, saving, etc. files from this script.
		Script agilentScript;
		std::pair<DoRows::which, UINT> getTriggerLine( );
		std::string getConfigDelim ();
		void programAgilentNow (std::vector<parameterType> constants);
		
		void setOutputSettings (deviceOutputInfo info);
		void verifyScriptable ( );
		AgilentCore& getCore ();
		void setDefault (UINT chan);
	private:
		AgilentCore core;
		minMaxDoublet chan2Range;
		const agilentSettings initSettings;
		// since currently all visaFlume communication is done to communicate with agilent machines, my visaFlume wrappers exist
		// in this class.
		int currentChannel=1;
		std::vector<minMaxDoublet> ranges;
		deviceOutputInfo currentGuiInfo;
		// GUI ELEMENTS
		
		QLabel* header;
		QLabel* deviceInfoDisplay;
		CQRadioButton* channel1Button;
		CQRadioButton* channel2Button;
		CQCheckBox* syncedButton;
		CQCheckBox* calibratedButton;
		CQComboBox* settingCombo;
		QLabel* optionsFormat;
		CQPushButton* programNow;
};


