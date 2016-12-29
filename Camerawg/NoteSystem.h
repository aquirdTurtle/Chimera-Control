#pragma once
#include "Control.h"
#include <string>


class NoteSystem
{
	public:
		bool setExperimentNotes(std::string notes);
		bool setCategoryNotes(std::string notes);
		bool setConfigurationNotes(std::string notes);
		bool initializeControls(POINT& topLeftPosition, CWnd* parentWindow, int& id,
			std::unordered_map<std::string, CFont*> fonts, std::vector<CToolTipCtrl*>& tooltips);
		std::string getExperimentNotes();
		std::string getCategoryNotes();
		std::string getConfigurationNotes();
	private:
		Control<CEdit> experimentNotes;
		Control<CEdit> categoryNotes;
		Control<CEdit> configurationNotes;
		Control<CStatic> experimentNotesHeader;
		Control<CStatic> categoryNotesHeader;
		Control<CStatic> configurationNotesHeader;
};