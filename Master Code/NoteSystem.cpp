#include "stdafx.h"
#include "Windows.h"
#include <string>
#include "NoteSystem.h"
#include "fonts.h"
#include "constants.h"
#include <unordered_map>
#include "MasterWindow.h"


void NoteSystem::initialize( POINT& pos, MasterWindow* master, int& id )
{
	/// EXPERIMENT LEVEL
	// Configuration Notes Title
	experimentNotesHeader.position = { pos.x, pos.y, pos.x + 480, pos.y += 20 };
	experimentNotesHeader.ID = id++;
	experimentNotesHeader.Create( "EXPERIMENT NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
								  experimentNotesHeader.position, master, experimentNotesHeader.ID );
	experimentNotesHeader.SetFont( CFont::FromHandle( sNormalFont ) );
	
	// Configuration Notes edit
	experimentNotes.position = { pos.x, pos.y, pos.x + 480, pos.y += 100 };
	experimentNotes.ID = id++;
	if (experimentNotes.ID != EXPERIMENT_NOTES_ID)
	{
		throw;
	}
	experimentNotes.Create( WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL | WS_BORDER,
							experimentNotes.position, master, experimentNotes.ID );
	experimentNotes.SetFont( CFont::FromHandle( sNormalFont ) );
	/// CATEGORY LEVEL
	// Category Notes Title
	categoryNotesHeader.position = { pos.x, pos.y, pos.x + 480, pos.y += 20 };
	categoryNotesHeader.ID = id++;
	categoryNotesHeader.Create( "CATEGORY NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, categoryNotesHeader.position, master,
								categoryNotesHeader.ID );
	categoryNotesHeader.SetFont( CFont::FromHandle( sNormalFont ) );
	// Category Notes edit
	categoryNotes.position = { pos.x, pos.y, pos.x + 480, pos.y += 100 };
	categoryNotes.ID = id++;
	if (categoryNotes.ID != CATEGORY_NOTES_ID)
	{
		throw;
	}
	categoryNotes.Create( WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL | WS_BORDER,
						  categoryNotes.position, master, categoryNotes.ID );
	categoryNotes.SetFont( CFont::FromHandle( sNormalFont ) );
	/// CONFIGURAITON LEVEL
	// Configuration Notes Title
	configurationNotesHeader.position = { pos.x, pos.y, pos.x + 480, pos.y += 20 };
	configurationNotesHeader.ID = id++;
	configurationNotesHeader.Create( "CONFIGURATION NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
									 configurationNotesHeader.position, master, configurationNotesHeader.ID );
	configurationNotesHeader.SetFont( CFont::FromHandle( sNormalFont ) );
	//  Configuration Notes edit
	configurationNotes.position = { pos.x, pos.y, pos.x + 480, pos.y += 100 };
	configurationNotes.ID = id++;
	if (configurationNotes.ID != CONFIGURATION_NOTES_ID)
	{
		throw;
	}
	configurationNotes.Create( WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL | WS_BORDER,
							   configurationNotes.position, master, configurationNotes.ID );
	configurationNotes.SetFont( CFont::FromHandle( sNormalFont ) );
	return;
}


void NoteSystem::setExperimentNotes(std::string notes)
{
	experimentNotes.SetWindowText(notes.c_str());
	return;
}


void NoteSystem::setCategoryNotes(std::string notes)
{
	categoryNotes.SetWindowText(notes.c_str());
	return;
}


void NoteSystem::setConfigurationNotes(std::string notes)
{
	configurationNotes.SetWindowText(notes.c_str());
	return;
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


INT_PTR NoteSystem::handleColorMessage(HWND parent, UINT msg, WPARAM wParam, LPARAM lParam, std::unordered_map<std::string, HBRUSH> brushes)
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


