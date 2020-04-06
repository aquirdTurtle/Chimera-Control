// created by Mark O. Brown
#pragma once
#include "Control.h"
#include <string>
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "GeneralObjects/commonTypes.h"
#include "QWinWidget.h"
#include <QLabel>
#include <QTextEdit>

 
class NoteSystem
{
	public:
		void handleSaveConfig(ConfigStream& saveFile);
		void handleOpenConfig(ConfigStream& openFile );
		void setConfigurationNotes(std::string notes);
		void initialize( POINT& topLeftPos, QWinWidget* parent, int& id, cToolTips& tooltips );
		std::string getConfigurationNotes();
		void rearrange(int width, int height, fontMap fonts);
	private:
		//Control<CEdit> configNotes;
		//Control<CStatic> configNotesHeader;
		QLabel* header;
		QTextEdit* edit;
};
