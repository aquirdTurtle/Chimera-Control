#include "stdafx.h"
#include "initializeMainWindow.h"
#include "Windows.h"
#include "externals.h"
#include "constants.h"
#include "fonts.h"

/*
 * This function is called on WM_CREATE in order to initialize the handles to all of the objects on the main window.
 */
int initializeMainWindow(HWND thisWindowHandle)
{
	eMainWindowHandle = thisWindowHandle;
	LoadLibrary(TEXT("Msftedit.dll"));

	RECT currentRect;

	// set formatting for these scripts
	CHARFORMAT myCharFormat;
	memset(&myCharFormat, 0, sizeof(CHARFORMAT));
	myCharFormat.cbSize = sizeof(CHARFORMAT);
	myCharFormat.dwMask = CFM_COLOR;

	/// EXPERIMENT STATUS
	//  Experiment Status
	eExperimentStatusTextHandle = CreateWindowEx(NULL, "STATIC", "Experiment Status", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		0, 0, 380, 20, thisWindowHandle, (HMENU)IDC_STATIC_4_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eExperimentStatusTextHandle, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	// clear button
	eScriptStatusClearButtonHandle = CreateWindowEx(NULL, "BUTTON", "clear", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		380, 0, 100, 20, thisWindowHandle, (HMENU)IDC_SCRIPT_STATUS_CLEAR_BUTTON, GetModuleHandle(NULL), NULL);
	SendMessage(eScriptStatusClearButtonHandle, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	// System Status
	eSystemStatusTextHandle = CreateWindowEx(NULL, _T("RICHEDIT50W"), "",
		WS_CHILD | WS_VISIBLE | ES_READONLY | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER,
		0, 20, 480, 975, thisWindowHandle, (HMENU)IDC_SYSTEM_STATUS_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eSystemStatusTextHandle, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	SendMessage(eSystemStatusTextHandle, EM_SETBKGNDCOLOR, 0, RGB(15, 15, 20));
	myCharFormat.crTextColor = RGB(50, 50, 250);
	SendMessage(eSystemStatusTextHandle, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&myCharFormat);

	///	ERROR STATUS
	// Title
	eErrorStatusTextDisplay = CreateWindowEx(NULL, "STATIC", "Error Status", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		480, 0, 380, 20, thisWindowHandle, (HMENU)IDC_ERROR_STATUS_TEXT_DISPLAY, GetModuleHandle(NULL), NULL);
	SendMessage(eErrorStatusTextDisplay, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	// Error Clear Button
	eScriptErrorClearButtonHandle = CreateWindowEx(NULL, "BUTTON", "clear", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		860, 0, 100, 20, thisWindowHandle, (HMENU)IDC_SCRIPT_ERROR_CLEAR_BUTTON, GetModuleHandle(NULL), NULL);
	SendMessage(eScriptErrorClearButtonHandle, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	// error edit
	eSystemErrorTextHandle = CreateWindowEx(NULL, _T("RICHEDIT50W"), "",
		WS_CHILD | WS_VISIBLE | ES_READONLY | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER,
		480, 20, 480, 480, thisWindowHandle, (HMENU)IDC_SYSTEM_ERROR_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eSystemErrorTextHandle, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	SendMessage(eSystemErrorTextHandle, EM_SETBKGNDCOLOR, 0, RGB(15, 15, 20));
	myCharFormat.crTextColor = RGB(200, 0, 0);
	SendMessage(eSystemErrorTextHandle, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&myCharFormat);

	/// DEBUG STATUS
	//  Debug Status
	eDebugStatusTextHandle = CreateWindowEx(NULL, "STATIC", "Debug Status", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		480, 500, 380, 20, thisWindowHandle, (HMENU)IDC_STATIC_5_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eDebugStatusTextHandle, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	// Debug Clear Button
	eScriptDebugClearButtonHandle = CreateWindowEx(NULL, "BUTTON", "clear", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		860, 500, 100, 20, thisWindowHandle, (HMENU)IDC_SCRIPT_DEBUG_CLEAR_BUTTON, GetModuleHandle(NULL), NULL);
	SendMessage(eScriptDebugClearButtonHandle, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	// Debug edit
	eSystemDebugTextHandle = CreateWindowEx(NULL, _T("RICHEDIT50W"), "",
		WS_CHILD | WS_VISIBLE | ES_READONLY | WS_HSCROLL | ES_AUTOHSCROLL | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER,
		480, 520, 480, 480, thisWindowHandle, (HMENU)IDC_SYSTEM_DEBUG_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eSystemDebugTextHandle, WM_SETFONT, WPARAM(sSmallCodeFont), TRUE);
	SendMessage(eSystemDebugTextHandle, EM_SETBKGNDCOLOR, 0, RGB(15, 15, 20));
	myCharFormat.crTextColor = RGB(13, 152, 186);
	SendMessage(eSystemDebugTextHandle, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&myCharFormat);

	/// colored status display
	eColoredStatusEdit = CreateWindowEx(NULL, "STATIC", "Passively Outuputing Default Waveform", WS_CHILD | WS_VISIBLE | SS_CENTER,
		960, 910, 960, 100, thisWindowHandle, (HMENU)IDC_GUI_STAT_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eColoredStatusEdit, WM_SETFONT, WPARAM(sLargeHeadingFont), TRUE);




	return 0;
}