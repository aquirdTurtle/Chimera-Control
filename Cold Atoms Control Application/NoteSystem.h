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
		HwndControl experimentNotes;
		HwndControl experimentNotesHeader;
		HwndControl categoryNotes;
		HwndControl categoryNotesHeader;
		HwndControl configurationNotes;
		HwndControl configurationNotesHeader;
};