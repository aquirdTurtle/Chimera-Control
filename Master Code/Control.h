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
		rearrange(std::string cameraMode, std::string trigMode, int width, int height, std::unordered_map<std::string, CFont*> fonts)
		void ClassControl<ControlType>::rearrange(std::string cameraMode, std::string trigMode, int width, int height,
												  std::unordered_map<std::string, CFont*> fonts);
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



template <class controlType> void ClassControl<controlType>::rearrange(std::string cameraMode, std::string trigMode, int width, int height,
																  std::unordered_map<std::string, CFont*> fonts)
{
	// make sure the control has been initialized
	if (!m_hWnd)
	{
		return;
	}
	// the only extra part for this specialization:
	/// Set Positions
	double widthScale = width / 1920.0;
	double heightScale = height / 997.0;
	SetItemHeight(-1, heightScale * 25);
	// extra heigh added to certain controls based on random things like the trigger mode.
	double extraHeight = 0;
	// The last check here is that the mode is affected by the trigger and that the control in question gets drawn in this mode.
	if (trigMode == "External" && triggerModeSensitive && ((cameraMode == "Kinetic Series Mode" && seriesPos.bottom != -1)
		|| cameraMode == "Accumulation Mode" && amPos.bottom != -1))
	{
		extraHeight += -25;
	}
	// handle simple case.
	if (sPos.bottom != 0 || sPos.top != 0)
	{
		ShowWindow(SW_SHOW);
		RECT position = { widthScale * sPos.left, heightScale * (sPos.top + extraHeight), widthScale * sPos.right,
			heightScale * sPos.bottom };
		MoveWindow(&position, TRUE);
	}
	else if (cameraMode == "Kinetic Series Mode")
	{
		if (seriesPos.left == -1 || (triggerModeSensitive == -1 && trigMode == "External"))
		{
			ShowWindow(SW_HIDE);
		}
		else
		{
			ShowWindow(SW_SHOW);
			RECT position = { widthScale * seriesPos.left, heightScale * (seriesPos.top + extraHeight), widthScale * seriesPos.right,
				heightScale * (seriesPos.bottom + extraHeight) };
			MoveWindow(&position, TRUE);
		}
	}
	else if (cameraMode == "Video Mode")
	{
		if (videoPos.left == -1 || (triggerModeSensitive == -1 && trigMode == "External"))
		{
			ShowWindow(SW_HIDE);
		}
		else
		{
			ShowWindow(SW_SHOW);
			RECT position = { widthScale * videoPos.left, heightScale * (videoPos.top + extraHeight), widthScale * videoPos.right,
				heightScale * (videoPos.bottom + extraHeight) };
			MoveWindow(&position, TRUE);
		}
	}
	else if (cameraMode == "Accumulation Mode")
	{
		if (amPos.left == -1 || (triggerModeSensitive == -1 && trigMode == "External"))
		{
			ShowWindow(SW_HIDE);
		}
		else
		{
			ShowWindow(SW_SHOW);
			RECT position = { widthScale * amPos.left, heightScale * (amPos.top + extraHeight),
				widthScale * amPos.right,
				heightScale * (amPos.bottom + extraHeight) };
			MoveWindow(&position, TRUE);
		}
	}
	/// Set Fonts
	if (fontType == "Normal")
	{
		if (widthScale * heightScale > 0.8)
		{
			SetFont(fonts["Normal Font Max"]);
		}
		else if (widthScale * heightScale > 0.6)
		{
			SetFont(fonts["Normal Font Med"]);
		}
		else
		{
			SetFont(fonts["Normal Font Small"]);
		}
	}
	else if (fontType == "Code")
	{
		if (widthScale * heightScale > 0.8)
		{
			SetFont(fonts["Code Font Max"]);
		}
		else if (widthScale * heightScale > 0.6)
		{
			SetFont(fonts["Code Font Med"]);
		}
		else
		{
			SetFont(fonts["Code Font Small"]);
		}
	}
	else if (fontType == "Heading")
	{
		if (widthScale * heightScale > 0.8)
		{
			SetFont(fonts["Heading Font Max"]);
		}
		else if (widthScale * heightScale > 0.6)
		{
			SetFont(fonts["Heading Font Med"]);
		}
		else
		{
			SetFont(fonts["Heading Font Small"]);
		}
	}
	else if (this->fontType == "Large")
	{
		if (widthScale * heightScale > 0.8)
		{
			SetFont(fonts["Larger Font Max"]);
		}
		else if (widthScale * heightScale > 0.6)
		{
			SetFont(fonts["Larger Font Med"]);
		}
		else
		{
			SetFont(fonts["Larger Font Small"]);
		}
	}
	else if (fontType == "Small")
	{
		if (widthScale * heightScale > 0.8)
		{
			SetFont(fonts["Smaller Font Max"]);
		}
		else if (widthScale * heightScale > 0.6)
		{
			SetFont(fonts["Smaller Font Med"]);
		}
		else
		{
			SetFont(fonts["Smaller Font Small"]);
		}
	}
}
