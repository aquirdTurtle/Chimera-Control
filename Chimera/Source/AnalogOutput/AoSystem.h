// created by Mark O. Brown
#pragma once
#include <array>
#include <string>
#include <unordered_map>

#include "CustomMfcControlWrappers/Control.h"
#include "CustomMfcControlWrappers/myButton.h"
#include "ParameterSystem/ParameterSystem.h"
#include "DigitalOutput/DoCore.h"
#include "AnalogOutput/DaqMxFlume.h"
#include "AnalogOutput/AoStructures.h"
#include "AnalogOutput/AnalogOutput.h"

#include "ConfigurationSystems/Version.h"

class MainWindow;

/**
 * AIO in the name stands for Analog In and Out, or measuring analog signals and producing analog signals.
 *
 * The AoSystem is meant to be a constant class but it currently doesn't actually prevent the user from making 
 * multiple copies of the object. This class is based off of the DAC.bas module in the original VB6 code, of course 
 * adapted for this gui in controlling the relevant controls and handling changes more directly.
 */
class AoSystem
{
	public:
		// THIS CLASS IS NOT COPYABLE.
		AoSystem& operator=(const AoSystem&) = delete;
		AoSystem (const AoSystem&) = delete;

		AoSystem( bool aoSafemode );

		// standard functions for gui elements
		void initialize( POINT& pos, cToolTips& toolTips, AuxiliaryWindow* master, int& id );
		void rearrange( UINT width, UINT height, fontMap fonts );
		HBRUSH handleColorMessage( CWnd* hwnd, CDC* cDC );
		// configs
		void handleNewConfig( std::ofstream& newFile );
		void handleSaveConfig(std::ofstream& saveFile);
		void handleOpenConfig(std::ifstream& openFile, Version ver);
		// macros
		void forceDacs( DoCore& ttls, DoSnapshot initSnap);
		void zeroDacs( DoCore& ttls, DoSnapshot initSnap);
		// Setting system settings, mostly non-crucial functionality.
		
		void handleRoundToDac( MainWindow* menu );
		void updateEdits( );
		void setDefaultValue( UINT dacNum, double val );
		void setName( int dacNumber, std::string name, cToolTips& toolTips, AuxiliaryWindow* master );
		void setNote ( int dacNumber, std::string note, cToolTips& toolTips, AuxiliaryWindow* master );
		bool isValidDACName( std::string name );
		void setMinMax( int dacNumber, double min, double max );
		void fillPlotData( UINT variation, std::vector<std::vector<pPlotDataVec>> dacData );
		void handleEditChange( UINT dacNumber );
		// processing to determine how dac's get set
		void handleSetDacsButtonPress( DoCore& ttls, bool useDefault=false );
		void setDacCommandForm( AoCommandForm command );
		void setDacStatusNoForceOut(std::array<double, 24> status);
		void prepareDacForceChange(int line, double voltage, DoCore& ttls);
		void setDacTriggerEvents( DoCore& ttls, UINT variation );
		void interpretKey( std::vector<parameterType>& variables, std::string& warnings );
		void organizeDacCommands( UINT variation );
		void handleDacScriptCommand( AoCommandForm command, std::string name, std::vector<parameterType>& vars, 
									 DoCore& ttls );
		void findLoadSkipSnapshots( double time, std::vector<parameterType>& variables, UINT variation );
		// formatting data and communicating with the underlying daqmx api for actual communicaition with the cards.
		void makeFinalDataFormat( UINT variation);
		void writeDacs( UINT variation, bool loadSkip );
		void startDacs( );
		void configureClocks( UINT variation, bool loadSkip );
		void stopDacs();
		void resetDacEvents( );
		void initializeDataObjects( UINT cmdNum );
		void prepareForce( );
		void standardNonExperiemntStartDacsSequence( );		
		void setSingleDac( UINT dacNumber, double val, DoCore& ttls, DoSnapshot initSnap);
		// checks
		void checkTimingsWork( UINT variation );
		void checkValuesAgainstLimits(UINT variation );
		// ask for info
		std::string getSystemInfo( );
		std::string getDacSequenceMessage( UINT variation );
		// getters
		double getDefaultValue( UINT dacNum );
		unsigned int getNumberSnapshots( UINT variation );
		std::string getName( int dacNumber );
		std::string getNote ( int dacNumber );
		ULONG getNumberEvents( UINT variation );
		int getDacIdentifier( std::string name );
		static int getBasicDacIdentifier (std::string name);

		double getDacValue( int dacNumber );
		unsigned int getNumberOfDacs( );
		std::pair<double, double> getDacRange( int dacNumber );
		std::array<AoInfo, 24> getDacInfo ( );
		std::array<double, 24> getFinalSnapshot( );
		bool handleArrow ( CWnd* focus, bool up );
		ExpWrap<std::vector<AoSnapshot>> getSnapshots ( );
		ExpWrap<std::array<std::vector<double>, 3>> getFinData ( );

	private:
		void setForceDacEvent (int line, double val, DoCore& ttls, UINT variation);

		Control<CStatic> dacTitle;
		Control<CleanPush> dacSetButton;
		Control<CleanPush> zeroDacsButton;
		Control<CleanCheck> quickChange;
		std::array<AnalogOutput, 24> outputs;

		std::vector<AoCommandForm> dacCommandFormList;
		ExpWrap<std::vector<AoCommand>> dacCommandList;
		ExpWrap<std::vector<AoSnapshot>> dacSnapshots, loadSkipDacSnapshots;
		ExpWrap<std::array<std::vector<double>, 3>> finalFormatDacData, loadSkipDacFinalFormat;
		std::pair<USHORT, USHORT> dacTriggerLine;

		double dacTriggerTime;
		bool roundToDacPrecision;

		// For DACboard0 (tasks are a national instruments DAQmx thing)
		TaskHandle analogOutTask0 = NULL;
		const std::string board0Name = "dev5";
		// task for DACboard1
		TaskHandle analogOutTask1 = NULL;
		const std::string board1Name = "dev4";
		// task for DACboard2
		TaskHandle analogOutTask2 = NULL;
		const std::string board2Name = "dev6";

		/// digital in lines not used at the moment.
		TaskHandle digitalDac_0_00 = NULL;
		TaskHandle digitalDac_0_01 = NULL;

		DaqMxFlume daqmx;
};


