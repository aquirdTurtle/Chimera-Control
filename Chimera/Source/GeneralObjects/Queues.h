// created by Mark O. Brown
#pragma once

#include <vector>
#include <GeneralObjects/Matrix.h>
#include <qobject.h>

// basically just a little nicer than a std::pair
struct AtomImage{
	unsigned picNum;
	std::vector<bool> image;
};

struct NormalImage{
	unsigned picNum;
	Matrix<long> image;
};

struct PixList{
	unsigned picNum;
	std::vector<long> image;
};

// imQueue[gridNum][queuePositions][pixelNum(flattened)]
typedef std::vector<AtomImage> atomQueue;
typedef std::vector<NormalImage> imageQueue;
typedef std::vector<PixList> PixListQueue;
//typedef std::vector<imageQueue> multiGridImageQueue;
typedef std::vector<PixListQueue> multiGridImageQueue;
typedef std::vector<atomQueue> multiGridAtomQueue;

Q_DECLARE_METATYPE (NormalImage)
Q_DECLARE_METATYPE (atomQueue)
Q_DECLARE_METATYPE (PixListQueue)