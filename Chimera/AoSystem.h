// created by Mark O. Brown
#pragma once
#include <array>
#include <string>
#include <unordered_map>

#include "Control.h"
#include "myButton.h"
#include "ParameterSystem.h"
#include "DioSystem.h"
#include "DaqMxFlume.h"
#include "AoStructures.h"
#include "AnalogOutput.h"

#include "Version.h"

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
		void forceDacs( DioSystem* ttls );
		void zeroDacs( DioSystem* ttls );
		// Setting system settings, mostly non-crucial functionality.
		
		void handleRoundToDac( MainWindow* menu );
		void updateEdits( );
		void shadeDacs( std::vector<unsigned int>& dacShadeLocations );
		void unshadeDacs( );
		void setDefaultValue( UINT dacNum, double val );
		void setName( int dacNumber, std::string name, cToolTips& toolTips, AuxiliaryWindow* master );
		void setNote ( int dacNumber, std::string note, cToolTips& toolTips, AuxiliaryWindow* master );
		bool isValidDACName( std::string name );
		void setMinMax( int dacNumber, double min, double max );
		void fillPlotData( UINT variation, std::vector<std::vector<pPlotDataVec>> dacData,
						   std::vector<std::vector<double>> finTimes );
		void handleEditChange( UINT dacNumber );
		// processing to determine how dac's get set
		void handleSetDacsButtonPress( DioSystem* ttls, bool useDefault=false );
		void setDacCommandForm( AoCommandForm command, UINT seqNum );
		void setForceDacEvent( int line, double val, DioSystem* ttls, UINT variation, UINT seqNum );		
		void setDacStatusNoForceOut(std::array<double, 24> status);
		void prepareDacForceChange(int line, double voltage, DioSystem* ttls);
		void setDacTriggerEvents( DioSystem& ttls, UINT variation, UINT seqNum, UINT totalVariations );
		void interpretKey( std::vector<std::vector<parameterType>>& variables, std::string& warnings );
		void organizeDacCommands( UINT variation, UINT seqNum );
		void handleDacScriptCommand( AoCommandForm command, std::string name, std::vector<UINT>& dacShadeLocations,
									 std::vector<parameterType>& vars, DioSystem& ttls, UINT seqNum );
		void findLoadSkipSnapshots( double time, std::vector<parameterType>& variables, UINT variation, UINT seqNum );
		// formatting data and communicating with the underlying daqmx api for actual communicaition with the cards.
		void makeFinalDataFormat( UINT variation, UINT seqNum );
		void writeDacs( UINT variation, UINT seqNum, bool loadSkip );
		void startDacs( );
		void configureClocks( UINT variation, UINT seqNum, bool loadSkip );
		void stopDacs();
		void resetDacEvents( );
		void initializeDataObjects( UINT sequenceNum, UINT cmdNum );
		void prepareForce( );
		void standardNonExperiemntStartDacsSequence( );		
		void setSingleDac( UINT dacNumber, double val, DioSystem* ttls );
		// checks
		void checkTimingsWork( UINT variation, UINT seqNum );
		void checkValuesAgainstLimits(UINT variation, UINT seqNum );
		// ask for info
		std::string getSystemInfo( );
		std::string getDacSequenceMessage( UINT variation, UINT seqNum );
		// getters
		double getDefaultValue( UINT dacNum );
		unsigned int getNumberSnapshots( UINT variation, UINT seqNum );
		std::string getName( int dacNumber );
		std::string getNote ( int dacNumber );
		ULONG getNumberEvents( UINT variation, UINT seqNum );
		int getDacIdentifier( std::string name );
		double getDacValue( int dacNumber );
		unsigned int getNumberOfDacs( );
		std::pair<double, double> getDacRange( int dacNumber );
		std::array<AoInfo, 24> getDacInfo ( );
		std::array<double, 24> getFinalSnapshot( );
		bool handleArrow ( CWnd* focus, bool up );
		ExpWrap<std::vector<AoSnapshot>> getSnapshots ( );
		ExpWrap<std::array<std::vector<double>, 3>> getFinData ( );
	private:
		Control<CStatic> dacTitle;
		Control<CleanButton> dacSetButton;
		Control<CleanButton> zeroDacsButton;
		Control<CleanCheck> quickChange;
		std::array<AnalogOutput, 24> outputs;

		std::vector<std::vector<AoCommandForm>> dacCommandFormList;
		// first = sequence, 2nd = variation
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


