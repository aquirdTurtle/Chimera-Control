#pragma once
#include "Control.h"

struct debugOptions
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
		void initialize(int& idStart, POINT& loc, CWnd* parent);
		bool handleEvent(UINT id, MainWindow* mainWin);
		debugOptions getOptions();
		void setOptions(debugOptions options);
	private:
		Control<CStatic> header;
		Control<CButton> readProgress;
		Control<CButton> writeProgress;
		Control<CButton> correctionTimes;
		Control<CButton> niawgScript;
		Control<CButton> outputAgilentScript;
		Control<CButton> niawgMachineScript;
		Control<CButton> excessInfo;
		debugOptions currentOptions;
};