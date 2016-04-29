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
	LoadLibrary(TEXT("Msftedit.dll"));

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

	/// VERTICAL SCRIPT
	// title
	eStaticVerticalEditHandle = CreateWindowEx(NULL, "STATIC", "VERTICAL AOM SCRIPT", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		0, 28, 640, 20, eScriptingWindowHandle, (HMENU)IDC_STATIC_X_HANDLE_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eStaticVerticalEditHandle, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	// Name
	eVerticalScriptNameTextHandle = CreateWindowEx(NULL, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_ENDELLIPSIS,
		0, 53, 520, 20, eScriptingWindowHandle, (HMENU)IDC_X_SCRIPT_NAME_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eVerticalScriptNameTextHandle, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	// Saved Indicator
	eVerticalScriptSavedIndicatorHandle = CreateWindowEx(NULL, "BUTTON", "Script Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT,
		520, 53, 120, 20, eScriptingWindowHandle, (HMENU)IDC_SAVE_X_INDICATOR_BUTTON, GetModuleHandle(NULL), NULL);
	SendMessage(eVerticalScriptSavedIndicatorHandle, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	SendMessage(eVerticalScriptSavedIndicatorHandle, BM_SETCHECK, BST_CHECKED, NULL);
	eVerticalScriptSaved = true;
	// View Text
	eVerticalViewText.normalPos = { 0, 73, 50, 98 };
	currentPos = eVerticalViewText.normalPos;
	eVerticalViewText.hwnd = CreateWindowEx(NULL, "STATIC", "View:", WS_CHILD | WS_VISIBLE | SS_ENDELLIPSIS,
											currentPos.left, currentPos.top, currentPos.right - currentPos.left, currentPos.bottom - currentPos.top, 
											eScriptingWindowHandle, (HMENU)IDC_VERTICAL_SCRIPT_VIEW_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eVerticalViewText.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);

	// View Combo

	eVerticalViewCombo.normalPos = { 50, 73, 640, 1000 };
	currentPos = eVerticalViewCombo.normalPos;
	eVerticalViewCombo.hwnd = CreateWindowEx(NULL, TEXT("ComboBox"), "", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		currentPos.left, currentPos.top, currentPos.right - currentPos.left, currentPos.bottom - currentPos.top, eScriptingWindowHandle,
		(HMENU)IDC_VERTICAL_SCRIPT_VIEW_COMBO, GetModuleHandle(NULL), NULL);
	SendMessage(eVerticalViewCombo.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	SendMessage(eVerticalViewCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)("Parent Script"));
	SendMessage(eVerticalViewCombo.hwnd, CB_SETCURSEL, 0, 0);
	// Edit
	eVerticalScriptEditHandle = CreateWindowEx(WS_EX_CLIENTEDGE, _T("RICHEDIT50W"), "",
		WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | WS_HSCROLL,
		0, 100, 640, 920, eScriptingWindowHandle, (HMENU)IDC_VERTICAL_SCRIPT_EDIT, GetModuleHandle(NULL), NULL);
	SendMessage(eVerticalScriptEditHandle, WM_SETFONT, WPARAM(sCodeFont), TRUE);
	SendMessage(eVerticalScriptEditHandle, EM_SETBKGNDCOLOR, 0, RGB(30, 25, 25));
	SendMessage(eVerticalScriptEditHandle, EM_SETEVENTMASK, 0, ENM_CHANGE);
	SendMessage(eVerticalScriptEditHandle, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&myCharFormat);
	/// HORIZONTAL SCRIPT
	// title
	eStaticHorizontalEditHandle = CreateWindowEx(NULL, "STATIC", "HORIZONTAL AOM SCRIPT", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		640, 28, 640, 20, eScriptingWindowHandle, (HMENU)IDC_STATIC_Y_HANDLE_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eStaticHorizontalEditHandle, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	// name
	eHorizontalScriptNameTextHandle = CreateWindowEx(NULL, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_ENDELLIPSIS,
		640, 53, 520, 20, eScriptingWindowHandle, (HMENU)IDC_Y_SCRIPT_NAME_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eHorizontalScriptNameTextHandle, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	// Saved Indicator
	eHorizontalScriptSavedIndicatorHandle = CreateWindowEx(NULL, "BUTTON", "Script Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT,
		1160, 53, 120, 20, eScriptingWindowHandle, (HMENU)IDC_SAVE_Y_INDICATOR_BUTTON, GetModuleHandle(NULL), NULL);
	SendMessage(eHorizontalScriptSavedIndicatorHandle, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	SendMessage(eHorizontalScriptSavedIndicatorHandle, BM_SETCHECK, BST_CHECKED, NULL);
	eHorizontalScriptSaved = true;
	// View Text
	eHorizontalViewText.normalPos = { 640, 73, 690, 98 };
	currentPos = eHorizontalViewText.normalPos;
	eHorizontalViewText.hwnd = CreateWindowEx(NULL, "STATIC", "View:", WS_CHILD | WS_VISIBLE | SS_ENDELLIPSIS,
		currentPos.left, currentPos.top, currentPos.right - currentPos.left, currentPos.bottom - currentPos.top,
		eScriptingWindowHandle, (HMENU)IDC_HORIZONTAL_SCRIPT_VIEW_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eHorizontalViewText.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);

	// View Combo

	eHorizontalViewCombo.normalPos = { 690, 73, 1280, 1000 };
	currentPos = eHorizontalViewCombo.normalPos;
	eHorizontalViewCombo.hwnd = CreateWindowEx(NULL, TEXT("ComboBox"), "", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		currentPos.left, currentPos.top, currentPos.right - currentPos.left, currentPos.bottom - currentPos.top, eScriptingWindowHandle,
		(HMENU)IDC_HORIZONTAL_SCRIPT_VIEW_COMBO, GetModuleHandle(NULL), NULL);
	SendMessage(eHorizontalViewCombo.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	SendMessage(eHorizontalViewCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)("Parent Script"));
	SendMessage(eHorizontalViewCombo.hwnd, CB_SETCURSEL, 0, 0);

	// edit
	eHorizontalScriptEditHandle = CreateWindowEx(WS_EX_CLIENTEDGE, _T("RICHEDIT50W"), "",
												 WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | WS_HSCROLL,
												 640, 100, 640, 920, eScriptingWindowHandle, (HMENU)IDC_HORIZONTAL_SCRIPT_EDIT, GetModuleHandle(NULL), NULL);
	SendMessage(eHorizontalScriptEditHandle, WM_SETFONT, WPARAM(sCodeFont), TRUE);
	SendMessage(eHorizontalScriptEditHandle, EM_SETBKGNDCOLOR, 0, RGB(30, 25, 25));
	SendMessage(eHorizontalScriptEditHandle, EM_SETEVENTMASK, 0, ENM_CHANGE);
	SendMessage(eHorizontalScriptEditHandle, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&myCharFormat);
	/// INTENSITY SCRIPT
	// title
	eStaticIntensityTitleHandle = CreateWindowEx(NULL, "STATIC", "INTENSITY SCRIPT", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		1280, 28, 640, 20, eScriptingWindowHandle, (HMENU)IDC_STATIC_INTENSITY_TITLE_HANDLE, GetModuleHandle(NULL), NULL);
	SendMessage(eStaticIntensityTitleHandle, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	// name
	eIntensityNameHandle = CreateWindowEx(NULL, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_ENDELLIPSIS,
		1280, 53, 520, 20, eScriptingWindowHandle, (HMENU)IDC_INTENSITY_NAME_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eIntensityNameHandle, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	// saved indicator
	eIntensityScriptSavedIndicatorHandle = CreateWindowEx(NULL, "BUTTON", "Script Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT,
		1800, 53, 120, 20, eScriptingWindowHandle, (HMENU)IDC_INTENSITY_SAVED_BUTTON, GetModuleHandle(NULL), NULL);
	SendMessage(eIntensityScriptSavedIndicatorHandle, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	SendMessage(eIntensityScriptSavedIndicatorHandle, BM_SETCHECK, BST_CHECKED, NULL);
	eIntensityScriptSaved = true;
	// View Text
	eIntensityViewText.normalPos = { 1280, 73, 1330, 98 };
	currentPos = eIntensityViewText.normalPos;
	eIntensityViewText.hwnd = CreateWindowEx(NULL, "STATIC", "View:", WS_CHILD | WS_VISIBLE | SS_ENDELLIPSIS,
		currentPos.left, currentPos.top, currentPos.right - currentPos.left, currentPos.bottom - currentPos.top,
		eScriptingWindowHandle, (HMENU)IDC_INTENSITY_SCRIPT_VIEW_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eIntensityViewText.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);

	// View Combo

	eIntensityViewCombo.normalPos = { 1330, 73, 1920, 1000 };
	currentPos = eIntensityViewCombo.normalPos;
	eIntensityViewCombo.hwnd = CreateWindowEx(NULL, TEXT("ComboBox"), "", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		currentPos.left, currentPos.top, currentPos.right - currentPos.left, currentPos.bottom - currentPos.top, eScriptingWindowHandle,
		(HMENU)IDC_INTENSITY_SCRIPT_VIEW_COMBO, GetModuleHandle(NULL), NULL);
	SendMessage(eIntensityViewCombo.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	SendMessage(eIntensityViewCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Parent Script");
	SendMessage(eIntensityViewCombo.hwnd, CB_SETCURSEL, 0, 0);

	// edit
	eIntensityScriptEditHandle = CreateWindowEx(WS_EX_CLIENTEDGE, _T("RICHEDIT50W"), "",
		WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | WS_HSCROLL,
		1280, 100, 640, 920, eScriptingWindowHandle, (HMENU)IDC_INTENSITY_SCRIPT_EDIT, GetModuleHandle(NULL), NULL);
	SendMessage(eIntensityScriptEditHandle, WM_SETFONT, WPARAM(sCodeFont), TRUE);
	SendMessage(eIntensityScriptEditHandle, EM_SETBKGNDCOLOR, 0, RGB(40, 35, 35));
	SendMessage(eIntensityScriptEditHandle, EM_SETEVENTMASK, 0, ENM_CHANGE);
	SendMessage(eIntensityScriptEditHandle, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&myCharFormat);

	return 0;
}
