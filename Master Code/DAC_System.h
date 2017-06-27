#pragma once
#include <array>
#include <string>
#include "Control.h"
#include "nidaqmx2.h"
#include <unordered_map>
#include "VariableSystem.h"
#include "TTL_System.h"
#include "KeyHandler.h"

struct DAC_ComplexEvent
{
	unsigned int line;
	timeType time;
	std::string initVal;
	std::string finalVal;
	std::string rampTime;
	std::string rampInc;
};

struct DAC_IndividualEvent
{
	unsigned int line;
	double time;
	double value;
};

struct DAC_Snapshot
{
	double time;
	std::array<double, 24> dacValues;
};

/*
]- The DacSystem is meant to be a singleton class but it currently doesn't actually prevent the user from making multiple copies of the object.
]- This class is based off of the DAC.bas module in the original VB6 code, of course adapted for this gui in controlling the relevant controls
]- and handling changes more directly.
*/
class DacSystem
{
	public:
		DacSystem::DacSystem();
		void abort();
		void initialize( POINT& pos, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master, int& id );
		std::string getDacSequenceMessage();
		void handleButtonPress(TtlSystem* ttls);
		void setDacComplexEvent(int line, timeType time, std::string initVal, std::string finalVal, std::string rampTime, std::string rampInc);
		void setForceDacEvent( int line, double val, TtlSystem* ttls );

		void setDacStatusNoForceOut(std::array<double, 24> status);
		void prepareDacForceChange(int line, double voltage, TtlSystem* ttls);
		void stopDacs();
		void configureClocks();
		void setDacTtlTriggerEvents( TtlSystem* ttls );
		void interpretKey(key variationKey, unsigned int variationNumber, std::vector<variable> vars);
		void analyzeDAC_Commands();
		void makeFinalDataFormat();
		void writeDacs();
		void startDacs();
		void rearrange();
		void setName(int dacNumber, std::string name, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master);
		std::string getName(int dacNumber);
		std::array<std::string, 24> getAllNames();
		std::string getErrorMessage(int errorCode);
		
		void DacSystem::handleDAC_ScriptCommand(timeType time, std::string name, std::string initVal, std::string finalVal,
												 std::string rampTime, std::string rampInc, std::vector<unsigned int>& dacShadeLocations, 
												 std::vector<variable> vars, TtlSystem* ttls);

		std::string getDacSystemInfo();

		void handleEditChange(unsigned int dacNumber);
		int getDAC_Identifier(std::string name);
		double getDAC_Value(int dacNumber);
		unsigned int getNumberOfDACs();
		
		void shadeDacs(std::vector<unsigned int>& dacShadeLocations);
		void unshadeDacs();
		
		void rearrange(UINT width, UINT height, fontMap fonts);

		bool isValidDACName(std::string name);

		HBRUSH handleColorMessage(CWnd* hwnd, std::unordered_map<std::string, HBRUSH> brushes, std::unordered_map<std::string, COLORREF> rgbs, CDC* cDC);
		void resetDACEvents();
		std::array<double, 24> getDacStatus();
		std::array<double, 24> getFinalSnapshot();
	private:
		Control<CStatic> dacTitle;
		Control<CButton> dacSetButton;
		Control<CButton> zeroDacs;
		std::array<Control<CStatic>, 24> dacLabels;
		std::array<Control<CEdit>, 24> breakoutBoardEdits;
		std::array<double, 24> dacValues;
		//std::array<bool, 24> dacShadeStatus;
		std::array<std::string, 24> dacNames;

		std::vector<DAC_ComplexEvent> dacComplexEventsList;
		std::vector<DAC_IndividualEvent> dacIndividualEvents;
		std::vector<DAC_Snapshot> dacSnapshots;
		std::array<std::vector<double>, 3> finalFormattedData;

		std::array<std::pair<unsigned int, unsigned int>, 3> dacTriggerLines;
		double dacTriggerTime;

		// task for DACboard0 (tasks are a national instruments DAQmx thing)
		TaskHandle staticDac0 = 0;
		// task for DACboard1
		TaskHandle staticDac1 = 0;
		// task for DACboard2
		TaskHandle staticDac2 = 0;
		/// I'm confused about the following two lines. I don't think our hardware even supports digital inputs, the cards
		/// pretty clearly just say analog out.
		// another task for DACboard0 (reads in a digital line)
		TaskHandle digitalDAC_0_00 = 0;
		// another task for DACboard0 (reads in a digital line)
		TaskHandle digitalDAC_0_01 = 0;
		
		/// My wrappers for all of the daqmx functions that I use currently. If I needed to use another function, I'd 
		/// create another wrapper!

		// note that DAQ stands for Data Aquisition (software). It's not a typo!
		void daqCreateTask( const char* taskName, TaskHandle& handle );
		void daqCreateAOVoltageChan( TaskHandle taskHandle, const char physicalChannel[], 
									 const char nameToAssignToChannel[], float64 minVal, float64 maxVal, int32 units, 
									 const char customScaleName[] );
		void daqCreateDIChan( TaskHandle taskHandle, const char lines[], const char nameToAssignToLines[], 
							  int32 lineGrouping );
		void daqStopTask( TaskHandle handle );
		void daqConfigSampleClkTiming( TaskHandle taskHandle, const char source[], float64 rate, int32 activeEdge, 
								  int32 sampleMode, uInt64 sampsPerChan );
		void daqWriteAnalogF64( TaskHandle handle, int32 numSampsPerChan, bool32 autoStart, float64 timeout, 
								bool32 dataLayout, const float64 writeArray[], int32 *sampsPerChanWritten);
		void daqStartTask( TaskHandle handle );



};