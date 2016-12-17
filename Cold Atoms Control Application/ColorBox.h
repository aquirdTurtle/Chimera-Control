#pragma once
#include "Control.h"
class ColorBox
{
	public:
		void initialize(POINT pos, int& id, CWnd* parent);
		void redraw();
		bool isColoringThisBox(int id);
	private:
		Control<CStatic> box;
};
