#pragma once

#include "Control.h"
#include "commonTypes.h"

template <typename type> struct colorBoxes
{
	type niawg;
	type camera;
	type intensity;
};

/*
 * I put one of these controls on every window. It shows the colors for every system running.
 */
class ColorBox
{
	public:
		void initialize(POINT& pos, int& id, CWnd* parent, int length, fontMap fonts, 
			std::vector<CToolTipCtrl*>& tooltips);
		void changeColor(colorBoxes<char> color);
		CBrush* handleColoring( int id, CDC* pDC, brushMap brushes, rgbMap rgbs );
		void rearrange(std::string cameraMode, std::string triggerMode, int width, int height, fontMap fonts);
	private:
		colorBoxes<Control<CStatic>> boxes;
		colorBoxes<char> colors;
};


