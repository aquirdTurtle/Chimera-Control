#pragma once
#include "Control.h"
#include <string>
#include <unordered_map>

class NoteSystem
{
	public:
		void setExperimentNotes(std::string notes);
		void setCategoryNotes(std::string notes);
		void setConfigurationNotes(std::string notes);
		void initialize(POINT& topLeftPosition, MasterWindow* master, int& id);
		std::string getExperimentNotes();
		std::string getCategoryNotes();
		std::string getConfigurationNotes();
		INT_PTR handleColorMessage(HWND parent, UINT msg, WPARAM wParam, LPARAM lParam, std::unordered_map<std::string, HBRUSH> brushes);
	private:
		Control<CEdit> experimentNotes;
		Control<CStatic> experimentNotesHeader;
		Control<CEdit> categoryNotes;
		Control<CStatic> categoryNotesHeader;
		Control<CEdit> configurationNotes;
		Control<CStatic> configurationNotesHeader;
};