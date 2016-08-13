#pragma once
#include "stdafx.h"
#include <unordered_map>
#include "fonts.h"
//#include "MasterWindow.h"
class MasterWindow;

class HwndControl
{
	public:
		HWND hwnd;
		RECT position;
		int ID;

		int colorState = 0;
		bool setToolTip(std::string text, std::unordered_map<HWND, std::string>& masterText);
	private:
		HWND toolTipHwnd;
		bool toolTipIsSet;
};

template <class ControlType> class ClassControl
{
	public:
		ControlType parent;
		RECT position;
		int ID;
		int colorState = 0;
		//bool ClassControl<CEdit>::setToolTip(std::string text, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master)
		bool ClassControl<ControlType>::setToolTip(std::string text, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master);
	private:
		//HWND toolTipHwnd;
		int toolTipID;
		CToolTipCtrl toolTip;
		bool toolTipIsSet;
};

/// template function for the class control system
template <class Parent> bool ClassControl<Parent>::setToolTip(std::string text, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master)
{
	if (!this->toolTipIsSet)
	{
		this->toolTipID = toolTips.size();
		toolTips.push_back(new CToolTipCtrl);
		toolTips.back()->Create(master, TTS_ALWAYSTIP | TTS_BALLOON);
		toolTips.back()->SetMaxTipWidth(500);
		toolTips.back()->SetTipBkColor(0x000000);
		toolTips.back()->SetTipTextColor(0xe0e0d0);
		toolTips.back()->SetFont(CFont::FromHandle(sNormalFont));
		toolTips.back()->SetDelayTime(TTDT_AUTOPOP, 30000);
		toolTips.back()->AddTool(&this->parent, text.c_str());
		this->toolTipIsSet = true;
	}
	else
	{
		toolTips[this->toolTipID]->DelTool(&this->parent);
		toolTips[this->toolTipID]->AddTool(&this->parent, text.c_str());
	}
	return true;
}



