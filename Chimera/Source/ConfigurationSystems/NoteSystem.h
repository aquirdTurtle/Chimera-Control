// created by Mark O. Brown
#pragma once
#include "Control.h"
#include <string>
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "GeneralObjects/commonTypes.h"
#include <QLabel>
#include <QTextEdit>
#include <PrimaryWindows/IChimeraWindowWidget.h>
#include <CustomQtControls/AutoNotifyCtrls.h>
 
class NoteSystem
{
	public:
		void handleSaveConfig(ConfigStream& saveFile);
		void handleOpenConfig(ConfigStream& openFile );
		void setConfigurationNotes(std::string notes);
		void initialize( POINT& topLeftPos, IChimeraWindowWidget* parent );
		std::string getConfigurationNotes();
		void rearrange(int width, int height, fontMap fonts);
	private:
		QLabel* header;
		CQTextEdit* edit;
};
