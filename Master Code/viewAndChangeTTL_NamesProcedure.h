#pragma once
#include "windows.h"
#include "TTL_System.h"
#include <unordered_map>

struct ttlInputStruct
{
	TtlSystem* ttls;
	std::vector<CToolTipCtrl*> toolTips;
	MasterWindow* master;
};

INT_PTR CALLBACK viewAndChangeTTL_NamesProcedure(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

