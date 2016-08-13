#pragma once
#include <array>
#include <string>
#include "Control.h"
#include "nidaqmx2.h"
#include <unordered_map>
#include "VariableSystem.h"
#include "TTL_System.h"

struct DAC_EventInfo
{
	unsigned int line;
	std::pair<std::string, double> time;
	std::string initVal;
	std::string finalVal;
	std::string rampTime;
	std::string rampInc;
};

struct DAC_Event
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
]- The DAC_System is meant to be a singleton class but it currently doesn't actually prevent the user from making multiple copies of the object.
]- This class is based off of the DAC.bas module in the original VB6 code, of course adapted for this gui in controlling the relevant controls
]- and handling changes more directly.
*/
class DAC_System
{
	public:
		DAC_System::DAC_System(int& startID);
		DAC_System::DAC_System();
		~DAC_System();
		bool initialize(POINT& upperLeftHandCornerPosition, HWND windowHandle, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master);
		bool handleButtonPress(TTL_System* ttls);
		bool setDacEvent(int line, std::pair<std::string, double> time, std::string initVal, std::string finalVal, std::string rampTime, std::string rampInc, TTL_System* ttls);
		bool forceDacChange(int line, double voltage, TTL_System* ttls);
		bool stopDacs();
		bool resetDacs();
		bool configureClocks(long sampleNumber);
		bool interpretKey(std::unordered_map<std::string, std::vector<double>> key, unsigned int variationNumber, std::vector<variable> vars);
		bool analyzeDAC_Commands();
		bool makeFinalDataFormat();
		bool writeDacs();

		bool setName(int dacNumber, std::string name, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master);
		std::string getName(int dacNumber);
		std::array<std::string, 24> getAllNames();
		std::string getErrorMessage(int errorCode);
		
		bool DAC_System::handleDAC_ScriptCommand(std::pair<std::string, long> time, std::string name, std::string initVal, std::string finalVal, 
												 std::string rampTime, std::string rampInc, std::vector<unsigned int>& dacShadeLocations, 
												 std::vector<variable> vars, TTL_System* ttls);

		bool handleEditChange(unsigned int dacNumber);
		int getDAC_Identifier(std::string name);
		double getDAC_Value(int dacNumber);
		unsigned int getNumberOfDACs();
		
		bool shadeDacs(std::vector<unsigned int>& dacShadeLocations);
		bool unshadeDacs();

		bool isValidDACName(std::string name);

		HBRUSH handleColorMessage(CWnd* hwnd, std::unordered_map<std::string, HBRUSH> brushes, std::unordered_map<std::string, COLORREF> rgbs, CDC* cDC);
		bool resetDACEvents();
	private:
		ClassControl<CStatic> dacTitle;
		ClassControl<CButton> dacSetButton;
		std::array<ClassControl<CStatic>, 24> dacLabels;
		std::array<ClassControl<CEdit>, 24> breakoutBoardEdits;
		std::array<double, 24> dacValues;
		//std::array<bool, 24> dacShadeStatus;
		std::array<std::string, 24> dacNames;
		std::vector<DAC_EventInfo> dacEventInfoList;
		std::vector<DAC_Event> dacEvents;
		std::vector<DAC_Snapshot> dacSnapshots;
		std::array<std::vector<double>, 3> finalFormattedData;
		std::array<std::pair<unsigned int, unsigned int>, 3> dacTriggerLines;
		double dacTriggerTime;

		// task for DACboard0 (tasks are a national instruments DAQmx thing)
		TaskHandle staticDAC_0 = 0;
		// task for DACboard1
		TaskHandle staticDAC_1 = 0;
		// task for DACboard2
		TaskHandle staticDAC_2 = 0;
		// another task for DACboard0 (reads in a digital line)
		TaskHandle digitalDAC_0_00 = 0;
		// another task for DACboard0 (reads in a digital line)
		TaskHandle digitalDAC_0_01 = 0;

};