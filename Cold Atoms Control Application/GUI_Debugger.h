#pragma once

#include "Control.h"
#include "stdafx.h"

struct debuggingOptions
{
	bool reportNiawgScript;
	bool reportCommandList;
};

class GUI_Debugger
{
	public:
		GUI_Debugger(int& idStart);
		bool initialize(POINT pos, HWND parent);
		debuggingOptions getCurrentOptions();
		bool handleClick(HWND parent, UINT msg, WPARAM wParam, LPARAM lParam);
	private:
		ClassControl<CButton> reportNiawgScriptOption;
		ClassControl<CButton> reportAllScriptCommands;
		debuggingOptions currentOptions;
};
