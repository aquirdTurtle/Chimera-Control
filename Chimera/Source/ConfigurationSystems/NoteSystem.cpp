// created by Mark O. Brown
#include "stdafx.h"
#include "NoteSystem.h"
#include <PrimaryWindows/QtMainWindow.h>
#include <QFile>
#include <string>

void NoteSystem::handleSaveConfig(ConfigStream& saveFile){
	saveFile << "CONFIGURATION_NOTES\n";
	saveFile << getConfigurationNotes();
	saveFile << "\nEND_CONFIGURATION_NOTES\n";
}

void NoteSystem::handleOpenConfig(ConfigStream& openFile){
	std::string notes;
	auto pos = openFile.tellg ( );
	std::string tempNote = openFile.getline();
	if (tempNote != "END_CONFIGURATION_NOTES"){
		while (openFile && tempNote != "END_CONFIGURATION_NOTES"){
			notes += tempNote + "\r\n";
			pos = openFile.tellg ( );
			std::getline(openFile, tempNote);
		}
		if (notes.size() > 2){
			notes = notes.substr(0, notes.size() - 2);
		}
		setConfigurationNotes(notes);
	}
	else{
		setConfigurationNotes("");
	}
	// for consistency with other open functions, the end delimiter will be readbtn outside this function, so go back 
	// one line
	openFile.seekg ( pos );
}

void NoteSystem::initialize(QPoint& topLeftPos, IChimeraQtWindow* win){
	unsigned noteSize = 80;
	auto & px = topLeftPos.rx(), & py = topLeftPos.ry();
	header = new QLabel ("CONFIGURATION NOTES", win);
	header->setGeometry (px, py, 480, 25);
	edit = new CQTextEdit (win);
	edit->setGeometry (px, py += 25, 480, noteSize);
	py += noteSize;
}

void NoteSystem::setConfigurationNotes(std::string notes){
	edit->setText (cstr(notes));
}

std::string NoteSystem::getConfigurationNotes(){
	std::string text = str(edit->toPlainText());
	return text;
}

