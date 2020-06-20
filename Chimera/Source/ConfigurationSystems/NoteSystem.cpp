// created by Mark O. Brown
#include "stdafx.h"
#include "Windows.h"
#include <string>
#include "NoteSystem.h"
#include "LowLevel/constants.h"
#include "ConfigurationSystems/ProfileSystem.h"

void NoteSystem::rearrange(int width, int height, fontMap fonts)
{
	configNotes.rearrange( width, height, fonts);
	configNotesHeader.rearrange( width, height, fonts);
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


void NoteSystem::handleOpenConfig(std::ifstream& openFile, Version ver )
{
	/// handle notes
	std::string notes;
	std::string tempNote;
	// no need to get a newline since this should be he first thing in the file.
	openFile.get();
	auto pos = openFile.tellg ( );
	std::getline(openFile, tempNote);
	if (tempNote != "END_CONFIGURATION_NOTES")
	{
		while (openFile && tempNote != "END_CONFIGURATION_NOTES")
		{
			notes += tempNote + "\r\n";
			pos = openFile.tellg ( );
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
	// for consistency with other open functions, the end delimiter will be readbtn outside this function, so go back one 
	// line
	openFile.seekg ( pos );
}


void NoteSystem::initialize(POINT& topLeftPos, CWnd* parentWindow, int& id, cToolTips& tooltips)
{
	/// CONFIGURATION LEVEL
	// Configuration Notes Title
	configNotesHeader.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 25 };
	configNotesHeader.Create( "CONFIGURATION NOTES", NORM_HEADER_OPTIONS, configNotesHeader.sPos, parentWindow, id++);
	configNotesHeader.fontType = fontTypes::HeadingFont;
	topLeftPos.y += 25;
	//  Configuration Notes edit
	configNotes.sPos = { topLeftPos.x, topLeftPos.y, topLeftPos.x + 480, topLeftPos.y + 195 };
	configNotes.Create( NORM_EDIT_OPTIONS | ES_AUTOVSCROLL | ES_WANTRETURN, configNotes.sPos, parentWindow, 
						IDC_CONFIGURATION_NOTES );
	topLeftPos.y += 195;
}

void NoteSystem::setConfigurationNotes(std::string notes)
{
	configNotes.SetWindowTextA(cstr(notes));
}

std::string NoteSystem::getConfigurationNotes()
{
	CString rawText;
	configNotes.GetWindowTextA(rawText);
	std::string text(rawText);
	return text;
}
