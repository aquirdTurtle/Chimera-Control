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

class DebugOptionsControl {
	public:
		void handleSaveConfig(ConfigStream& saveFile);
		void handleOpenConfig(ConfigStream& openFile );
		void initialize( QPoint& loc, IChimeraQtWindow* parent );
		void handleEvent(unsigned id, MainWindow* comm);
		debugInfo getOptions();
		void setOptions(debugInfo options);

	private:

		QLabel* header;
		QLabel* pauseText;
		QLineEdit* pauseEdit;

		debugInfo currentOptions;
};