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
	experimentNotesHeader.Create( "EXPERIMENT NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
								  experimentNotesHeader.sPos, master, id++ );
	experimentNotesHeader.fontType = HeadingFont;
	
	// Configuration Notes edit
	experimentNotes.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 100 };
	experimentNotes.Create( WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL | WS_BORDER,
							experimentNotes.sPos, master, id++ );
	idVerify(experimentNotes, EXPERIMENT_NOTES_ID);
	/// CATEGORY LEVEL
	// Category Notes Title
	categoryNotesHeader.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 25 };
	categoryNotesHeader.Create( "CATEGORY NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, 
							   categoryNotesHeader.sPos, master, id++ );
	categoryNotesHeader.fontType = HeadingFont;
	// Category Notes edit
	categoryNotes.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 100 };
	
	categoryNotes.Create( WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL | WS_BORDER,
						  categoryNotes.sPos, master, id++ );
	idVerify(categoryNotes, CATEGORY_NOTES_ID);
	/// CONFIGURAITON LEVEL
	// Configuration Notes Title
	configurationNotesHeader.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 25 };
	configurationNotesHeader.Create( "CONFIGURATION NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
									 configurationNotesHeader.sPos, master, id++ );
	configurationNotesHeader.fontType = HeadingFont;
	//  Configuration Notes edit
	configurationNotes.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 100 };
	configurationNotes.Create( WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL | WS_BORDER,
							   configurationNotes.sPos, master, id++);
	idVerify(configurationNotes, CONFIGURATION_NOTES_ID);
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
	if (ctrlID == categoryNotes.GetDlgCtrlID() || ctrlID == experimentNotes.GetDlgCtrlID() || 
		ctrlID == configurationNotes.GetDlgCtrlID())
	{
		SetTextColor(hdcStatic, RGB(255, 255, 255));
		SetBkColor(hdcStatic, RGB(15, 15, 15));
		return (INT_PTR)brushes["Dark Grey"];
	}
	else if (ctrlID == categoryNotesHeader.GetDlgCtrlID() || ctrlID == experimentNotesHeader.GetDlgCtrlID()
			 || ctrlID == configurationNotesHeader.GetDlgCtrlID())
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


