#include "stdafx.h"
#include "Control.h"
#include <unordered_map>

bool HwndControl::setToolTip(std::string text, std::unordered_map<HWND, std::string>& masterTexts)
{
	HWND parent = GetParent(this->hwnd);
	if (parent == NULL || text == "" || this->hwnd == NULL)
	{
		return false;
	}
	// if the tooltip has already been set, delete it before creating the next one.
	if (this->toolTipIsSet)
	{
		TOOLINFO deletedTool = { 0 };
		deletedTool.cbSize = sizeof(deletedTool);
		deletedTool.hwnd = parent;
		deletedTool.uId = (UINT_PTR)this->hwnd;
		SendMessage(this->toolTipHwnd, TTM_DELTOOL, 0, (LPARAM)&deletedTool);
	}
	// Create the tooltip.
	this->toolTipHwnd = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		parent, NULL, GetModuleHandle(NULL), NULL);

	if (this->toolTipHwnd == NULL)
	{
		// failed
		return false;
	}

	// Associate the tooltip with the tool.
	TOOLINFO toolInfo = { 0 };
	toolInfo.cbSize = sizeof(toolInfo);
	toolInfo.hwnd = parent;
	toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	toolInfo.uId = (UINT_PTR)this->hwnd;
	// this structure can only handle text of size 80 or less.
	if (text.size() >= 80)
	{
		// set it to callback, meaning it requests the text at time of hovering, and let the master know what the text is.
		toolInfo.lpszText = LPSTR_TEXTCALLBACK;
		masterTexts[this->toolTipHwnd] = text;
	}
	else
	{
		toolInfo.lpszText = (LPSTR)text.c_str();
	}
	// add the tool info.
	SendMessage(this->toolTipHwnd, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
	// marker so that I know I need to delete something if I try to set this again.
	this->toolTipIsSet = true;
	// activate it
	SendMessage(this->toolTipHwnd, TTM_ACTIVATE, TRUE, 0);
	// make it have finite width. Aesthetic only.
	SendMessage(this->toolTipHwnd, TTM_SETMAXTIPWIDTH, 0, 300);
	// set this to maximum value.
	int test3 = 0x0000ffff;
	int test2 = SendMessage(this->toolTipHwnd, TTM_GETDELAYTIME, TTDT_AUTOPOP, 0);
	int result = SendMessage(this->toolTipHwnd, TTM_SETDELAYTIME, TTDT_AUTOPOP, 0x00009999);
	int test = SendMessage(this->toolTipHwnd, TTM_GETDELAYTIME, TTDT_AUTOPOP, 0);
	
	return true;
}

