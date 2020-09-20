// created by Mark O. Brown
#pragma once
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "GeneralObjects/commonTypes.h"
#include "debugInfo.h"
#include "PrimaryWindows/IChimeraQtWindow.h"
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>

class MainWindow;

class DebugOptionsControl
{
	public:
		void handleSaveConfig(ConfigStream& saveFile);
		void handleOpenConfig(ConfigStream& openFile );
		void initialize( POINT& loc, IChimeraQtWindow* parent );
		void handleEvent(unsigned id, MainWindow* comm);
		debugInfo getOptions();
		void setOptions(debugInfo options);

	private:

		QLabel* header;
		QCheckBox* readProgress;
		QCheckBox* writeProgress;
		QCheckBox* correctionTimes;
		QCheckBox* niawgScript;
		QCheckBox* outputAgilentScript;
		QCheckBox* niawgMachineScript;
		QCheckBox* excessInfo;
		QCheckBox* showTtlsButton;
		QCheckBox* showDacsButton;
		QCheckBox* outputNiawgWavesToText;
		QLabel* pauseText;
		QLineEdit* pauseEdit;

		debugInfo currentOptions;
};