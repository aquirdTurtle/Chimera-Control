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
		RECT ksmPos;
		// Accumulate Mode Position
		RECT amPos;
		// Continuous Single Scan Mode Position
		RECT cssmPos;
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
	if (trigMode == "External" && this->triggerModeSensitive && (cameraMode == "Kinetic Series Mode" || cameraMode == "Accumulate Mode"))
	{
		extraHeight += -25;
	}
	// handle simple case.
	if (this->sPos.bottom != 0 || this->sPos.top != 0)
	{
		this->ShowWindow(SW_SHOW);
		RECT position = { widthScale * this->sPos.left, heightScale * (this->sPos.top + extraHeight),
			widthScale * this->sPos.right,
			heightScale * this->sPos.bottom };
		this->MoveWindow(&position, TRUE);
	}
	else if (cameraMode == "Kinetic Series Mode")
	{
		if (this->ksmPos.left == -1 || (this->triggerModeSensitive == -1 && trigMode == "External"))
		{
			this->ShowWindow(SW_HIDE);
		}
		else
		{
			this->ShowWindow(SW_SHOW);
			RECT position = { widthScale * this->ksmPos.left, heightScale * (this->ksmPos.top + extraHeight),
				widthScale * this->ksmPos.right,
				heightScale * (this->ksmPos.bottom + extraHeight) };
			this->MoveWindow(&position, TRUE);
		}
	}	 
	else if (cameraMode == "Continuous Single Scans Mode")
	{
		if (this->cssmPos.left == -1 || (this->triggerModeSensitive == -1 && trigMode == "External"))
		{
			this->ShowWindow(SW_HIDE);
		}
		else
		{
			this->ShowWindow(SW_SHOW);
			RECT position = { widthScale * this->cssmPos.left, heightScale * (this->cssmPos.top + extraHeight),
				widthScale * this->cssmPos.right,
				heightScale * (this->cssmPos.bottom + extraHeight) };
			this->MoveWindow(&position, TRUE);
		}
	}
	else if (cameraMode == "Accumulate Mode")
	{
		if (this->amPos.left == -1 || (this->triggerModeSensitive == -1 && trigMode == "External"))
		{
			this->ShowWindow(SW_HIDE);
		}
		else
		{
			this->ShowWindow(SW_SHOW);
			RECT position = { widthScale * this->amPos.left, heightScale * (this->amPos.top + extraHeight),
				widthScale * this->amPos.right,
				heightScale * (this->amPos.bottom + extraHeight) };
			this->MoveWindow(&position, TRUE);
		}
	}
	/// Set Fonts
	if (this->fontType == "Normal")
	{
		if (widthScale * heightScale > 0.8)
		{
			this->SetFont(fonts["Normal Font Max"]);
		}
		else if (widthScale * heightScale > 0.6)
		{
			this->SetFont(fonts["Normal Font Med"]);
		}
		else
		{
			this->SetFont(fonts["Normal Font Small"]);
		}
	}
	else if (this->fontType == "Code")
	{
		if (widthScale * heightScale > 0.8)
		{
			this->SetFont(fonts["Code Font Max"]);
		}
		else if (widthScale * heightScale > 0.6)
		{
			this->SetFont(fonts["Code Font Med"]);
		}
		else
		{
			this->SetFont(fonts["Code Font Small"]);
		}
	}
	else if (this->fontType == "Heading")
	{
		if (widthScale * heightScale > 0.8)
		{
			this->SetFont(fonts["Heading Font Max"]);
		}
		else if (widthScale * heightScale > 0.6)
		{
			this->SetFont(fonts["Heading Font Med"]);
		}
		else
		{
			this->SetFont(fonts["Heading Font Small"]);
		}
	}
	else if (this->fontType == "Large")
	{
		if (widthScale * heightScale > 0.8)
		{
			this->SetFont(fonts["Larger Font Max"]);
		}
		else if (widthScale * heightScale > 0.6)
		{
			this->SetFont(fonts["Larger Font Med"]);
		}
		else
		{
			this->SetFont(fonts["Larger Font Small"]);
		}
	}
	else if (this->fontType == "Small")
	{
		if (widthScale * heightScale > 0.8)
		{
			this->SetFont(fonts["Smaller Font Max"]);
		}
		else if (widthScale * heightScale > 0.6)
		{
			this->SetFont(fonts["Smaller Font Med"]);
		}
		else
		{
			this->SetFont(fonts["Smaller Font Small"]);
		}
	}
}

/// template function for the class control system
template <class ControlType> bool Control<ControlType>::setToolTip(std::string text, std::vector<CToolTipCtrl*>& tooltips, CWnd* parentWindow, CFont* font)
{
	if (!this->toolTipIsSet)
	{
		this->toolTipID = tooltips.size();
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
		this->toolTipIsSet = true;
	}
	else
	{
		tooltips[toolTipID]->DelTool(this);
		tooltips[toolTipID]->AddTool(this, text.c_str());
	}
	return true;
}