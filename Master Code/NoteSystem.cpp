#include "stdafx.h"
#include "Windows.h"
#include <string>
#include "NoteSystem.h"
#include "fonts.h"
#include "constants.h"
#include <unordered_map>

NoteSystem::NoteSystem(int& startID)
{
	experimentNotesHeader.ID = startID;
	experimentNotes.ID = startID + 1;
	if (experimentNotes.ID != EXPERIMENT_NOTES_ID)
	{
		errBox("ERROR: experimentNotes.ID doesn't match EXPERIMENT_NOTES_ID!: " + std::to_string(experimentNotes.ID) + " and " + std::to_string(EXPERIMENT_NOTES_ID));
	}
	categoryNotesHeader.ID = startID + 2;
	categoryNotes.ID = startID + 3;
	if (categoryNotes.ID != CATEGORY_NOTES_ID)
	{
		errBox("ERROR: categoryNotes.ID doesn't match CATEGORY_NOTES_ID!: " + std::to_string(categoryNotes.ID) + " and " + std::to_string(CATEGORY_NOTES_ID));
	}
	configurationNotesHeader.ID = startID + 4;
	configurationNotes.ID = startID + 5;
	if (configurationNotes.ID != CONFIGURATION_NOTES_ID)
	{
		errBox("ERROR: configurationNotes.ID doesn't match CONFIGURATION_NOTES_ID!: " + std::to_string(configurationNotes.ID) + " and " + std::to_string(CONFIGURATION_NOTES_ID));
	}

	startID += 6;
}

bool NoteSystem::initializeControls(POINT& topLeftPosition, HWND parentWindow)
{
	RECT currentRect;
	/// EXPERIMENT LEVEL
	// Configuration Notes Title
	currentRect = experimentNotesHeader.position = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 20};
	experimentNotesHeader.Create("EXPERIMENT NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, currentRect, CWnd::FromHandle(parentWindow),
		experimentNotesHeader.ID);
	experimentNotesHeader.SetFont(CFont::FromHandle(sNormalFont));
	topLeftPosition.y += 20;
	// Configuration Notes edit
	currentRect = experimentNotes.position = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 200 };
	experimentNotes.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL | WS_BORDER, currentRect, CWnd::FromHandle(parentWindow), experimentNotes.ID);
	experimentNotes.SetFont(CFont::FromHandle(sNormalFont));
	topLeftPosition.y += 200;
	/// CATEGORY LEVEL
	// Category Notes Title
	currentRect = categoryNotesHeader.position = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 20 };
	categoryNotesHeader.Create("CATEGORY NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, currentRect, CWnd::FromHandle(parentWindow),
		categoryNotesHeader.ID);
	categoryNotesHeader.SetFont(CFont::FromHandle(sNormalFont));
	topLeftPosition.y += 20;
	// Category Notes edit
	currentRect = categoryNotes.position = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 200 };
	categoryNotes.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL | WS_BORDER, currentRect, CWnd::FromHandle(parentWindow), categoryNotes.ID);
	categoryNotes.SetFont(CFont::FromHandle(sNormalFont));
	topLeftPosition.y += 200;
	/// CONFIGURAITON LEVEL
	// Configuration Notes Title
	currentRect = configurationNotesHeader.position = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 20 };
	configurationNotesHeader.Create("CONFIGURATION NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, currentRect, CWnd::FromHandle(parentWindow),
		configurationNotesHeader.ID);
	configurationNotesHeader.SetFont(CFont::FromHandle(sNormalFont));
	topLeftPosition.y += 20;
	//  Configuration Notes edit
	currentRect = configurationNotes.position = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 200 };
	configurationNotes.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL | WS_BORDER, currentRect, CWnd::FromHandle(parentWindow), configurationNotes.ID);
	
	configurationNotes.SetFont(CFont::FromHandle(sNormalFont));
	topLeftPosition.y += 200;
	return false;
}
bool NoteSystem::setExperimentNotes(std::string notes)
{
	experimentNotes.SetWindowText(notes.c_str());
	//SendMessage(experimentNotes.hwnd, WM_SETTEXT, 0, (LPARAM)notes.c_str());
	return false;
}
bool NoteSystem::setCategoryNotes(std::string notes)
{
	categoryNotes.SetWindowText(notes.c_str());
	//SendMessage(categoryNotes.hwnd, WM_SETTEXT, 0, (LPARAM)notes.c_str());
	return false;
}
bool NoteSystem::setConfigurationNotes(std::string notes)
{
	configurationNotes.SetWindowText(notes.c_str());
	//SendMessage(configurationNotes.hwnd, WM_SETTEXT, 0, (LPARAM)notes.c_str());
	return false;
}
std::string NoteSystem::getExperimentNotes()
{
	CString text;
	experimentNotes.GetWindowText(text);
	return text;
	/*
	int length = SendMessage(experimentNotes.hwnd, WM_GETTEXTLENGTH, 0, 0);
	TCHAR* rawText = new TCHAR[length + 1];
	SendMessage(experimentNotes.hwnd, WM_GETTEXT, length + 1, (LPARAM)rawText);
	std::string text(rawText);
	delete rawText;
	return text;
	*/
}
std::string NoteSystem::getCategoryNotes()
{
	CString text;
	categoryNotes.GetWindowText(text);
	return text;
	/*
	int length = SendMessage(categoryNotes.hwnd, WM_GETTEXTLENGTH, 0, 0);
	TCHAR* rawText = new TCHAR[length + 1];
	SendMessage(categoryNotes.hwnd, WM_GETTEXT, length + 1, (LPARAM)rawText);
	std::string text(rawText);
	delete rawText;
	return text;
	*/
}
std::string NoteSystem::getConfigurationNotes()
{
	CString text;
	configurationNotes.GetWindowText(text);
	return text;
	/*
	int length = SendMessage(configurationNotes.hwnd, WM_GETTEXTLENGTH, 0, 0);
	TCHAR* rawText = new TCHAR[length + 1];
	SendMessage(configurationNotes.hwnd, WM_GETTEXT, length + 1, (LPARAM)rawText);
	std::string text(rawText);
	delete rawText;
	return text;
	*/
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


