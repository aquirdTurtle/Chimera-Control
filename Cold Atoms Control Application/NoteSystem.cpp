#include "stdafx.h";
#include "Windows.h"
#include <string>
#include "NoteSystem.h";
#include "fonts.h";
#include "constants.h";

bool NoteSystem::initializeControls(POINT& topLeftPosition, HWND parentWindow)
{
	RECT currentRect;
	/// EXPERIMENT LEVEL
	// Configuration Notes Title
	currentRect = experimentNotesHeader.position = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 20};
	experimentNotesHeader.hwnd = CreateWindowEx(NULL, "STATIC", "EXPERIMENT NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top,
		parentWindow, (HMENU)IDC_EXPERIMENT_NOTES_HEADER, GetModuleHandle(NULL), NULL);
	SendMessage(experimentNotesHeader.hwnd, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	topLeftPosition.y += 20;
	// Configuration Notes edit
	currentRect = experimentNotes.position = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 200 };
	experimentNotes.hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top,
		parentWindow, (HMENU)IDC_EXPERIMENT_NOTES, GetModuleHandle(NULL), NULL);
	SendMessage(experimentNotes.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	topLeftPosition.y += 200;
	/// CATEGORY LEVEL
	// Category Notes Title
	currentRect = categoryNotesHeader.position = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 20 };
	categoryNotesHeader.hwnd = CreateWindowEx(NULL, "STATIC", "CATEGORY NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top,
		parentWindow, (HMENU)IDC_CATEGORY_NOTES_HEADER, GetModuleHandle(NULL), NULL);
	SendMessage(categoryNotesHeader.hwnd, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	topLeftPosition.y += 20;
	//  Category Notes edit
	currentRect = categoryNotes.position = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 200 };
	categoryNotes.hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top,
		parentWindow, (HMENU)IDC_CATEGORY_NOTES, GetModuleHandle(NULL), NULL);
	SendMessage(categoryNotes.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	topLeftPosition.y += 200;
	/// CONFIGURAITON LEVEL
	// Configuration Notes Title
	currentRect = configurationNotesHeader.position = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 20 };
	configurationNotesHeader.hwnd = CreateWindowEx(NULL, "STATIC", "CONFIGURAITON NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top,
		parentWindow, (HMENU)IDC_CONFIGURATION_NOTES_HEADER, GetModuleHandle(NULL), NULL);
	SendMessage(configurationNotesHeader.hwnd, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	topLeftPosition.y += 20;
	//  Configuration Notes edit
	currentRect = configurationNotes.position = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 200 };
	configurationNotes.hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top,
		parentWindow, (HMENU)IDC_CONFIGURATION_NOTES, GetModuleHandle(NULL), NULL);
	SendMessage(configurationNotes.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	topLeftPosition.y += 200;
	return false;
}
bool NoteSystem::setExperimentNotes(std::string notes)
{
	SendMessage(experimentNotes.hwnd, WM_SETTEXT, 0, (LPARAM)notes.c_str());
	return false;
}
bool NoteSystem::setCategoryNotes(std::string notes)
{
	SendMessage(categoryNotes.hwnd, WM_SETTEXT, 0, (LPARAM)notes.c_str());
	return false;
}
bool NoteSystem::setConfigurationNotes(std::string notes)
{
	SendMessage(configurationNotes.hwnd, WM_SETTEXT, 0, (LPARAM)notes.c_str());
	return false;
}
std::string NoteSystem::getExperimentNotes()
{
	int length = SendMessage(experimentNotes.hwnd, WM_GETTEXTLENGTH, 0, 0);
	TCHAR* rawText = new TCHAR[length + 1];
	SendMessage(experimentNotes.hwnd, WM_GETTEXT, length + 1, (LPARAM)rawText);
	std::string text(rawText);
	delete rawText;
	return text;
}
std::string NoteSystem::getCategoryNotes()
{
	int length = SendMessage(categoryNotes.hwnd, WM_GETTEXTLENGTH, 0, 0);
	TCHAR* rawText = new TCHAR[length + 1];
	SendMessage(categoryNotes.hwnd, WM_GETTEXT, length + 1, (LPARAM)rawText);
	std::string text(rawText);
	delete rawText;
	return text;
}
std::string NoteSystem::getConfigurationNotes()
{
	int length = SendMessage(configurationNotes.hwnd, WM_GETTEXTLENGTH, 0, 0);
	TCHAR* rawText = new TCHAR[length + 1];
	SendMessage(configurationNotes.hwnd, WM_GETTEXT, length + 1, (LPARAM)rawText);
	std::string text(rawText);
	delete rawText;
	return text;
}