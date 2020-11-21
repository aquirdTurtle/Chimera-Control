// created by Mark O. Brown
#pragma once

#include "GeneralFlumes/GpibFlume.h"
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
		void handleListviewDblClick ();
		void handleListviewRClick ();
		std::string getIdentity();
		MicrowaveCore& getCore ();
		void handleSaveConfig (ConfigStream& saveFile);
		void setMicrowaveSettings (microwaveSettings settings);
		void programNow (std::vector<parameterType> constants);
		void handleReadPress ();
		void handleWritePress ();
	private:
		MicrowaveCore core;
		QLabel* header;
		QLineEdit* writeTxt;
		QLabel* readTxt;
		// this is a readbtn-only control that shows the user how the RSG actually gets programmed in the end.
		QTableWidget* uwListListview;
		QCheckBox* controlOptionCheck;
		QPushButton* programNowPush;
		std::vector<microwaveListEntry> currentList;

		void refreshListview ();
};

