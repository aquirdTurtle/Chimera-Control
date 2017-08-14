#pragma once
#include <array>
#include <string>
#include <unordered_map>

#include "Control.h"
#include "VariableSystem.h"
#include "TtlSystem.h"
#include "KeyHandler.h"
#include "miscellaneousCommonFunctions.h"

#include "nidaqmx2.h"

struct DacComplexEvent
{
	unsigned short line;
	timeType time;
	std::string initVal;
	std::string finalVal;
	std::string rampTime;
	std::string rampInc;
};

struct DacIndividualEvent
{
	unsigned short line;
	double time;
	double value;
};

struct DacSnapshot
{
	double time;
	std::array<double, 24> dacValues;
};


/*
]- The DacSystem is meant to be a constant class but it currently doesn't actually prevent the user from making multiple copies of the object.
]- This class is based off of the DAC.bas module in the original VB6 code, of course adapted for this gui in controlling the relevant controls
]- and handling changes more directly.
*/
class DacSystem
{
	public:
		DacSystem();
		void handleSaveConfig(std::ofstream& saveFile);
		void handleOpenConfig(std::ifstream& openFile, double version, TtlSystem* ttls);
		void abort();
		void initialize( POINT& pos, cToolTips& toolTips, DeviceWindow* master, int& id );
		std::string getDacSequenceMessage(UINT var);
		void handleButtonPress(TtlSystem* ttls);
		void setDacComplexEvent(int line, timeType time, std::string initVal, std::string finalVal, std::string rampTime, std::string rampInc);
		void setForceDacEvent( int line, double val, TtlSystem* ttls, UINT var );
		void handleRoundToDac(CMenu& menu);
		void setDacStatusNoForceOut(std::array<double, 24> status);
		void prepareDacForceChange(int line, double voltage, TtlSystem* ttls);
		void stopDacs();
		void configureClocks(UINT var);
		void setDacTriggerEvents( TtlSystem* ttls, UINT var );
		void interpretKey(key variationKey, std::vector<variable>& vars, std::string& warnings);
		void analyzeDacCommands(UINT var);
		void makeFinalDataFormat(UINT var);
		void writeDacs(UINT var);
		void startDacs();

		void setDefaultValue(UINT dacNum, double val);
		double getDefaultValue(UINT dacNum);

		unsigned int getNumberSnapshots(UINT var);
		void checkTimingsWork(UINT var);
		void setName(int dacNumber, std::string name, cToolTips& toolTips, DeviceWindow* master);
		std::string getName(int dacNumber);
		std::array<std::string, 24> getAllNames();
		std::string getErrorMessage(int errorCode);
		ULONG getNumberEvents(UINT var);
		void handleDacScriptCommand( timeType time, std::string name, std::string initVal, std::string finalVal,
									  std::string rampTime, std::string rampInc, std::vector<unsigned int>& dacShadeLocations, 
									  std::vector<variable>& vars, TtlSystem* ttls);

		std::string getDacSystemInfo();

		void handleEditChange(unsigned int dacNumber);
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
		void resetDacEvents();
		std::array<double, 24> getDacStatus();
		std::array<double, 24> getFinalSnapshot();
		void checkValuesAgainstLimits(UINT var);
		void prepareForce();
		double roundToDacResolution(double);

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
		std::vector<DacComplexEvent> dacComplexEventsList;
		// the first vector is for each variation.
		std::vector<std::vector<DacIndividualEvent>> dacIndividualEvents;
		std::vector<std::vector<DacSnapshot>> dacSnapshots;
		std::vector<std::array<std::vector<double>, 3>> finalFormattedData;

		std::pair<USHORT, USHORT> dacTriggerLine;

		double dacTriggerTime;
		bool roundToDacPrecision;

		// task for DACboard0 (tasks are a national instruments DAQmx thing)
		TaskHandle staticDac0 = 0;
		// task for DACboard1
		TaskHandle staticDac1 = 0;
		// task for DACboard2
		TaskHandle staticDac2 = 0;
		/// I'm confused about the following two lines. I don't think our hardware even supports digital inputs, the 
		/// cards pretty clearly just say analog out.
		// another task for DACboard0 (reads in a digital line)
		TaskHandle digitalDac_0_00 = 0;
		// another task for DACboard0 (reads in a digital line)
		TaskHandle digitalDac_0_01 = 0;
		
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