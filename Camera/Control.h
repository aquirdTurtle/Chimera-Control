#pragma once

#include "Windows.h"

struct Control
{
	HWND hwnd;
	RECT kineticSeriesModePos;
	RECT continuousSingleScansModePos;
	RECT accumulateModePos;
	std::string fontType;
};