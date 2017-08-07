#pragma once
#include "stdafx.h"
#include <unordered_map>
#include "fonts.h"
#include "commonTypes.h"
#include "afxwin.h"

static HFONT toolTipFont = CreateFont(30, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
									  DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));

// a template class that inherits its template argument.
template <class ControlType> class Control : public ControlType
{
	public:
		// contains an assert that the inherited class inherits from CWnd.
		Control();
		
		// Standard Position: for objects that don't move around and have permanent positions in their windows.
		RECT sPos;
		// Kinetic Series Mode Positon: position for camera-mode-sensitive objects in kinetic series mode.
		RECT seriesPos;
		// Accumulate Mode Position
		RECT amPos;
		// video Mode Position
		RECT videoPos;
		fontType fontType = NormalFont;
		int triggerModeSensitive;
		//
		//int ID;
		int colorState = 0;
		void rearrange(int width, int height, fontMap fonts);
		void rearrange(std::string cameraMode, std::string trigMode, int width, int height, fontMap fonts);
		bool Control<ControlType>::setToolTip(std::string text, std::vector<CToolTipCtrl*>& toolTips, 
											  MasterWindow* master);
	private:
		int toolTipID;
		CToolTipCtrl toolTip;
		bool toolTipIsSet;
};



template<class ControlType> Control<ControlType>::Control()
{
	// assert that the template class is derived from CWnd. This doesn't actually do anything in run-time. It's also
	// probably redundant because of all the functionality designed around CWnd in this class, like the below function.
	ControlType obj;
	ASSERT((CWnd const*)&obj);
}

template <class ControlType> void Control<ControlType>::rearrange(int width, int height, fontMap fonts)
{
	rearrange("", "", width, height, fonts);
}

template <class ControlType> void Control<ControlType>::rearrange(std::string cameraMode, std::string trigMode, 
																  int width, int height, fontMap fonts)
{
	// make sure the control has been initialized
	if (!m_hWnd)
	{
		return;
	}
	/// Set Positions
	double widthScale = width / 1920.0;
	double heightScale = height / 997.0;
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
		RECT newPos = { widthScale * sPos.left, heightScale * (sPos.top + extraHeight), widthScale * sPos.right,
			heightScale * sPos.bottom };
		MoveWindow(&newPos, TRUE);
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
			RECT sPos = { widthScale * seriesPos.left, heightScale * (seriesPos.top + extraHeight), widthScale * seriesPos.right,
				heightScale * (seriesPos.bottom + extraHeight) };
			MoveWindow(&sPos, TRUE);
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
			RECT sPos = { widthScale * videoPos.left, heightScale * (videoPos.top + extraHeight), widthScale * videoPos.right,
				heightScale * (videoPos.bottom + extraHeight) };
			MoveWindow(&sPos, TRUE);
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
			RECT sPos = { widthScale * amPos.left, heightScale * (amPos.top + extraHeight),
				widthScale * amPos.right,
				heightScale * (amPos.bottom + extraHeight) };
			MoveWindow(&sPos, TRUE);
		}
	}
	/// Set Fonts
	if (fontType == NormalFont)
	{
		if (widthScale * heightScale > 2)
		{
			SetFont(fonts["Normal Font Max"]);
		}
		else if (widthScale * heightScale > 0.8)
		{
			SetFont(fonts["Normal Font Large"]);
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
	else if (fontType == CodeFont)
	{
		if (widthScale * heightScale > 2)
		{
			SetFont(fonts["Code Font Max"]);
		}
		else if (widthScale * heightScale > 0.8)
		{
			SetFont(fonts["Code Font Large"]);
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
	else if (fontType == HeadingFont)
	{
		if (widthScale * heightScale > 2)
		{
			SetFont(fonts["Heading Font Max"]);
		}
		else if (widthScale * heightScale > 0.8)
		{
			SetFont(fonts["Heading Font Large"]);
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
	else if (fontType == LargeFont)
	{
		if (widthScale * heightScale > 2)
		{
			SetFont(fonts["Larger Font Max"]);
		}
		else if (widthScale * heightScale > 0.8)
		{
			SetFont(fonts["Larger Font Large"]);
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
	else if (fontType == SmallFont)
	{
		if (widthScale * heightScale > 2)
		{
			SetFont(fonts["Smaller Font Max"]);
		}
		else if (widthScale * heightScale > 0.8)
		{
			SetFont(fonts["Smaller Font Large"]);
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


/// template function for the class control system
template <class Parent> bool Control<Parent>::setToolTip(std::string text, std::vector<CToolTipCtrl*>& toolTips, 
														 MasterWindow* master)
{
	if (!toolTipIsSet)
	{
		toolTipID = toolTips.size();
		toolTips.push_back(new CToolTipCtrl);
		toolTips.back()->Create(master, TTS_ALWAYSTIP | TTS_BALLOON);
		toolTips.back()->SetMaxTipWidth(1000);
		toolTips.back()->SetTipBkColor(0x000000);
		toolTips.back()->SetTipTextColor(0xe0e0d0);
		toolTips.back()->SetFont(CFont::FromHandle(toolTipFont));
		toolTips.back()->SetDelayTime(TTDT_AUTOPOP, 30000);
		toolTips.back()->AddTool(this, text.c_str());
		toolTipIsSet = true;
	}
	else
	{
		toolTips[toolTipID]->DelTool(this);
		toolTips[toolTipID]->AddTool(this, text.c_str());
	}
	return true;
}



