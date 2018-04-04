#pragma once
#include <array>
#include <string>
#include <unordered_map>

#include "Control.h"
#include "VariableSystem.h"
#include "DioSystem.h"
#include "DaqMxFlume.h"
#include "AoStructures.h"

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
		std::string getSystemInfo( );
		void handleNewConfig( std::ofstream& newFile );
		void handleSaveConfig(std::ofstream& saveFile);
		void handleOpenConfig(std::ifstream& openFile, int versionMajor, int versionMinor, DioSystem* ttls);
		void initialize( POINT& pos, cToolTips& toolTips, AuxiliaryWindow* master, int& id );
		std::string getDacSequenceMessage(UINT variation, UINT seqNum );
		void handleSetDacsButtonPress( DioSystem* ttls, bool useDefault=false );
		void updateEdits( );
		void setDacCommandForm( AoCommandForm command, UINT seqNum );
		void setForceDacEvent( int line, double val, DioSystem* ttls, UINT variation, UINT seqNum );
		void handleRoundToDac(CMenu& menu);
		void setDacStatusNoForceOut(std::array<double, 24> status);
		void prepareDacForceChange(int line, double voltage, DioSystem* ttls);
		void stopDacs();
		void setDacTriggerEvents( DioSystem* ttls, UINT variation, UINT seqNum );
		void interpretKey( std::vector<std::vector<variableType>>& variables, std::string& warnings );
		void organizeDacCommands(UINT variation, UINT seqNum );
		void makeFinalDataFormat(UINT variation, UINT seqNum );
		void writeDacs( UINT variation, UINT seqNum, bool loadSkip );
		void startDacs();
		void configureClocks( UINT variation, UINT seqNum, bool loadSkip );
		void setDefaultValue(UINT dacNum, double val);
		double getDefaultValue(UINT dacNum);

		unsigned int getNumberSnapshots(UINT variation, UINT seqNum );
		void checkTimingsWork(UINT variation, UINT seqNum );
		void setName(int dacNumber, std::string name, cToolTips& toolTips, AuxiliaryWindow* master);
		std::string getName(int dacNumber);
		std::array<std::string, 24> getAllNames();
		ULONG getNumberEvents(UINT variation, UINT seqNum );
		void handleDacScriptCommand( AoCommandForm command, std::string name, std::vector<UINT>& dacShadeLocations, 
									 std::vector<variableType>& vars, DioSystem* ttls, UINT seqNum );
		
		int getDacIdentifier(std::string name);
		double getDacValue(int dacNumber);
		unsigned int getNumberOfDacs();
		std::pair<double, double> getDacRange(int dacNumber);
		void setMinMax(int dacNumber, double min, double max);
		void shadeDacs(std::vector<unsigned int>& dacShadeLocations);
		void unshadeDacs();
		void rearrange(UINT width, UINT height, fontMap fonts);
		bool isValidDACName(std::string name);
		HBRUSH handleColorMessage(CWnd* hwnd, brushMap brushes, rgbMap rgbs, CDC* cDC);
		void fillPlotData( UINT variation, std::vector<std::vector<pPlotDataVec>> dacData, 
						   std::vector<std::vector<double>> finTimes );
		void resetDacEvents();
		void initDacObjs( UINT totalSequenceNumber );
		std::array<double, 24> getDacStatus();
		std::array<double, 24> getFinalSnapshot();
		void checkValuesAgainstLimits(UINT variation, UINT seqNum );
		void prepareForce();
		double roundToDacResolution(double);
		void findLoadSkipSnapshots( double time, std::vector<variableType>& variables, UINT variation, UINT seqNum );
		void handleEditChange( UINT dacNumber );
		std::vector<std::vector<std::vector<AoSnapshot>>> getSnapshots( );
		std::vector<std::vector<std::array<std::vector<double>, 3>>> getFinData( );
	private:
		Control<CStatic> dacTitle;
		Control<CButton> dacSetButton;
		Control<CButton> zeroDacs;
		std::array<Control<CStatic>, 24> dacLabels;
		std::array<Control<CEdit>, 24> breakoutBoardEdits;
		std::array<double, 24> dacValues;
		std::array<std::string, 24> dacNames;
		std::array<double, 24> dacMinVals;
		std::array<double, 24> dacMaxVals;
		std::array<double, 24> defaultVals;
		const double dacResolution;
		std::vector<std::vector<AoCommandForm>> dacCommandFormList;
		// first = sequence, 2nd = variation
		std::vector<std::vector<std::vector<AoCommand>>> dacCommandList;
		std::vector<std::vector<std::vector<AoSnapshot>>> dacSnapshots, loadSkipDacSnapshots;
		std::vector<std::vector<std::array<std::vector<double>, 3>>> finalFormatDacData, loadSkipDacFinalFormat;
		std::pair<USHORT, USHORT> dacTriggerLine;

		double dacTriggerTime;
		bool roundToDacPrecision;

		// task for DACboard0 (tasks are a national instruments DAQmx thing)
		TaskHandle analogOutTask0 = NULL;
		// task for DACboard1
		TaskHandle analogOutTask1 = NULL;
		// task for DACboard2
		TaskHandle analogOutTask2 = NULL;

		/// digital in lines not used at the moment.
		TaskHandle digitalDac_0_00 = NULL;
		TaskHandle digitalDac_0_01 = NULL;

		DaqMxFlume daqmx;
};


