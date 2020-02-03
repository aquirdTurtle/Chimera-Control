// created by Mark O. Brown
#pragma once

#include "Control.h"
#include "GeneralObjects/commonTypes.h"


class StatusIndicator
{
	public:
		void initialize(POINT &loc, CWnd* parent, int& id, cToolTips& tooltips);
		void setText(std::string text);
		void setColor(std::string color);
		CBrush* handleColor(CWnd* window, CDC* pDC );
		void rearrange(int width, int height, fontMap fonts);
	private:
		Control<CStatic> status;
		std::string currentColor;
};
