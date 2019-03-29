// created by Mark O. Brown
#pragma once
#include <vector>
#include "afxwin.h"

// basically just a little nicer than a std::pair
struct AtomImage
{
	UINT repNum;
	std::vector<bool> image;
};

struct NormalImage
{
	UINT repNum;
	std::vector<long> image;
};

// imQueue[gridNum][queuePositions][pixelNum(flattened)]
typedef std::vector<AtomImage> atomQueue;
typedef std::vector<NormalImage> imageQueue;
typedef std::vector<imageQueue> multiGridImageQueue;
typedef std::vector<atomQueue> multiGridAtomQueue;
