#include "stdafx.h"
#include "reorganizeControl.h"
#include "fonts.h"

int reorganizeControl(Control<CWnd>& controlToReposition, std::string mode, RECT parentRectangle)
{
	std::string trigMode;
	/// Set Positions
	double widthScale = (parentRectangle.right - parentRectangle.left) / 1936.0;
	double heightScale = (parentRectangle.bottom - parentRectangle.top) / 1056.0;
	// extra heigh added to certain controls based on random things like the trigger mode.
	double extraHeight = 0;
	if (trigMode == "External" && controlToReposition.triggerModeSensitive && (mode == "Kinetic Series Mode" || mode == "Accumulate Mode"))
	{
		extraHeight += -30;
	}
	if (mode == "Kinetic Series Mode")
	{
		if (controlToReposition.seriesPos.left == -1 || (controlToReposition.triggerModeSensitive == -1 && trigMode == "External Trigger"))
		{
			controlToReposition.ShowWindow(SW_HIDE);
		}
		else
		{
			controlToReposition.ShowWindow(SW_SHOW);
			RECT position = { long(widthScale * controlToReposition.seriesPos.left), 
				long(heightScale * (controlToReposition.seriesPos.top + extraHeight)),
				long(widthScale * (controlToReposition.seriesPos.right - controlToReposition.seriesPos.left)),
				long(heightScale * (controlToReposition.seriesPos.bottom - controlToReposition.seriesPos.top)) };
			controlToReposition.MoveWindow(&position, TRUE);
		}
	}
	else if (mode == "Continuous Single Scans Mode")
	{
		if (controlToReposition.videoPos.left == -1 || (controlToReposition.triggerModeSensitive == -1 && trigMode == "External Trigger"))
		{
			controlToReposition.ShowWindow(SW_HIDE);
		}
		else
		{
			controlToReposition.ShowWindow(SW_SHOW);
			RECT position = { long(widthScale * controlToReposition.videoPos.left), 
				long(heightScale * (controlToReposition.videoPos.top + extraHeight)),
				long(widthScale * (controlToReposition.videoPos.right - controlToReposition.videoPos.left)),
				long(heightScale * (controlToReposition.videoPos.bottom - controlToReposition.videoPos.top)) };
			controlToReposition.MoveWindow(&position, TRUE);
		}
	}
	else if (mode == "Accumulate Mode")
	{
		if (controlToReposition.amPos.left == -1 || (controlToReposition.triggerModeSensitive == -1 && trigMode == "External Trigger"))
		{
			controlToReposition.ShowWindow(SW_HIDE);
		}
		else
		{
			controlToReposition.ShowWindow(SW_SHOW);
			RECT position = { long(widthScale * controlToReposition.amPos.left), 
				long(heightScale * (controlToReposition.amPos.top + extraHeight)),
				long(widthScale * (controlToReposition.amPos.right - controlToReposition.amPos.left)),
				long(heightScale * (controlToReposition.amPos.bottom - controlToReposition.amPos.top)) };
			controlToReposition.MoveWindow(&position, TRUE);
		}
	}
	/// Set Fonts
	/// TODO
	/*
	if (controlToReposition.fontType == NormalFont)
	{
		if (widthScale * heightScale > 0.8)
		{
			
			SendMessage(controlToReposition.hwnd, WM_SETFONT, WPARAM(sNormalFontMax), TRUE);
		}
		else if (widthScale * heightScale > 0.6)
		{
			SendMessage(controlToReposition.hwnd, WM_SETFONT, WPARAM(sNormalFontMed), TRUE);
		}
		else
		{
			SendMessage(controlToReposition.hwnd, WM_SETFONT, WPARAM(sNormalFontSmall), TRUE);
		}
	}
	else if (controlToReposition.fontType == CodeFont)
	{
		if (widthScale * heightScale > 0.8)
		{
			SendMessage(controlToReposition.hwnd, WM_SETFONT, WPARAM(sCodeFontMax), TRUE);
		}
		else if (widthScale * heightScale > 0.6)
		{
			SendMessage(controlToReposition.hwnd, WM_SETFONT, WPARAM(sCodeFontMed), TRUE);
		}
		else
		{
			SendMessage(controlToReposition.hwnd, WM_SETFONT, WPARAM(sCodeFontSmall), TRUE);
		}
	}
	else if (controlToReposition.fontType == HeadingFont)
	{
		if (widthScale * heightScale > 0.8)
		{
			SendMessage(controlToReposition.hwnd, WM_SETFONT, WPARAM(sHeadingFontMax), TRUE);
		}
		else if (widthScale * heightScale > 0.6)
		{
			SendMessage(controlToReposition.hwnd, WM_SETFONT, WPARAM(sHeadingFontMed), TRUE);
		}
		else
		{
			SendMessage(controlToReposition.hwnd, WM_SETFONT, WPARAM(sHeadingFontSmall), TRUE);
		}
	}
	else if (controlToReposition.fontType == LargeFont)
	{
		if (widthScale * heightScale > 0.8)
		{
			SendMessage(controlToReposition.hwnd, WM_SETFONT, WPARAM(sLargerFontMax), TRUE);
		}
		else if (widthScale * heightScale > 0.6)
		{
			SendMessage(controlToReposition.hwnd, WM_SETFONT, WPARAM(sLargerFontMed), TRUE);
		}
		else
		{
			SendMessage(controlToReposition.hwnd, WM_SETFONT, WPARAM(sLargerFontSmall), TRUE);
		}
	}
	else if (controlToReposition.fontType == SmallFont)
	{
		if (widthScale * heightScale > 0.8)
		{
			SendMessage(controlToReposition.hwnd, WM_SETFONT, WPARAM(sSmallerFontMax), TRUE);
		}
		else if (widthScale * heightScale > 0.6)
		{
			SendMessage(controlToReposition.hwnd, WM_SETFONT, WPARAM(sSmallerFontMed), TRUE);
		}
		else
		{
			SendMessage(controlToReposition.hwnd, WM_SETFONT, WPARAM(sSmallerFontSmall), TRUE);
		}
	}
	*/

	return 0;
}
