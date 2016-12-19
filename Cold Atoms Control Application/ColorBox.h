#pragma once
#include "Control.h"
class ColorBox
{
	public:
		void initialize(POINT pos, int& id, CWnd* parent);
		void changeColor(std::string color);
		HBRUSH handleColoring(int id, CDC* pDC);
	private:
		Control<CStatic> box;
		std::string currentColor;
};
