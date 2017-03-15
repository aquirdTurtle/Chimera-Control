#pragma once
#include "Control.h"

struct debugInfo
{
	bool showReadProgress;
	bool showWriteProgress;
	bool showCorrectionTimes;
	bool outputNiawgMachineScript;
	bool outputNiawgHumanScript;
	bool outputAgilentScript;
	bool outputExcessInfo;
};

class DebuggingOptionsControl
{
	public:
		void initialize(int& idStart, POINT& loc, CWnd* parent, std::unordered_map<std::string, CFont*> fonts,
			std::vector<CToolTipCtrl*>& tooltips);
		void handleEvent(UINT id, MainWindow* comm);
		debugInfo getOptions();
		void setOptions(debugInfo options);
	private:
		Control<CStatic> header;
		Control<CButton> readProgress;
		Control<CButton> writeProgress;
		Control<CButton> correctionTimes;
		Control<CButton> niawgScript;
		Control<CButton> outputAgilentScript;
		Control<CButton> niawgMachineScript;
		Control<CButton> excessInfo;
		debugInfo currentOptions;
};