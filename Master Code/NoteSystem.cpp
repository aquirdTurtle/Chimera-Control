#include "stdafx.h"
#include "Windows.h"
#include <string>
#include "NoteSystem.h"
#include "fonts.h"
#include "constants.h"
#include <unordered_map>
#include "MasterWindow.h"

void NoteSystem::setActiveControls(std::string level)
{
	if (level == "configuration")
	{
		configurationNotes.EnableWindow();
		categoryNotes.EnableWindow();
		experimentNotes.EnableWindow();
	}
	else if (level == "category")
	{
		configurationNotes.EnableWindow(false);
		categoryNotes.EnableWindow();
		experimentNotes.EnableWindow();
	}
	else if (level == "experiment")
	{
		configurationNotes.EnableWindow(false);
		categoryNotes.EnableWindow(false);
		experimentNotes.EnableWindow();
	}
	else if (level == "none")
	{
		configurationNotes.EnableWindow(false);
		categoryNotes.EnableWindow(false);
		experimentNotes.EnableWindow(false);
	}
	else
	{
		thrower("Bad level in setActiveControls call!");
	}
}


void NoteSystem::initialize( POINT& pos, MasterWindow* master, int& id )
{
	/// EXPERIMENT LEVEL
	// Configuration Notes Title
	experimentNotesHeader.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 25 };
	experimentNotesHeader.ID = id++;
	experimentNotesHeader.Create( "EXPERIMENT NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
								  experimentNotesHeader.sPos, master, experimentNotesHeader.ID );
	experimentNotesHeader.fontType = Heading;
	
	// Configuration Notes edit
	experimentNotes.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 100 };
	experimentNotes.ID = id++;
	if (experimentNotes.ID != EXPERIMENT_NOTES_ID)
	{
		throw;
	}
	experimentNotes.Create( WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL | WS_BORDER,
							experimentNotes.sPos, master, experimentNotes.ID );
	experimentNotes.fontType = Normal;
	/// CATEGORY LEVEL
	// Category Notes Title
	categoryNotesHeader.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 25 };
	categoryNotesHeader.ID = id++;
	categoryNotesHeader.Create( "CATEGORY NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, categoryNotesHeader.sPos, master,
								categoryNotesHeader.ID );
	categoryNotesHeader.fontType = Heading;
	// Category Notes edit
	categoryNotes.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 100 };
	categoryNotes.ID = id++;
	if (categoryNotes.ID != CATEGORY_NOTES_ID)
	{
		throw;
	}
	categoryNotes.Create( WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL | WS_BORDER,
						  categoryNotes.sPos, master, categoryNotes.ID );
	categoryNotes.fontType = Normal;
	/// CONFIGURAITON LEVEL
	// Configuration Notes Title
	configurationNotesHeader.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 25 };
	configurationNotesHeader.ID = id++;
	configurationNotesHeader.Create( "CONFIGURATION NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
									 configurationNotesHeader.sPos, master, configurationNotesHeader.ID );
	configurationNotesHeader.fontType = Heading;
	//  Configuration Notes edit
	configurationNotes.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 100 };
	configurationNotes.ID = id++;
	if (configurationNotes.ID != CONFIGURATION_NOTES_ID)
	{
		throw;
	}
	configurationNotes.Create( WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL | WS_BORDER,
							   configurationNotes.sPos, master, configurationNotes.ID );
	configurationNotes.fontType = Normal;
}

void NoteSystem::rearrange(UINT width, UINT height, fontMap fonts)
{
	experimentNotes.rearrange("", "", width, height, fonts);
	experimentNotesHeader.rearrange("", "", width, height, fonts);
	categoryNotes.rearrange("", "", width, height, fonts);
	categoryNotesHeader.rearrange("", "", width, height, fonts);
	configurationNotes.rearrange("", "", width, height, fonts);
	configurationNotesHeader.rearrange("", "", width, height, fonts);
}

void NoteSystem::setExperimentNotes(std::string notes)
{
	experimentNotes.SetWindowText(notes.c_str());
}


void NoteSystem::setCategoryNotes(std::string notes)
{
	categoryNotes.SetWindowText(notes.c_str());
}


void NoteSystem::setConfigurationNotes(std::string notes)
{
	configurationNotes.SetWindowText(notes.c_str());
}


std::string NoteSystem::getExperimentNotes()
{
	CString text;
	experimentNotes.GetWindowText( text );
	return text;
}


std::string NoteSystem::getCategoryNotes()
{
	CString text;
	categoryNotes.GetWindowText(text);
	return text;
}


std::string NoteSystem::getConfigurationNotes()
{
	CString text;
	configurationNotes.GetWindowText(text);
	return text;
}


INT_PTR NoteSystem::handleColorMessage(WPARAM wParam, LPARAM lParam, brushMap brushes)
{
	DWORD ctrlID = GetDlgCtrlID((HWND)lParam); // Window Control ID
	HDC hdcStatic = (HDC)wParam;
	if (ctrlID == this->categoryNotes.ID || ctrlID == this->experimentNotes.ID || ctrlID == this->configurationNotes.ID)
	{
		SetTextColor(hdcStatic, RGB(255, 255, 255));
		SetBkColor(hdcStatic, RGB(15, 15, 15));
		return (INT_PTR)brushes["Dark Grey"];
	}
	else if (ctrlID == this->categoryNotesHeader.ID || ctrlID == this->experimentNotesHeader.ID || ctrlID == this->configurationNotesHeader.ID)
	{
		SetTextColor(hdcStatic, RGB(218, 165, 32));
		SetBkColor(hdcStatic, RGB(30, 30, 30));
		return (INT_PTR)brushes["Medium Grey"];
	}
	else
	{
		return NULL;
	}
}


