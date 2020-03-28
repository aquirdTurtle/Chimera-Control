// created by Mark O. Brown

#pragma once

#include "Control.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include <Windows.h>
#include <unordered_map>

class Repetitions
{
	public:
		void initialize(POINT& pos, cToolTips& toolTips, CWnd* mainWin, int& id );
		void setRepetitions(UINT number);
		unsigned int getRepetitionNumber();
		static UINT getSettingsFromConfig (ConfigStream& openFile, Version ver );
		HBRUSH handleColorMessage(CWnd* window, brushMap brushes, rgbMap rGBs, CDC* cDC);
		void rearrange(UINT width, UINT height, fontMap fonts);
		void updateNumber(long repNumber);
		void handleSaveConfig(ConfigStream& saveFile);
	private:
		UINT repetitionNumber;
		Control<CEdit> repetitionEdit;
		Control<CEdit> repetitionDisp;
		Control<CStatic> repetitionText;
};
