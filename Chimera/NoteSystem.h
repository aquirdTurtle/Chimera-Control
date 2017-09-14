#pragma once
#include "Control.h"
#include <string>
#include "commonTypes.h"

class NoteSystem
{
	public:
		void handleNewConfig( std::ofstream& saveFile );
		void handleSaveConfig(std::ofstream& saveFile);
		void handleOpenConfig(std::ifstream& openFile, double version);
		void setExperimentNotes(std::string notes);
		void setCategoryNotes(std::string notes);
		void setConfigurationNotes(std::string notes);
		void initialize(POINT& topLeftPosition, CWnd* parentWindow, int& id, cToolTips& tooltips, std::array<UINT, 3> ids );
		std::string getExperimentNotes();
		std::string getCategoryNotes();
		std::string getConfigurationNotes();
		void rearrange(int width, int height, fontMap fonts);
	private:
		Control<CEdit> experimentNotes;
		Control<CEdit> categoryNotes;
		Control<CEdit> configurationNotes;
		Control<CStatic> experimentNotesHeader;
		Control<CStatic> categoryNotesHeader;
		Control<CStatic> configurationNotesHeader;
};