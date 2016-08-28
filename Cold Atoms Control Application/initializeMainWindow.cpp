#include "stdafx.h"
#include "initializeMainWindow.h"
#include "Windows.h"
#include "externals.h"
#include "constants.h"
#include "fonts.h"
#include "fileManage.h"

/*
 * This function is called on WM_CREATE in order to initialize the handles to all of the objects on the main window.
 */
int initializeMainWindow(HWND eMainWindowHandle)
{
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
		0, 0, 380, 20, eMainWindowHandle, (HMENU)IDC_STATIC_4_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eExperimentStatusTextHandle, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	// clear button
	eScriptStatusClearButtonHandle = CreateWindowEx(NULL, "BUTTON", "clear", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		380, 0, 100, 20, eMainWindowHandle, (HMENU)IDC_SCRIPT_STATUS_CLEAR_BUTTON, GetModuleHandle(NULL), NULL);
	SendMessage(eScriptStatusClearButtonHandle, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	// System Status
	eSystemStatusTextHandle = CreateWindowEx(NULL, _T("RICHEDIT50W"), "",
		WS_CHILD | WS_VISIBLE | ES_READONLY | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER,
		0, 20, 480, 975, eMainWindowHandle, (HMENU)IDC_SYSTEM_STATUS_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eSystemStatusTextHandle, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	SendMessage(eSystemStatusTextHandle, EM_SETBKGNDCOLOR, 0, RGB(15, 15, 20));
	myCharFormat.crTextColor = RGB(50, 50, 250);
	SendMessage(eSystemStatusTextHandle, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&myCharFormat);

	///	ERROR STATUS
	// Title
	eErrorStatusTextDisplay = CreateWindowEx(NULL, "STATIC", "Error Status", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		480, 0, 380, 20, eMainWindowHandle, (HMENU)IDC_ERROR_STATUS_TEXT_DISPLAY, GetModuleHandle(NULL), NULL);
	SendMessage(eErrorStatusTextDisplay, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	// Error Clear Button
	eScriptErrorClearButtonHandle = CreateWindowEx(NULL, "BUTTON", "clear", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		860, 0, 100, 20, eMainWindowHandle, (HMENU)IDC_SCRIPT_ERROR_CLEAR_BUTTON, GetModuleHandle(NULL), NULL);
	SendMessage(eScriptErrorClearButtonHandle, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	// error edit
	eSystemErrorTextHandle = CreateWindowEx(NULL, _T("RICHEDIT50W"), "",
		WS_CHILD | WS_VISIBLE | ES_READONLY | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER,
		480, 20, 480, 480, eMainWindowHandle, (HMENU)IDC_SYSTEM_ERROR_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eSystemErrorTextHandle, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	SendMessage(eSystemErrorTextHandle, EM_SETBKGNDCOLOR, 0, RGB(15, 15, 20));
	myCharFormat.crTextColor = RGB(200, 0, 0);
	SendMessage(eSystemErrorTextHandle, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&myCharFormat);

	/// DEBUG STATUS
	//  Debug Status
	eDebugStatusTextHandle = CreateWindowEx(NULL, "STATIC", "Debug Status", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		480, 500, 380, 20, eMainWindowHandle, (HMENU)IDC_STATIC_5_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eDebugStatusTextHandle, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	// Debug Clear Button
	eScriptDebugClearButtonHandle = CreateWindowEx(NULL, "BUTTON", "clear", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		860, 500, 100, 20, eMainWindowHandle, (HMENU)IDC_SCRIPT_DEBUG_CLEAR_BUTTON, GetModuleHandle(NULL), NULL);
	SendMessage(eScriptDebugClearButtonHandle, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	// Debug edit
	eSystemDebugTextHandle = CreateWindowEx(NULL, _T("RICHEDIT50W"), "",
		WS_CHILD | WS_VISIBLE | ES_READONLY | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER,
		480, 520, 480, 480, eMainWindowHandle, (HMENU)IDC_SYSTEM_DEBUG_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eSystemDebugTextHandle, WM_SETFONT, WPARAM(sCodeFont), TRUE);
	SendMessage(eSystemDebugTextHandle, EM_SETBKGNDCOLOR, 0, RGB(15, 15, 20));
	myCharFormat.crTextColor = RGB(13, 152, 186);
	SendMessage(eSystemDebugTextHandle, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&myCharFormat);


	/// NOTES
	// TODO
	POINT notesStart = { 960, 250 };
	eNotes.initializeControls(notesStart, eMainWindowHandle);
	/// VARIABLES
	POINT controlLocation = { 1440, 95 };
	eVariables.initializeControls(controlLocation, eMainWindowHandle);
	/// SETUP / EXPERIMENTAL PARAMETERS
	// Setup / Experimental Parameters Title
	eSetupParametersTextHandle = CreateWindowEx(NULL, "STATIC", "SETUP / EXPERIMENTAL PARAMETERS", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		1440, 320, 480, 20, eMainWindowHandle, (HMENU)IDC_STATIC_3_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eSetupParametersTextHandle, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	// Connect to Master Option
	eConnectToMasterHandle = CreateWindowEx(NULL, "BUTTON", "Connect to Master Computer?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT,
		1440, 345, 480, 20, eMainWindowHandle, (HMENU)IDC_CONNECT_TO_MASTER_BUTTON, GetModuleHandle(NULL), NULL);
	SendMessage(eConnectToMasterHandle, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	CheckDlgButton(eMainWindowHandle, IDC_CONNECT_TO_MASTER_BUTTON, BST_CHECKED);
	eConnectToMaster = true;
	// Recieve Variable Files Option
	eReceiveVariableFiles = CreateWindowEx(NULL, "BUTTON", "Get Variable Files From Master Computer?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT,
		1440, 370, 480, 20, eMainWindowHandle, (HMENU)IDC_RECEIVE_VAR_FILES_BUTTON, GetModuleHandle(NULL), NULL);
	SendMessage(eReceiveVariableFiles, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	CheckDlgButton(eMainWindowHandle, IDC_RECEIVE_VAR_FILES_BUTTON, BST_CHECKED);
	eGetVarFilesFromMaster = true;
	// Loc Script And Parameters Option
	eLogScriptAndParamsButton = CreateWindowEx(NULL, "BUTTON", "Log Current Script and Experiment Parameters?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT,
		1440, 395, 480, 20, eMainWindowHandle, (HMENU)IDC_LOG_SCRIPT_PARAMS, GetModuleHandle(NULL), NULL);
	SendMessage(eLogScriptAndParamsButton, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	CheckDlgButton(eMainWindowHandle, IDC_LOG_SCRIPT_PARAMS, BST_CHECKED);
	eLogScriptAndParams = true;
	// Program intensity option
	eProgramIntensityOptionButton = CreateWindowEx(NULL, "BUTTON", "Program Intensity?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT,
		1440, 420, 480, 20, eMainWindowHandle, (HMENU)IDC_PROGRAM_INTENSITY_BOX, GetModuleHandle(NULL), NULL);
	SendMessage(eProgramIntensityOptionButton, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	CheckDlgButton(eMainWindowHandle, IDC_LOG_SCRIPT_PARAMS, BST_CHECKED);
	eProgramIntensityOption = true;
	/// DEBUGGING OPTIONS
	// Debugging Options Title
	eDebuggingOptionsDisplayHandle = CreateWindowEx(NULL, "STATIC", "DEBUGGING OPTIONS", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		1440, 445, 480, 20, eMainWindowHandle, (HMENU)IDC_DEBUG_OPTION_DISPLAY_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eDebuggingOptionsDisplayHandle, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	// Output Read Status Option
	eOutputReadStatusButton = CreateWindowEx(NULL, "BUTTON", "Output Waveform Read Progress?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT,
											1440, 495, 480, 20, eMainWindowHandle, (HMENU)IDC_OUTPUT_READ_STATUS, GetModuleHandle(NULL), NULL);
	SendMessage(eOutputReadStatusButton, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	CheckDlgButton(eMainWindowHandle, IDC_OUTPUT_READ_WRITE_STATUS, BST_UNCHECKED);
	eOutputReadStatus = false;
	// Output Write Status Option
	eOutputWriteStatusButton = CreateWindowEx(NULL, "BUTTON", "Output Waveform Write Progress?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT,
											 1440, 520, 480, 20, eMainWindowHandle, (HMENU)IDC_OUTPUT_WRITE_STATUS, GetModuleHandle(NULL), NULL);
	SendMessage(eOutputWriteStatusButton, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	CheckDlgButton(eMainWindowHandle, IDC_OUTPUT_READ_WRITE_STATUS, BST_UNCHECKED);
	eOutputWriteStatus = false;
	// Output Correction Waveform Times Option
	eOutputCorrTimeButton = CreateWindowEx(NULL, "BUTTON", "Output Correction Waveform Times?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT,
										  1440, 470, 480, 20, eMainWindowHandle, (HMENU)IDC_OUTPUT_CORR_TIME_BUTTON, GetModuleHandle(NULL), NULL);
	SendMessage(eOutputCorrTimeButton, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	CheckDlgButton(eMainWindowHandle, IDC_OUTPUT_CORR_TIME_BUTTON, BST_UNCHECKED);
	eOutputCorrTime = false;

	eOutputMoreInfoCheckButton = CreateWindowEx(NULL, "BUTTON", "Output More Run Info?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT,
												1440, 545, 480, 20, eMainWindowHandle, (HMENU)IDC_OUTPUT_MORE_RUN_INFO, GetModuleHandle(NULL), NULL);
	SendMessage(eOutputMoreInfoCheckButton, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	CheckDlgButton(eMainWindowHandle, IDC_OUTPUT_MORE_RUN_INFO, BST_UNCHECKED);
	eOutputRunInfo = false;

	eDebugger.initialize({ 1440, 565 }, eMainWindowHandle);

	/// colored status display
	eColoredStatusEdit = CreateWindowEx(NULL, "STATIC", "Passively Outuputing Default Waveform", WS_CHILD | WS_VISIBLE | SS_CENTER,
		960, 910, 960, 100, eMainWindowHandle, (HMENU)IDC_GUI_STAT_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eColoredStatusEdit, WM_SETFONT, WPARAM(sLargeHeadingFont), TRUE);


	/// PROFILE DATA
	POINT configStart = { 960, 0 };
	eProfile.initializeControls(configStart, eMainWindowHandle);

	return 0;
}