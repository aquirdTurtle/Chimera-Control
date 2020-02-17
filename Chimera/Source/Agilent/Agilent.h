// created by Mark O. Brown
#pragma once
#include "AgilentCore.h"
#include "CustomMfcControlWrappers/myButton.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ProfileSystem.h"
#include "Scripts/ScriptStream.h"
#include "GeneralFlumes/VisaFlume.h"
#include "GeneralObjects/commonTypes.h"
#include "Agilent/agilentStructures.h"
#include "Scripts/Script.h"
#include "ParameterSystem/Expression.h"
#include "DigitalOutput/DioRows.h"
#include <vector>
#include <array>


// A class for programming agilent arbitrary waveform generators.
// in essense this includes a wrapper around agilent's implementation of the VISA protocol. 
class Agilent
{
	public:
		// THIS CLASS IS NOT COPYABLE.
		Agilent& operator=(const Agilent&) = delete;
		Agilent (const Agilent&) = delete;

		Agilent( const agilentSettings & settings );
		void initialize( POINT& loc, cToolTips& toolTips, CWnd* master, int& id,   
						 std::string header, UINT editHeight, COLORREF color, UINT width = 480);
		void updateButtonDisplay( int chan );
		void checkSave( std::string configPath, RunInfo info );
		void handleChannelPress( int chan, std::string configPath, RunInfo currentRunInfo );
		void handleModeCombo();

		void readGuiSettings();
		void Agilent::readGuiSettings (int chan);
		bool scriptingModeIsSelected( );
		bool getSavedStatus ();
		void updateSavedStatus (bool isSaved);
		HBRUSH handleColorMessage(CWnd* window, CDC* cDC);
		void handleNewConfig( std::ofstream& saveFile );
		void handleSavingConfig( std::ofstream& saveFile, std::string configPath, RunInfo info );
		std::string getDeviceIdentity();
		void handleOpenConfig( std::ifstream& file, Version ver );
		void updateSettingsDisplay( int chan, std::string configPath, RunInfo currentRunInfo );
		void updateSettingsDisplay( std::string configPath, RunInfo currentRunInfo );
		deviceOutputInfo getOutputInfo();
		void rearrange(UINT width, UINT height, fontMap fonts);

		void handleScriptVariation( UINT variation, scriptedArbInfo& scriptInfo, UINT channel, 
									std::vector<parameterType>& variables );
		// making the script public greatly simplifies opening, saving, etc. files from this script.
		Script agilentScript;
		std::pair<DioRows::which, UINT> getTriggerLine( );
		std::string getConfigDelim ();
		void programAgilentNow (std::vector<parameterType> constants);
		
		static deviceOutputInfo getOutputSettingsFromConfigFile (std::ifstream& file, Version ver);
		void Agilent::setOutputSettings (deviceOutputInfo info);
		void verifyScriptable ( );
		AgilentCore* getCore ();
		void setDefault (UINT chan);
	private:
		AgilentCore core;
		minMaxDoublet chan2Range;
		const agilentSettings initSettings;
		// since currently all visaFlume communication is done to communicate with agilent machines, my visaFlume wrappers exist
		// in this class.
		int currentChannel;
		std::vector<minMaxDoublet> ranges;
		deviceOutputInfo currentGuiInfo;
		// GUI ELEMENTS
		Control<CStatic> header;
		Control<CStatic> deviceInfoDisplay;
		Control<CButton> channel1Button;
		Control<CButton> channel2Button;
		Control<CleanCheck> syncedButton;
		Control<CleanCheck> calibratedButton;
		Control<CComboBox> settingCombo;
		Control<CStatic> optionsFormat;
		Control<CleanPush> programNow;
};


