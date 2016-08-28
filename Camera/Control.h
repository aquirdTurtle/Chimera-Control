#pragma once

#include "Windows.h"

struct HwndControl
{
	// main window handle used by win32
	HWND hwnd;
	// positions for different camera modes
	RECT kineticSeriesModePos;
	RECT continuousSingleScansModePos;
	RECT accumulateModePos;
	// 0 means no change, 1 means adjust position, -1 means don't show.
	int triggerModeSensitive = 0;
	// a string that indicates the general font type. Exact size determined by this and size of system window.
	std::string fontType;
	// TODO: add colors themselves as a control.
	int colorState = 0;
	int ID;
};