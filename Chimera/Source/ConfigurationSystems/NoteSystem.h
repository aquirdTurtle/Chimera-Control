// created by Mark O. Brown
#pragma once
#include "Control.h"
#include <string>
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "GeneralObjects/commonTypes.h"

class NoteSystem
{
	public:
		void handleSaveConfig(ConfigStream& saveFile);
		void handleOpenConfig(ConfigStream& openFile );
		void setConfigurationNotes(std::string notes);
		void initialize( POINT& topLeftPos, CWnd* parentWindow, int& id, cToolTips& tooltips );		
		std::string getConfigurationNotes();
		void rearrange(int width, int height, fontMap fonts);
	private:
		Control<CEdit> configNotes;
		Control<CStatic> configNotesHeader;
};
