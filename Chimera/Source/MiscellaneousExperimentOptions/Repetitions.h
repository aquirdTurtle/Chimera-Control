// created by Mark O. Brown

#pragma once

#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include <unordered_map>
#include <QLabel>
#include <PrimaryWindows/IChimeraWindow.h>
#include <CustomQtControls/AutoNotifyCtrls.h>

class Repetitions
{
	public:
		void initialize(POINT& pos, IChimeraWindowWidget* mainWin );
		void setRepetitions(UINT number);
		unsigned int getRepetitionNumber();
		static UINT getSettingsFromConfig (ConfigStream& openFile );
		void updateNumber(long repNumber);
		void handleSaveConfig(ConfigStream& saveFile);
	private:
		UINT repetitionNumber;
		CQLineEdit* repetitionEdit;
		QLabel* repetitionDisp;
		QLabel* repetitionText;
};
