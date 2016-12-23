#pragma once
#include "Control.h"
class ColorBox
{
	public:
		void initialize(POINT& pos, int& id, CWnd* parent, int length);
		void changeColor(std::string color);
		CBrush* handleColoring(int id, CDC* pDC, std::unordered_map<std::string, CBrush*> brushes);
		void rearrange(std::string cameraMode, std::string triggerMode, int width, int height);
	private:
		Control<CStatic> box;
		std::string currentColor;
};
