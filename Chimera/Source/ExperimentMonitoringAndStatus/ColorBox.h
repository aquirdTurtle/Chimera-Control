// created by Mark O. Brown
#pragma once

#include "Control.h"
#include "ExperimentThread/DeviceList.h"
#include "GeneralObjects/commonTypes.h"
#include <PrimaryWindows/IChimeraWindow.h>
#include <qlabel.h>
/*
 * I put one of these controls on every window. It shows the colors for every system running.
 */
struct boxInfo
{
	QLabel* ctrl;
	//std::unique_ptr<Control<CEdit>> ctrl;
	std::string color;
	std::string delim;
};

class ColorBox
{
	public:
		void initialize( POINT& pos, IChimeraWindowWidget* parent, int length, DeviceList devices );
		void changeColor (std::string delim, std::string color);
		CBrush* handleColoring( int id, CDC* pDC );
		bool initialized = false;
	private:
		std::vector<boxInfo> boxes;
};


