// created by Mark O. Brown
#pragma once

#include "GeneralObjects/coordinate.h"

struct atomGrid{
	coordinate topLeftCorner;
	unsigned long pixelSpacing;
	// in atoms
	unsigned long width;
	unsigned long height;
	unsigned long numAtoms ( ){
		return width * height;
	}
};

