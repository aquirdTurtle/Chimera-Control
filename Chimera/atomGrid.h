#pragma once

#include "afxwin.h"
#include "coordinate.h"

struct atomGrid
{
	coordinate topLeftCorner;
	ULONG pixelSpacing;
	// in atoms
	ULONG width;
	ULONG height;
};

