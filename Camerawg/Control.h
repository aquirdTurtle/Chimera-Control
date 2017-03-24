#pragma once

#include "Windows.h"
#include <unordered_map>
#include <string>
#include <type_traits>

/*
 * This is a complex class definition. The first line here declares that this is a template class with a class template
 * argument named ControlType.  I.e. my class is based on a class called ControlType.
 * 
 * The second line forces ControlType to be inherited from some CWnd class, i.e. ControlType should always be
 * some typical MFC control, the purpose of which is clear in the next line.
 *
 * the last line names the template class to be "Control" and says that it inherits it's template argument. This allows
 * me to quickly create class objects based on existing MFC class objects which all have a set of useful additional 
 * parameters that I make use of in my GUI. It ensures that the user correctly inherits from an MFC type both because
 * this is simply what this is designed for and also so that I can use CWnd functions inside the rearrange function 
 * here.
 */
template <class ControlType> class Control : public ControlType
{
	public:
		Control();
		// Standard Position: for objects that don't move around and have permanent positions in their windows.
		RECT sPos;
		// Kinetic Series Mode Positon: position for camera-mode-sensitive objects in kinetic series mode.
		RECT seriesPos;
		// Accumulate Mode Position
		RECT amPos;
		// Continuous Single Scan Mode Position
		RECT videoPos;
		std::string fontType;
		int triggerModeSensitive;
		//
		int ID;
		int colorState = 0;
		void rearrange(std::string cameraMode, std::string trigMode, int width, int height, 
			std::unordered_map<std::string, CFont*> fonts);
		//bool ClassControl<ControlType>::setToolTip(std::string text, std::vector<CToolTipCtrl*>& tooltips, MasterWindow* master);
		bool setToolTip(std::string text, std::vector<CToolTipCtrl*>& tooltips, CWnd* master, CFont* font);
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
	assert((CWnd const*)&obj);
}

template <class ControlType> void Control<ControlType>::rearrange(std::string cameraMode, std::string trigMode, int width, int height, 
																   std::unordered_map<std::string, CFont*> fonts)
{
	// make sure the control has been initialized
	if (!m_hWnd)
	{
		return;
	}
	/*
	HMONITOR monitor = MonitorFromWindow(this->GetParent()->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
	MONITORINFO info;
	info.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor, &info);
	double workspaceWidth = info.rcWork.right - info.rcWork.left;
	double workspaceHeight = info.rcWork.bottom - info.rcWork.top;
	*/
	/// Set Positions
	//double widthScale = width / workspaceWidth;
	//double heightScale = height / workspaceHeight;
	double widthScale = width / 1920.0;
	double heightScale = height / 997.0;
	// extra heigh added to certain controls based on random things like the trigger mode.
	double extraHeight = 0;
	if (trigMode == "External" && triggerModeSensitive && (cameraMode == "Kinetic Series Mode" || cameraMode == "Accumulate Mode"))
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
	else if (cameraMode == "Continuous Single Scans Mode")
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
	else if (cameraMode == "Accumulate Mode")
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

/// template function for the class control system
template <class ControlType> bool Control<ControlType>::setToolTip(std::string text, std::vector<CToolTipCtrl*>& tooltips, CWnd* parentWindow, CFont* font)
{
	if (!toolTipIsSet)
	{
		toolTipID = tooltips.size();
		tooltips.push_back(new CToolTipCtrl);
		tooltips.back()->Create(parentWindow, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_BALLOON);
		tooltips.back()->SetMaxTipWidth(500);
		tooltips.back()->SetTipBkColor(0x000000);
		tooltips.back()->SetTipTextColor(0xe0e0d0);
		tooltips.back()->SetDlgCtrlID(ID);
		tooltips.back()->SetFont(font);
		tooltips.back()->SetDelayTime(TTDT_AUTOPOP, 30000);
		tooltips.back()->AddTool(this, text.c_str());
		tooltips.back()->Activate(TRUE);
		toolTipIsSet = true;
	}
	else
	{
		tooltips[toolTipID]->DelTool(this);
		tooltips[toolTipID]->AddTool(this, text.c_str());
	}
	return true;
}