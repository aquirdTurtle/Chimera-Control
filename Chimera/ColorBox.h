#pragma once

#include "Control.h"
#include "commonTypes.h"


template <typename type> struct systemInfo
{
	type niawg;
	type camera;
	type intensity;
	type master;
};

/*
 * I put one of these controls on every window. It shows the colors for every system running.
 */
class ColorBox
{
	public:
		void initialize(POINT& pos, int& id, CWnd* parent, int length, fontMap fonts, 
			std::vector<CToolTipCtrl*>& tooltips);
		void changeColor(systemInfo<char> color);
		CBrush* handleColoring( int id, CDC* pDC, brushMap brushes, rgbMap rgbs );
		void rearrange(std::string cameraMode, std::string triggerMode, int width, int height, fontMap fonts);
	private:
		systemInfo<Control<CStatic>> boxes;
		systemInfo<char> colors;
};


