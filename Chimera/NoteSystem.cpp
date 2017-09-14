#include "stdafx.h"
#include "Windows.h"
#include <string>
#include "NoteSystem.h"
#include "constants.h"

void NoteSystem::rearrange(int width, int height, fontMap fonts)
{
	experimentNotes.rearrange( width, height, fonts);
	categoryNotes.rearrange( width, height, fonts);
	configurationNotes.rearrange( width, height, fonts);
	experimentNotesHeader.rearrange( width, height, fonts);
	categoryNotesHeader.rearrange( width, height, fonts);
	configurationNotesHeader.rearrange( width, height, fonts);
}


void NoteSystem::handleNewConfig( std::ofstream& saveFile )
{
	saveFile << "CONFIGURATION_NOTES\n";
	saveFile << ">>>Enter configuration-level notes here<<<\n";
	saveFile << "\nEND_CONFIGURATION_NOTES\n";
}

void NoteSystem::handleSaveConfig(std::ofstream& saveFile)
{
	saveFile << "CONFIGURATION_NOTES\n";
	saveFile << getConfigurationNotes();
	saveFile << "\nEND_CONFIGURATION_NOTES\n";
}


void NoteSystem::handleOpenConfig(std::ifstream& openFile, double version)
{
	ProfileSystem::checkDelimiterLine(openFile, "CONFIGURATION_NOTES");
	/// handle notes
	std::string notes;
	std::string tempNote;
	// no need to get a newline since this should be he first thing in the file.
	openFile.get();
	std::getline(openFile, tempNote);
	if (tempNote != "END_CONFIGURATION_NOTES")
	{
		while (openFile && tempNote != "END_CONFIGURATION_NOTES")
		{
			notes += tempNote + "\r\n";
			std::getline(openFile, tempNote);
		}
		if (notes.size() > 2)
		{
			notes = notes.substr(0, notes.size() - 2);
		}
		setConfigurationNotes(notes);
	}
	else
	{
		setConfigurationNotes("");
	}
}


void NoteSystem::initialize(POINT& topLeftPos, CWnd* parentWindow, int& id, cToolTips& tooltips, std::array<UINT, 3> ids)
{
	/// EXPERIMENT LEVEL
	experimentNotesHeader.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 25};
	experimentNotesHeader.Create( "EXPERIMENT NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, 
								  experimentNotesHeader.sPos, parentWindow, id++ );
	experimentNotesHeader.fontType = HeadingFont;
	topLeftPos.y += 25;
	//
	experimentNotes.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 200 };
	experimentNotes.Create( WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL | ES_WANTRETURN, experimentNotes.sPos,
						    parentWindow, ids[0]++ );
	topLeftPos.y += 200;
	/// CATEGORY LEVEL
	// Category Notes Title
	categoryNotesHeader.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 25 };
	categoryNotesHeader.Create( "CATEGORY NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, categoryNotesHeader.sPos, 
							    parentWindow, id++ );
	categoryNotesHeader.fontType = HeadingFont;
	topLeftPos.y += 25;
	//  Category Notes edit
	categoryNotes.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 200 };
	categoryNotes.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL | ES_WANTRETURN, categoryNotes.sPos,
						 parentWindow, ids[1]++ );
	topLeftPos.y += 200;
	/// CONFIGURAITON LEVEL
	// Configuration Notes Title
	configurationNotesHeader.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 25 };
	configurationNotesHeader.Create( "CONFIGURAITON NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, 
									 configurationNotesHeader.sPos, parentWindow, id++);
	configurationNotesHeader.fontType = HeadingFont;
	topLeftPos.y += 25;
	//  Configuration Notes edit
	configurationNotes.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 195 };
	configurationNotes.Create( WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL | ES_WANTRETURN,
							   configurationNotes.sPos, parentWindow, ids[2]++ );
	topLeftPos.y += 195;
}

void NoteSystem::setExperimentNotes(std::string notes)
{
	experimentNotes.SetWindowTextA(cstr(notes));
}

void NoteSystem::setCategoryNotes(std::string notes)
{
	categoryNotes.SetWindowTextA(cstr(notes));
}

void NoteSystem::setConfigurationNotes(std::string notes)
{
	configurationNotes.SetWindowTextA(cstr(notes));
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
