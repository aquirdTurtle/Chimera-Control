// created by Mark O. Brown
#pragma once

#include "afxwin.h"
#include "GeneralObjects/coordinate.h"

struct atomGrid
{
	coordinate topLeftCorner;
	ULONG pixelSpacing;
	// in atoms
	ULONG width;
	ULONG height;
	ULONG numAtoms ( )
	{
		return width * height;
	}
};

