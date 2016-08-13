#pragma once
#include "windows.h"
#include "DAC_System.h"
#include <unordered_map>
struct dacInputStruct
{
	DAC_System* dacs;
	std::vector<CToolTipCtrl*> toolTips;
	MasterWindow* master;
};

INT_PTR CALLBACK viewAndChangeDAC_NamesProcedure(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);