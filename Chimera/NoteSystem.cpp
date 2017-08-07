#include "stdafx.h"
#include "Windows.h"
#include <string>
#include "NoteSystem.h"
#include "fonts.h"
#include "constants.h"

void NoteSystem::rearrange(int width, int height, fontMap fonts)
{
	experimentNotes.rearrange("", "", width, height, fonts);
	categoryNotes.rearrange("", "", width, height, fonts);
	configurationNotes.rearrange("", "", width, height, fonts);
	experimentNotesHeader.rearrange("", "", width, height, fonts);
	categoryNotesHeader.rearrange("", "", width, height, fonts);
	configurationNotesHeader.rearrange("", "", width, height, fonts);
}

void NoteSystem::initializeControls(POINT& topLeftPos, CWnd* parentWindow, int& id, fontMap fonts, 
									std::vector<CToolTipCtrl*>& tooltips)
{
	/// EXPERIMENT LEVEL
	experimentNotesHeader.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 20};
	experimentNotesHeader.Create("EXPERIMENT NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, 
								 experimentNotesHeader.sPos, parentWindow, id++ );
	experimentNotesHeader.SetFont(fonts["Heading Font"]);
	topLeftPos.y += 20;
	//
	experimentNotes.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 200 };
	experimentNotes.Create( WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL, experimentNotes.sPos, 
						    parentWindow, id++ );
	experimentNotes.SetFont(fonts["Normal Font"]);
	topLeftPos.y += 200;
	/// CATEGORY LEVEL
	// Category Notes Title
	categoryNotesHeader.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 20 };
	categoryNotesHeader.Create("CATEGORY NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, categoryNotesHeader.sPos, 
							   parentWindow, id++ );
	categoryNotesHeader.SetFont(fonts["Heading Font"]);
	topLeftPos.y += 20;
	//  Category Notes edit
	categoryNotes.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 200 };
	categoryNotes.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL, categoryNotes.sPos, 
						 parentWindow, id++ );
	categoryNotes.SetFont(fonts["Normal Font"]);
	topLeftPos.y += 200;
	/// CONFIGURAITON LEVEL
	// Configuration Notes Title
	configurationNotesHeader.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 20 };
	configurationNotesHeader.Create("CONFIGURAITON NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, 
									configurationNotesHeader.sPos, parentWindow, id++);
	configurationNotesHeader.SetFont(fonts["Heading Font"]);
	topLeftPos.y += 20;
	//  Configuration Notes edit
	configurationNotes.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 215 };
	configurationNotes.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL, 
							  configurationNotes.sPos, parentWindow, id++ );
	configurationNotes.SetFont(fonts["Normal Font"]);
	topLeftPos.y += 215;
}

void NoteSystem::setExperimentNotes(std::string notes)
{
	experimentNotes.SetWindowTextA(notes.c_str());
}

void NoteSystem::setCategoryNotes(std::string notes)
{
	categoryNotes.SetWindowTextA(notes.c_str());
}

void NoteSystem::setConfigurationNotes(std::string notes)
{
	categoryNotes.SetWindowTextA(notes.c_str());
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
