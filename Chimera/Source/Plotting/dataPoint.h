// created by Mark O. Brown
#pragma once

#include <vector>
#include <qmetatype.h>

struct dataPoint{
	double x;
	double y=-1;
	// yerr, could add xerr in future.
	double err;
};
Q_DECLARE_METATYPE (dataPoint)
Q_DECLARE_METATYPE (std::vector<dataPoint>)
Q_DECLARE_METATYPE (std::vector<std::vector<dataPoint>>)
