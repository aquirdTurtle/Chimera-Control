#pragma once
#include <array>
#include <string>
#include "Control.h"
#include "nidaqmx2.h"
#include <unordered_map>
#include "VariableSystem.h"
#include "TTL_System.h"

struct DAC_ComplexEvent
{
	unsigned int line;
	std::pair<std::string, double> time;
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
		DacSystem::DacSystem(int& startID);
		DacSystem::DacSystem();
		~DacSystem();
		void initialize(POINT& upperLeftHandCornerPosition, HWND windowHandle, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master);
		void handleButtonPress(TtlSystem* ttls);
		void setDacComplexEvent(int line, std::pair<std::string, double> time, std::string initVal, std::string finalVal, std::string rampTime, std::string rampInc, TtlSystem* ttls);
		void setForceDacEvent( int line, double val, TtlSystem* ttls );
		void prepareDacForceChange(int line, double voltage, TtlSystem* ttls);
		void stopDacs();
		void resetDacs();
		void configureClocks();
		void interpretKey(std::unordered_map<std::string, std::vector<double>> key, unsigned int variationNumber, std::vector<variable> vars);
		void analyzeDAC_Commands();
		void makeFinalDataFormat();
		void writeDacs();
		void startDacs();

		void setName(int dacNumber, std::string name, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master);
		std::string getName(int dacNumber);
		std::array<std::string, 24> getAllNames();
		std::string getErrorMessage(int errorCode);
		
		void DacSystem::handleDAC_ScriptCommand(std::pair<std::string, long> time, std::string name, std::string initVal, std::string finalVal, 
												 std::string rampTime, std::string rampInc, std::vector<unsigned int>& dacShadeLocations, 
												 std::vector<variable> vars, TtlSystem* ttls);

		void handleEditChange(unsigned int dacNumber);
		int getDAC_Identifier(std::string name);
		double getDAC_Value(int dacNumber);
		unsigned int getNumberOfDACs();
		
		void shadeDacs(std::vector<unsigned int>& dacShadeLocations);
		void unshadeDacs();

		bool isValidDACName(std::string name);

		HBRUSH handleColorMessage(CWnd* hwnd, std::unordered_map<std::string, HBRUSH> brushes, std::unordered_map<std::string, COLORREF> rgbs, CDC* cDC);
		void resetDACEvents();
		
	private:

		Control<CStatic> dacTitle;
		Control<CButton> dacSetButton;
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