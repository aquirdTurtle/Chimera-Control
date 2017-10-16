#pragma once
#include "Control.h"
#include <string>
#include "commonTypes.h"

class NoteSystem
{
	public:
		void handleNewConfig( std::ofstream& saveFile );
		void handleSaveConfig(std::ofstream& saveFile);
		void handleOpenConfig(std::ifstream& openFile, int versionMajor, int versionMinor );
		void setConfigurationNotes(std::string notes);
		void initialize( POINT& topLeftPos, CWnd* parentWindow, int& id, cToolTips& tooltips );		
		std::string getConfigurationNotes();
		void rearrange(int width, int height, fontMap fonts);
	private:
		Control<CEdit> configNotes;
		Control<CStatic> configNotesHeader;
};