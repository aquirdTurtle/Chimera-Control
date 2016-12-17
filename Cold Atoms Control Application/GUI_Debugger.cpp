
#include "stdafx.h"
#include "GUI_Debugger.h"
#include "fonts.h"
/*
GUI_Debugger::GUI_Debugger(int& idStart)
{
	this->reportAllScriptCommands.ID = idStart;
	this->reportNiawgScriptOption.ID = idStart + 1;
	idStart += 2;
}

bool GUI_Debugger::initialize(POINT pos, HWND parent)
{
	this->reportAllScriptCommands.position = { pos.x, pos.y, pos.x + 480, pos.y + 20 };
	//reportAllScriptCommands.parent.Create("Report All Script Commands Read by the Program", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_RIGHT | BS_CHECKBOX,
	//											this->reportAllScriptCommands.position, 
	//											CWnd::FromHandle(parent), this->reportAllScriptCommands.ID);
	//reportAllScriptCommands.parent.SetFont(CFont::FromHandle(sNormalFont));
	pos.y += 20;
	this->reportNiawgScriptOption.position = { pos.x, pos.y, pos.x + 480, pos.y + 20 };
	//reportNiawgScriptOption.parent.Create("Report The Literal NIAWG Script", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_RIGHT | BS_CHECKBOX,
	//									  this->reportNiawgScriptOption.position,
	//									  CWnd::FromHandle(parent), this->reportNiawgScriptOption.ID);
	//reportNiawgScriptOption.parent.SetFont(CFont::FromHandle(sNormalFont));
	pos.y += 20;
	return true;
}

debuggingOptions GUI_Debugger::getCurrentOptions()
{
	return this->currentOptions;
}

bool GUI_Debugger::handleClick(HWND parent, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg != WM_COMMAND)
	{
		return false;
	}
	DWORD id = GetDlgCtrlID(parent);
//	int id = LOWORD(wParam);
	if (id == this->reportAllScriptCommands.ID)
	{
		if (reportAllScriptCommands.GetCheck())
		{
			reportAllScriptCommands.SetCheck(FALSE);
			this->currentOptions.reportCommandList = false;
		}
		else
		{
			reportAllScriptCommands.SetCheck(TRUE);
			this->currentOptions.reportCommandList = true;
		}
		return true;
	}
	else if (id == this->reportNiawgScriptOption.ID)
	{
		if (reportNiawgScriptOption.GetCheck())
		{
			reportNiawgScriptOption.SetCheck(FALSE);
			this->currentOptions.reportNiawgScript = false;
		}
		else
		{
			reportNiawgScriptOption.SetCheck(TRUE);
			this->currentOptions.reportNiawgScript = true;
		}
		return true;
	}
	// if not handled.
	return false;
}
*/