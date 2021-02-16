// created by Mark O. Brown
#pragma once

#include "Control.h"
#include "ExperimentThread/DeviceList.h"
#include "GeneralObjects/commonTypes.h"
#include <PrimaryWindows/IChimeraQtWindow.h>
#include <qlabel.h>
/*
 * I put one of these controls on every window. It shows the colors for every system running.
 */
struct boxInfo{
	QLabel* ctrl;
	std::string color;
	std::string delim;
};

class ColorBox{
	public:
		void initialize( QPoint& pos, IChimeraQtWindow* parent, int length, DeviceList devices, unsigned numRows=1);
		void changeColor (std::string delim, std::string color);
		bool initialized = false;
	private:
		std::vector<boxInfo> boxes;
};


