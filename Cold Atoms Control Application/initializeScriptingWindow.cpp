#include "stdafx.h"
#include "initializeScriptingWindow.h"
#include "fonts.h"
#include "constants.h"
#include "externals.h"

/*
 * This function is called on WM_CREATE in order to initialize all of the handles to objects on the scripting window.
 */
int initializeScriptingWindow(HWND eScriptingWindowHandle)
{
	//LoadLibrary(TEXT("Msftedit.dll"));

	CHARFORMAT myCharFormat;
	memset(&myCharFormat, 0, sizeof(CHARFORMAT));
	myCharFormat.cbSize = sizeof(CHARFORMAT);
	myCharFormat.dwMask = CFM_COLOR;
	myCharFormat.crTextColor = RGB(255, 255, 255);

	RECT currentPos;

	/// CONFIGURATION DISPLAY
	eConfigurationTextInScripting = CreateWindowEx(NULL, "STATIC", "CONFIGURATION:", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
												0, 3, 200, 20, eScriptingWindowHandle, (HMENU)IDC_CONFIG_LABEL_HANDLE_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eConfigurationTextInScripting, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	
	eConfigurationDisplayInScripting = CreateWindowEx(NULL, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_ENDELLIPSIS,
											   200, 3, 1500, 20, eScriptingWindowHandle, (HMENU)IDC_CONFIG_NAME_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eConfigurationDisplayInScripting, WM_SETFONT, WPARAM(sNormalFont), TRUE);

	/// STATUS INDICATOR COLORBOX
	eColorBox = CreateWindowEx(NULL, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_WORDELLIPSIS | SS_CENTER,
							   1880, 3, 40, 20, eScriptingWindowHandle, (HMENU)IDC_COLOR_BOX, GetModuleHandle(NULL), NULL);

	return 0;
}
