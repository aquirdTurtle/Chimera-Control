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

	/// PROFILE DATA
	// Experiment Type
	eExperimentTypeLabelHandle2 = CreateWindowEx(NULL, "STATIC", "EXPERIMENT", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		960, 0, 480, 20, eMainWindowHandle, (HMENU)IDC_EXPERIMENT_TYPE_LABEL, GetModuleHandle(NULL), NULL);
	SendMessage(eExperimentTypeLabelHandle2, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	// Experiment Saved Indicator
	eExperimentSavedIndicatorHandle = CreateWindowEx(NULL, "BUTTON", "Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT,
		1340, 1, 100, 18, eMainWindowHandle, (HMENU)IDC_SAVE_EXPERIMENT_INDICATOR_BUTTON, GetModuleHandle(NULL), NULL);
	SendMessage(eExperimentSavedIndicatorHandle, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	SendMessage(eExperimentSavedIndicatorHandle, BM_SETCHECK, BST_CHECKED, NULL);
	eExperimentSaved = true;
	// Configuration Title
	eConfigurationComboLabelHandle = CreateWindowEx(NULL, "STATIC", "CATEGORY", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		1440, 0, 480, 20, eMainWindowHandle, (HMENU)IDC_CONFIGURATION_COMBO_LABEL, GetModuleHandle(NULL), NULL);
	SendMessage(eConfigurationComboLabelHandle, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	// Experiment Combo
	eExperimentTypeCombo = CreateWindowEx(NULL, TEXT("ComboBox"), "", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		960, 20, 480, 900, eMainWindowHandle, (HMENU)IDC_EXPERIMENT_COMBO, GetModuleHandle(NULL), NULL);
	SendMessage(eExperimentTypeCombo, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	fileManage::reloadCombo(eExperimentTypeCombo, EXPERIMENT_CONFIGURATION_FILES_FOLDER_PATH, "*", "__NONE__");
	// Category Combo
	eCategoryCombo = CreateWindowEx(NULL, TEXT("ComboBox"), "", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		1440, 20, 480, 900, eMainWindowHandle, (HMENU)IDC_CATEGORY_COMBO, GetModuleHandle(NULL), NULL);
	SendMessage(eCategoryCombo, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	// Orientation Title
	eOrientationLabelHandle = CreateWindowEx(NULL, "STATIC", "ORIENTATION", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		960, 50, 120, 20, eMainWindowHandle, (HMENU)IDC_ORIENTATION_COMBO_LABEL, GetModuleHandle(NULL), NULL);
	SendMessage(eOrientationLabelHandle, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	// Configuration Title
	eSubConfigComboLabelHandle = CreateWindowEx(NULL, "STATIC", "CONFIGURATION", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		1080, 50, 840, 20, eMainWindowHandle, (HMENU)IDC_SUB_CONFIG_COMBO_LABEL, GetModuleHandle(NULL), NULL);
	SendMessage(eSubConfigComboLabelHandle, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	// Configuration Saved Indicator
	eConfigurationSavedIndicatorHandle = CreateWindowEx(NULL, "BUTTON", "Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT,
		1820, 51, 100, 18, eMainWindowHandle, (HMENU)IDC_SAVE_CONFIGURATION_INDICATOR_BUTTON, GetModuleHandle(NULL), NULL);
	SendMessage(eConfigurationSavedIndicatorHandle, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	SendMessage(eConfigurationSavedIndicatorHandle, BM_SETCHECK, BST_CHECKED, NULL);
	eConfigurationSaved = true;
	// orientation combo
	std::vector<std::string> orientationNames;
	orientationNames.push_back("Horizontal");
	orientationNames.push_back("Vertical");
	eOrientationCombo = CreateWindowEx(NULL, TEXT("ComboBox"), "", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		960, 70, 120, 900, eMainWindowHandle, (HMENU)IDC_ORIENTATION_COMBO, GetModuleHandle(NULL), NULL);
	SendMessage(eOrientationCombo, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	for (int comboInc = 0; comboInc < orientationNames.size(); comboInc++)
	{
		SendMessage(eOrientationCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(_T(orientationNames[comboInc].c_str())));
	}
	SendMessage(eOrientationCombo, CB_SETCURSEL, 0, 0);
	// configuration combo
	eConfigurationCombo = CreateWindowEx(NULL, TEXT("ComboBox"), "", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		1080, 70, 840, 900, eMainWindowHandle, (HMENU)IDC_CONFIGURATION_COMBO, GetModuleHandle(NULL), NULL);
	SendMessage(eConfigurationCombo, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	/// SEQUENCE
	// Control eSequenceText, eSequenceCombo, eSequenceDisplay;
	eSequenceText.normalPos = { 960, 100, 1440, 120 };
	currentRect = eSequenceText.normalPos;
	eSequenceText.hwnd = CreateWindowEx(NULL, "STATIC", "SEQUENCE", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top,
		eMainWindowHandle, (HMENU)IDC_SEQUENCE_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eSequenceText.hwnd, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	// combo
	eSequenceCombo.normalPos = { 960, 120, 1440, 1000 };
	currentRect = eSequenceCombo.normalPos;
	eSequenceCombo.hwnd = CreateWindowEx(NULL, TEXT("ComboBox"), "", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top,
		eMainWindowHandle, (HMENU)IDC_SEQUENCE_COMBO, GetModuleHandle(NULL), NULL);
	SendMessage(eSequenceCombo.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	SendMessage(eSequenceCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"NO SEQUENCE");
	SendMessage(eSequenceCombo.hwnd, CB_SETCURSEL, 0, 0);
	// display
	eSequenceDisplay.normalPos = { 960, 150, 1440, 250 };
	currentRect = eSequenceDisplay.normalPos;
	eSequenceDisplay.hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "Sequence of Configurations to Run:\r\n", ES_READONLY | WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL,
		currentRect.left, currentRect.top, currentRect.right - currentRect.left, currentRect.bottom - currentRect.top,
		eMainWindowHandle, (HMENU)IDC_SEQUENCE_DISPLAY, GetModuleHandle(NULL), NULL);

	/// NOTES
	// Configuration Notes Title
	eNotesLabelTextHandle = CreateWindowEx(NULL, "STATIC", "NOTES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		960, 250, 480, 20, eMainWindowHandle, (HMENU)IDC_NOTES_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eNotesLabelTextHandle, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	//  Configuration Notes edit
	eExperimentConfigurationNotesEditHandle = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL | ES_AUTOVSCROLL,
		960, 270, 480, 480, eMainWindowHandle, (HMENU)IDC_CONFIG_NOTES, GetModuleHandle(NULL), NULL);
	SendMessage(eExperimentConfigurationNotesEditHandle, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	/// VARIABLES
	// Variables Title
	eVariablesLabelTextHandle = CreateWindowEx(NULL, "STATIC", "VARIABLES", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		1440, 100, 480, 20, eMainWindowHandle, (HMENU)IDC_STATIC_2_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eVariablesLabelTextHandle, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	// Set Variable Names Button
	eVariableSetButtonHandle = CreateWindowEx(NULL, "BUTTON", "Set Variable Names", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		1440, 125, 480, 24, eMainWindowHandle, (HMENU)IDC_VAR_SET_BUTTON, GetModuleHandle(NULL), NULL);
	SendMessage(eVariableSetButtonHandle, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	//  Variable Names edit Handle
	eVariableNamesEditHandle = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL,
		1440, 150, 480, 50, eMainWindowHandle, (HMENU)IDC_VAR_NAMES_EDIT, GetModuleHandle(NULL), NULL);
	SendMessage(eVariableNamesEditHandle, WM_SETFONT, WPARAM(sCodeFont), TRUE);
	// Current Variable Name Display
	eVar1NameTextHandle = CreateWindowEx(NULL, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_WORDELLIPSIS | WS_BORDER,
		1440, 205, 160, 25, eMainWindowHandle, (HMENU)IDC_VAR_NAME_1_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eVar1NameTextHandle, WM_SETFONT, WPARAM(sCodeFont), TRUE);
	eVar2NameTextHandle = CreateWindowEx(NULL, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_WORDELLIPSIS | WS_BORDER,
		1600, 205, 160, 25, eMainWindowHandle, (HMENU)IDC_VAR_NAME_2_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eVar2NameTextHandle, WM_SETFONT, WPARAM(sCodeFont), TRUE);
	eVar3NameTextHandle = CreateWindowEx(NULL, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_WORDELLIPSIS | WS_BORDER,
		1760, 205, 160, 25, eMainWindowHandle, (HMENU)IDC_VAR_NAME_3_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eVar3NameTextHandle, WM_SETFONT, WPARAM(sCodeFont), TRUE);
	eVar4NameTextHandle = CreateWindowEx(NULL, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_WORDELLIPSIS | WS_BORDER,
		1440, 235, 160, 25, eMainWindowHandle, (HMENU)IDC_VAR_NAME_4_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eVar4NameTextHandle, WM_SETFONT, WPARAM(sCodeFont), TRUE);
	eVar5NameTextHandle = CreateWindowEx(NULL, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_WORDELLIPSIS | WS_BORDER,
		1600, 235, 160, 25, eMainWindowHandle, (HMENU)IDC_VAR_NAME_5_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eVar5NameTextHandle, WM_SETFONT, WPARAM(sCodeFont), TRUE);
	eVar6NameTextHandle = CreateWindowEx(NULL, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_WORDELLIPSIS | WS_BORDER,
		1760, 235, 160, 25, eMainWindowHandle, (HMENU)IDC_VAR_NAME_6_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eVar6NameTextHandle, WM_SETFONT, WPARAM(sCodeFont), TRUE);
	// Dummy Variable Usage Box
	eDummyVariableSelectHandle = CreateWindowEx(NULL, "BUTTON", "Use Dummy Variables?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT,
		1440, 265, 480, 20, eMainWindowHandle, (HMENU)IDC_DUMMY_BUTTON, GetModuleHandle(NULL), NULL);
	SendMessage(eDummyVariableSelectHandle, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	SendMessage(eDummyVariableSelectHandle, BM_SETCHECK, BST_UNCHECKED, NULL);
	eUseDummyVariables = false;
	eDummyNum = 0;
	// Dummy Num Display
	eDummyNumTextHandle = CreateWindowEx(NULL, "STATIC", "0", WS_CHILD | WS_VISIBLE | SS_WORDELLIPSIS,
		1440, 290, 85, 25, eMainWindowHandle, (HMENU)IDC_DUMMY_NUM_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eDummyNumTextHandle, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	// Dummy Variable Number Edit
	eDummyNumEditHandle = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "0", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
		1525, 290, 85, 25, eMainWindowHandle, (HMENU)IDC_DUMMY_NUM_EDIT, GetModuleHandle(NULL), NULL);
	SendMessage(eDummyNumEditHandle, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	// Set Dummy Variable Numbers
	eDummyVariableButtonHandle = CreateWindowEx(NULL, "BUTTON", "Set Dummy Variable Number", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		1615, 290, 305, 25, eMainWindowHandle, (HMENU)IDC_DUMMY_NUM_BUTTON, GetModuleHandle(NULL), NULL);
	SendMessage(eDummyVariableButtonHandle, WM_SETFONT, WPARAM(sNormalFont), TRUE);

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

	/// colored status display
	eColoredStatusEdit = CreateWindowEx(NULL, "STATIC", "Passively Outuputing Default Waveform", WS_CHILD | WS_VISIBLE | SS_CENTER,
		960, 910, 960, 100, eMainWindowHandle, (HMENU)IDC_GUI_STAT_TEXT, GetModuleHandle(NULL), NULL);
	SendMessage(eColoredStatusEdit, WM_SETFONT, WPARAM(sLargeHeadingFont), TRUE);
	return 0;
}