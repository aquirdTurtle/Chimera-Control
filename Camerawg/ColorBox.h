#pragma once
#include "Control.h"

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
		void initialize(POINT& pos, int& id, CWnd* parent, int length, std::unordered_map<std::string, CFont*> fonts, 
			std::vector<CToolTipCtrl*>& tooltips);
		void changeColor(colorBoxes<char> color);
		CBrush* handleColoring( int id, CDC* pDC, std::unordered_map<std::string, CBrush*> brushes,
								std::unordered_map<std::string, COLORREF> rgbs );
		void rearrange(std::string cameraMode, std::string triggerMode, int width, int height,
			std::unordered_map<std::string, CFont*> fonts);
	private:
		colorBoxes<Control<CStatic>> boxes;
		colorBoxes<char> colors;
};


