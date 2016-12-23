#include "stdafx.h"
#include "Windows.h"
#include <string>
#include "NoteSystem.h"
#include "fonts.h"
#include "constants.h"

bool NoteSystem::initializeControls(POINT& topLeftPos, CWnd* parentWindow, int& id)
{
	/// EXPERIMENT LEVEL
	experimentNotesHeader.ID = id++;
	experimentNotesHeader.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 20};
	experimentNotesHeader.Create("EXPERIMENT NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, experimentNotesHeader.sPos, parentWindow, 
		experimentNotesHeader.ID);
	experimentNotesHeader.SetFont(&eHeadingFont);
	topLeftPos.y += 20;
	//
	experimentNotes.ID = id++;
	experimentNotes.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 200 };
	experimentNotes.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL, experimentNotes.sPos, parentWindow, 
		experimentNotes.ID);
	experimentNotes.SetFont(&eNormalFont);
	topLeftPos.y += 200;
	/// CATEGORY LEVEL
	// Category Notes Title
	categoryNotesHeader.ID = id++;
	categoryNotesHeader.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 20 };
	categoryNotesHeader.Create("CATEGORY NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, categoryNotesHeader.sPos, 
		parentWindow, categoryNotesHeader.ID);
	categoryNotesHeader.SetFont(&eHeadingFont);
	topLeftPos.y += 20;
	//  Category Notes edit
	categoryNotes.ID = id++;
	categoryNotes.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 200 };
	categoryNotes.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL, categoryNotes.sPos, parentWindow, categoryNotes.ID);
	categoryNotes.SetFont(&eNormalFont);
	topLeftPos.y += 200;
	/// CONFIGURAITON LEVEL
	// Configuration Notes Title
	configurationNotesHeader.ID = id++;
	configurationNotesHeader.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 20 };
	configurationNotesHeader.Create("CONFIGURAITON NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, configurationNotesHeader.sPos, parentWindow, id);
	configurationNotesHeader.SetFont(&eHeadingFont);
	topLeftPos.y += 20;
	//  Configuration Notes edit
	configurationNotes.ID = id++;
	configurationNotes.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 215 };
	configurationNotes.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL, configurationNotes.sPos, parentWindow, configurationNotes.ID);
	configurationNotes.SetFont(&eNormalFont);
	topLeftPos.y += 215;
	return false;
}

bool NoteSystem::setExperimentNotes(std::string notes)
{
	experimentNotes.SetWindowTextA(notes.c_str());
	return false;
}
bool NoteSystem::setCategoryNotes(std::string notes)
{
	categoryNotes.SetWindowTextA(notes.c_str());
	return false;
}
bool NoteSystem::setConfigurationNotes(std::string notes)
{
	categoryNotes.SetWindowTextA(notes.c_str());
	return false;
}
std::string NoteSystem::getExperimentNotes()
{
	CString rawText;
	experimentNotes.GetWindowTextA(rawText);
	std::string text(rawText);
	return text;
}
std::string NoteSystem::getCategoryNotes()
{
	CString rawText;
	categoryNotes.GetWindowTextA(rawText);
	std::string text(rawText);
	return text;
}
std::string NoteSystem::getConfigurationNotes()
{
	CString rawText;
	configurationNotes.GetWindowTextA(rawText);
	std::string text(rawText);
	return text;
}
