// created by Mark O. Brown
#pragma once

#include "Control.h"
#include "ExperimentThread/DeviceList.h"
#include "GeneralObjects/commonTypes.h"

/*
 * I put one of these controls on every window. It shows the colors for every system running.
 */
struct boxInfo
{
	std::unique_ptr<Control<CEdit>> ctrl;
	char color;
	std::string delim;
};

class ColorBox
{
	public:
		void initialize( POINT& pos, int& id, CWnd* parent, int length, cToolTips& tooltips, DeviceList devices );
		void changeColor (std::string delim, char color);
		CBrush* handleColoring( int id, CDC* pDC );
		void rearrange( int width, int height, fontMap fonts );
		bool initialized = false;
	private:
		std::vector<boxInfo> boxes;
};


