
#pragma once

#include "Control.h"
#include <Windows.h>
#include <unordered_map>

class Repetitions
{
	public:
		void initialize(POINT& pos, std::vector<CToolTipCtrl*>& toolTips, MainWindow* mainWin, int& id);
		void setRepetitions(unsigned int number);
		unsigned int getRepetitionNumber();
		HBRUSH handleColorMessage(CWnd* window, brushMap brushes, rgbMap rGBs, CDC* cDC);
		//void handleButtonPush();
		void rearrange(UINT width, UINT height, fontMap fonts);
		void updateNumber(long repNumber);
	private:
		unsigned int repetitionNumber;
		Control<CEdit> repetitionEdit;
		Control<CEdit> repetitionDisp;
		Control<CStatic> repetitionText;
		//Control<CButton> setRepetitionButton;
};
