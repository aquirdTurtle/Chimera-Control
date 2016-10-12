#pragma once
#include "Control.h"

struct debuggingOptionsList
{
	bool outputNiawgMachineScriptSetting;
	bool outputNiawgHumanScript;
	bool outputAgilentScript;
};

class DebuggingOptionsControl
{
	public:
		void initialize(int& idStart, POINT& loc);
		bool handleEvent(HWND parent, UINT msg, WPARAM wParam, LPARAM lParam);
		debuggingOptionsList getOptions();
	private:
		HwndControl outputNiawgScript;
		HwndControl outputAgilentScript;
		HwndControl outputNiawgMachineScript;
		debuggingOptionsList currentOptions;
};