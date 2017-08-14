
#pragma once

#include "Control.h"
#include <Windows.h>
#include <unordered_map>

class Repetitions
{
	public:
		void initialize(POINT& pos, cToolTips& toolTips, MainWindow* mainWin, int& id);
		void setRepetitions(unsigned int number);
		unsigned int getRepetitionNumber();
		HBRUSH handleColorMessage(CWnd* window, brushMap brushes, rgbMap rGBs, CDC* cDC);
		void rearrange(UINT width, UINT height, fontMap fonts);
		void updateNumber(long repNumber);
		void handleSaveConfig(std::ofstream& saveFile);
		void handleOpenConfig(std::ifstream& openFile, double version);

	private:
		unsigned int repetitionNumber;
		Control<CEdit> repetitionEdit;
		Control<CEdit> repetitionDisp;
		Control<CStatic> repetitionText;
};
