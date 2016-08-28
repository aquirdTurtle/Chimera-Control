#include "stdafx.h"
#include "reorganizeControl.h"
#include "fonts.h"

int reorganizeControl(HwndControl controlToReposition, std::string mode, RECT parentRectangle)
{
	/// Set Positions
	double widthScale = (parentRectangle.right - parentRectangle.left) / 1936.0;
	double heightScale = (parentRectangle.bottom - parentRectangle.top) / 1056.0;
	// extra heigh added to certain controls based on random things like the trigger mode.
	double extraHeight = 0;
	if (eCurrentTriggerMode == "External" && controlToReposition.triggerModeSensitive == 1 && (mode == "Kinetic Series Mode" || mode == "Accumulate Mode"))
	{
		extraHeight += -30;
	}
	if (mode == "Kinetic Series Mode")
	{
		if (controlToReposition.kineticSeriesModePos.left == -1 || (controlToReposition.triggerModeSensitive == -1 && eCurrentTriggerMode == "External"))
		{
			ShowWindow(controlToReposition.hwnd, SW_HIDE);
		}
		else
		{
			ShowWindow(controlToReposition.hwnd, SW_SHOW);
			MoveWindow(controlToReposition.hwnd, widthScale * controlToReposition.kineticSeriesModePos.left, heightScale * (controlToReposition.kineticSeriesModePos.top + extraHeight),
				widthScale * (controlToReposition.kineticSeriesModePos.right - controlToReposition.kineticSeriesModePos.left),
				heightScale * (controlToReposition.kineticSeriesModePos.bottom - controlToReposition.kineticSeriesModePos.top), TRUE);
		}
	}
	else if (mode == "Continuous Single Scans Mode")
	{
		if (controlToReposition.continuousSingleScansModePos.left == -1 || (controlToReposition.triggerModeSensitive == -1 && eCurrentTriggerMode == "External"))
		{
			ShowWindow(controlToReposition.hwnd, SW_HIDE);
		}
		else
		{
			ShowWindow(controlToReposition.hwnd, SW_SHOW);
			MoveWindow(controlToReposition.hwnd, widthScale * controlToReposition.continuousSingleScansModePos.left, heightScale * (controlToReposition.continuousSingleScansModePos.top + extraHeight),
					   widthScale * (controlToReposition.continuousSingleScansModePos.right - controlToReposition.continuousSingleScansModePos.left),
					   heightScale * (controlToReposition.continuousSingleScansModePos.bottom - controlToReposition.continuousSingleScansModePos.top), TRUE);
		}
	}
	else if (mode == "Accumulate Mode")
	{
		if (controlToReposition.accumulateModePos.left == -1 || (controlToReposition.triggerModeSensitive == -1 && eCurrentTriggerMode == "External"))
		{
			ShowWindow(controlToReposition.hwnd, SW_HIDE);
		}
		else
		{
			ShowWindow(controlToReposition.hwnd, SW_SHOW);
			MoveWindow(controlToReposition.hwnd, widthScale * controlToReposition.accumulateModePos.left, heightScale * (controlToReposition.accumulateModePos.top + extraHeight),
				widthScale * (controlToReposition.accumulateModePos.right - controlToReposition.accumulateModePos.left),
				heightScale * (controlToReposition.accumulateModePos.bottom - controlToReposition.accumulateModePos.top), TRUE);
		}
	}
	/// Set Fonts
	if (controlToReposition.fontType == "Normal")
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
	else if (controlToReposition.fontType == "Code")
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
	else if (controlToReposition.fontType == "Heading")
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
	else if (controlToReposition.fontType == "Large")
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
	else if (controlToReposition.fontType == "Small")
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

	return 0;
}
