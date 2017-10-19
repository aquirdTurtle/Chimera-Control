#pragma once

#include "ProfileSystem.h"
#include "ScriptStream.h"
#include "VisaFlume.h"
#include "commonTypes.h"
#include "agilentStructures.h"
#include "Script.h"
#include "Expression.h"
#include <vector>
#include <array>
#include "Windows.h"


// A class for programming agilent machines.
// in essense this includes a wrapper around agilent's implementation of the VISA protocol. This could be pretty easily
// abstracted away from the agilent class if new systems wanted to use this functionality.
class Agilent
{
	public:
		Agilent( const agilentSettings & settings );
		~Agilent( );
		void initialize( POINT& loc, cToolTips& toolTips, CWnd* master, int& id,   
						 std::string header, UINT editHeight, COLORREF color );
		void updateButtonDisplay( int chan );
		void checkSave( std::string categoryPath, RunInfo info );
		void handleChannelPress( int chan, std::string currentCategoryPath, RunInfo currentRunInfo );
		void handleCombo();
		void setDC( int channel, dcInfo info );
		void setExistingWaveform( int channel, preloadedArbInfo info );
		void setSquare( int channel, squareInfo info );
		void setSine( int channel, sineInfo info );
		void outputOff(int channel);
		void handleInput( std::string categoryPath, RunInfo info );
		void handleInput( int chan, std::string categoryPath, RunInfo info );
		void setDefault( int channel );
		void prepAgilentSettings(UINT channel );
		bool connected();
		void analyzeAgilentScript( scriptedArbInfo& infoObj, std::vector<variableType>& vars );
		HBRUSH handleColorMessage(CWnd* window, brushMap brushes, rgbMap rGBs, CDC* cDC);
		void handleNewConfig( std::ofstream& saveFile );
		void handleSavingConfig( std::ofstream& saveFile, std::string categoryPath, RunInfo info );
		std::string getDeviceIdentity();
		std::string getName();
		void readConfigurationFile( std::ifstream& file, int versionMajor, int versionMinor );
		void convertInputToFinalSettings(UINT chan, UINT variation, std::vector<variableType>& variables);
		void convertInputToFinalSettings(UINT chan);
		void updateSettingsDisplay( int chan, std::string currentCategoryPath, RunInfo currentRunInfo );
		void updateSettingsDisplay( std::string currentCategoryPath, RunInfo currentRunInfo );
		deviceOutputInfo getOutputInfo();
		void rearrange(UINT width, UINT height, fontMap fonts);
		void setAgilent( UINT variation, std::vector<variableType>& variables);
		void setAgilent();
		void handleScriptVariation( UINT variation, scriptedArbInfo& scriptInfo, UINT channel, 
			std::vector<variableType>& variables);
		void handleNoVariations( scriptedArbInfo& scriptInfo, UINT channel );
		void setScriptOutput(UINT varNum, scriptedArbInfo scriptInfo, UINT channel );
		// making the script public greatly simplifies opening, saving, etc. files from this script.
		Script agilentScript;
		static double convertPowerToSetPoint(double power, bool conversionOption );
		std::pair<UINT, UINT> getTriggerLine( );
	private:
		// not that important, just used to check that number of triggers in script matches number in agilent.
		const UINT triggerRow;
		const UINT triggerNumber;
		const agilentSettings initSettings;
		std::string name;
		minMaxDoublet chan2Range;
		VisaFlume visaFlume;
		const double sampleRate;
		const std::string load;
		const std::string filterState;
		const std::string memoryLocation;
		// since currently all visaFlume communication is done to communicate with agilent machines, my visaFlume wrappers exist
		// in this class.
		bool isConnected;
		int currentChannel;
		std::string deviceInfo;
		std::vector<minMaxDoublet> ranges;		
		deviceOutputInfo settings;
		// GUI ELEMENTS
		Control<CStatic> header;
		Control<CStatic> deviceInfoDisplay;
		Control<CButton> channel1Button;
		Control<CButton> channel2Button;
		Control<CButton> syncedButton;
		Control<CButton> calibratedButton;
		Control<CComboBox> settingCombo;
		Control<CStatic> optionsFormat;
		Control<CButton> programNow;
};
