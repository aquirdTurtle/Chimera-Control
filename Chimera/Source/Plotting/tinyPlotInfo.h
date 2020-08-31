// created by Mark O. Brown
#pragma once

#include <string>

// "Tiny" because it only contains a little info right now.
struct tinyPlotInfo{
	bool isActive;
	std::string name;
	bool isHist=false;
	unsigned whichGrid=0;
	unsigned numPics=1;
};

