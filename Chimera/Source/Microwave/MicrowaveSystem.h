// created by Mark O. Brown
#pragma once

#include "Microwave/MicrowaveCore.h"
#include "Microwave/MicrowaveSettings.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include <vector>
#include <string>
#include <unordered_map>
#include "PrimaryWindows/IChimeraQtWindow.h"
#include <GeneralObjects/IChimeraSystem.h>
#include <QCheckBox.h>
#include <QLabel.h>
#include <QPushButton.h>
#include <qtablewidget.h>
#include <qlineedit.h>

class AuxiliaryWindow;

/**/
class MicrowaveSystem : public IChimeraSystem{
	public:
		MicrowaveSystem(IChimeraQtWindow* parent);
		void handleContextMenu (const QPoint& pos);
		void initialize( QPoint& pos, IChimeraQtWindow* parentWin );
		std::string getIdentity();
		MicrowaveCore& getCore ();
		void handleSaveConfig (ConfigStream& saveFile);
		void setMicrowaveSettings (microwaveSettings settings);
		void programNow (std::vector<parameterType> constants);
		void refreshCurrentUwList ();
		void handleReadPress ();
		void handleWritePress ();
	private:
		MicrowaveCore core;
		QLabel* header;
		QPushButton* writeNow;
		QLineEdit* writeTxt;
		QPushButton* readNow;
		QLabel* readTxt;
		
		QLabel* triggerStepTimeLabel;
		QLineEdit* triggerStepTimeEdit;

		QTableWidget* uwListListview;
		QCheckBox* controlOptionCheck;
		QPushButton* programNowPush;
		std::vector<microwaveListEntry> currentList;

		void refreshListview ();
};

