#pragma once
#include "Control.h"
#include <string>


class NoteSystem
{
	public:
		bool setExperimentNotes(std::string notes);
		bool setCategoryNotes(std::string notes);
		bool setConfigurationNotes(std::string notes);
		bool initializeControls(POINT& topLeftPosition, HWND parentWindow);
		std::string getExperimentNotes();
		std::string getCategoryNotes();
		std::string getConfigurationNotes();
	private:
		Control experimentNotes;
		Control experimentNotesHeader;
		Control categoryNotes;
		Control categoryNotesHeader;
		Control configurationNotes;
		Control configurationNotesHeader;
};