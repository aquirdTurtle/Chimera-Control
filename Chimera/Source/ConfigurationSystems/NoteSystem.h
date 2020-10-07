// created by Mark O. Brown
#pragma once

#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "GeneralObjects/commonTypes.h"

#include <QLabel>
#include <QTextEdit>
#include <PrimaryWindows/IChimeraQtWindow.h>
#include <CustomQtControls/AutoNotifyCtrls.h>

#include <string>

class NoteSystem{
	public:
		void handleSaveConfig(ConfigStream& saveFile);
		void handleOpenConfig(ConfigStream& openFile );
		void setConfigurationNotes(std::string notes);
		void initialize( QPoint& topLeftPos, IChimeraQtWindow* parent );
		std::string getConfigurationNotes();
	private:
		QLabel* header;
		CQTextEdit* edit;
};
