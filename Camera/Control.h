#pragma once

#include "Windows.h"

struct Control
{
	// main window handle used by win32
	HWND hwnd;
	// positions for different camera modes
	RECT kineticSeriesModePos;
	RECT continuousSingleScansModePos;
	RECT accumulateModePos;
	// this is an extra displacement that gets added when internal trigger mode is on.
	bool triggerModeSensitive;
	// a string that indicates the general font type. Exact size determined by this and size of system window.
	std::string fontType;
};