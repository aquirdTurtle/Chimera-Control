#include "stdafx.h"
#include "reorganizeControl.h"
#include "fonts.h"

int reorganizeControl(Control controlToReposition, std::string mode, RECT parentRectangle)
{
	double widthScale = (parentRectangle.right - parentRectangle.left) / 1936.0;
	double heightScale = (parentRectangle.bottom - parentRectangle.top) / 1056.0;
	if (mode == "Kinetic Series Mode")
	{
		if (controlToReposition.kineticSeriesModePos.left == -1)
		{
			ShowWindow(controlToReposition.hwnd, SW_HIDE);
		}
		else
		{
			ShowWindow(controlToReposition.hwnd, SW_SHOW);
			MoveWindow(controlToReposition.hwnd, widthScale * controlToReposition.kineticSeriesModePos.left, heightScale * controlToReposition.kineticSeriesModePos.top,
				widthScale * (controlToReposition.kineticSeriesModePos.right - controlToReposition.kineticSeriesModePos.left),
				heightScale * (controlToReposition.kineticSeriesModePos.bottom - controlToReposition.kineticSeriesModePos.top), TRUE);
		}
	}
	else if (mode == "Continuous Single Scans Mode")
	{
		if (controlToReposition.continuousSingleScansModePos.left == -1)
		{
			ShowWindow(controlToReposition.hwnd, SW_HIDE);
		}
		else
		{
			ShowWindow(controlToReposition.hwnd, SW_SHOW);
			MoveWindow(controlToReposition.hwnd, widthScale * controlToReposition.continuousSingleScansModePos.left, heightScale * controlToReposition.continuousSingleScansModePos.top,
				widthScale * (controlToReposition.continuousSingleScansModePos.right - controlToReposition.continuousSingleScansModePos.left),
				heightScale * (controlToReposition.continuousSingleScansModePos.bottom - controlToReposition.continuousSingleScansModePos.top), TRUE);
		}
	}
	else if (mode == "Accumulate Mode")
	{
		if (controlToReposition.accumulateModePos.left == -1)
		{
			ShowWindow(controlToReposition.hwnd, SW_HIDE);
		}
		else
		{
			ShowWindow(controlToReposition.hwnd, SW_SHOW);
			MoveWindow(controlToReposition.hwnd, widthScale * controlToReposition.accumulateModePos.left, heightScale * controlToReposition.accumulateModePos.top,
				widthScale * (controlToReposition.accumulateModePos.right - controlToReposition.accumulateModePos.left),
				heightScale * (controlToReposition.accumulateModePos.bottom - controlToReposition.accumulateModePos.top), TRUE);
		}
	}

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
	

	return 0;
}
