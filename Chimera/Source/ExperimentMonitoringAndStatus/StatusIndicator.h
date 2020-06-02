// created by Mark O. Brown
#pragma once

#include "Control.h"
#include "GeneralObjects/commonTypes.h"
#include <PrimaryWindows/IChimeraWindow.h>
#include <qlabel.h>

class StatusIndicator
{
	public:
		void initialize(POINT &loc, IChimeraWindowWidget* parent );
		void setText(std::string text);
		void setColor(std::string color);
		CBrush* handleColor(CWnd* window, CDC* pDC );
		void rearrange(int width, int height, fontMap fonts);
	private:
		QLabel* status;
		std::string currentColor;
};
