#include "stdafx.h"

#include "winProcScripts.h"

#include "constants.h"
#include "externals.h"

#include "fonts.h"
#include "getFileName.h"
#include "saveTextFileFromEdit.h"
#include "scriptWriteHelpProc.h"
#include "appendText.h"
#include "cleanString.h"
#include "fileManage.h"
#include "initializeScriptingWindow.h"

#include <sstream>
#include <fstream>

#include "boost/lexical_cast.hpp"
#include <boost/algorithm/string.hpp>
#include "menuAndAcceleratorFunctions.h"
#include "resource.h"

#include "Windows.h"

#include "experimentThreadInputStructure.h"
#include "experimentProgrammingThread.h"
#include "myAgilent.h"

#include "myErrorHandler.h"
#include "myNIAWG.h"

#include "colorScript.h"

/**
 * This functin handles the creation and messages for the scripting window. Inputs are message information.
 */
LRESULT CALLBACK winProcScripts(HWND thisWindow, UINT msg, WPARAM wParam, LPARAM lParam)
{
	/// Handle incoming messages.
	switch (msg)
	{
		case WM_CREATE: 
		{
			initializeScriptingWindow(thisWindow);
			break;
		}
		/// Hanlde Colors
		case WM_CTLCOLOREDIT:
		{
			DWORD CtrlID = GetDlgCtrlID((HWND)lParam); // Window Control ID
			HDC hdcStatic = (HDC)wParam;
			if (CtrlID == IDC_HORIZONTAL_SCRIPT_EDIT || CtrlID == IDC_VERTICAL_SCRIPT_EDIT || CtrlID == IDC_VAR_NAMES_EDIT)
			{
				SetTextColor(hdcStatic, RGB(255, 255, 255));
				SetBkColor(hdcStatic, RGB(50, 45, 45));
				return (INT_PTR)eGreyRedBrush;
			}
			if (CtrlID == IDC_INTENSITY_SCRIPT_EDIT)
			{
				SetTextColor(hdcStatic, RGB(255, 255, 255));
				SetBkColor(hdcStatic, RGB(30, 25, 25));
				return (INT_PTR)eNearBlackRedBrush;
			}
		}

		case WM_CTLCOLORSTATIC:
		{
			DWORD CtrlID = GetDlgCtrlID((HWND)lParam); // Window Control ID
			HDC hdcStatic = (HDC)wParam;

			if (CtrlID == IDC_SAVE_X_INDICATOR_BUTTON || CtrlID == IDC_SAVE_Y_INDICATOR_BUTTON || CtrlID == IDC_SAVE_EXPERIMENT_INDICATOR_BUTTON 
				|| CtrlID == IDC_INTENSITY_SAVED_BUTTON)
			{
				SetTextColor(hdcStatic, RGB(255, 255, 255));
				SetBkMode(hdcStatic, TRANSPARENT);
				SetBkColor(hdcStatic, RGB(50, 45, 45));
				return (INT_PTR)eGreyRedBrush;
			}
			else if (CtrlID == IDC_STATIC_X_HANDLE_TEXT || CtrlID == IDC_STATIC_Y_HANDLE_TEXT || CtrlID == IDC_CONFIG_LABEL_HANDLE_TEXT 
					 || CtrlID == IDC_STATIC_INTENSITY_TITLE_HANDLE)
			{
				SetTextColor(hdcStatic, RGB(255, 255, 255));
				SetBkMode(hdcStatic, TRANSPARENT);
				SetBkColor(hdcStatic, RGB(75, 0, 0));
				return (INT_PTR)eDarkRedBrush;
			}
			else if (CtrlID == IDC_X_SCRIPT_NAME_TEXT || CtrlID == IDC_Y_SCRIPT_NAME_TEXT || CtrlID == IDC_CONFIG_NAME_TEXT 
					 || CtrlID == IDC_INTENSITY_NAME_TEXT)
			{
				SetTextColor(hdcStatic, RGB(255, 255, 255));
				SetBkMode(hdcStatic, TRANSPARENT);
				SetBkColor(hdcStatic, RGB(25, 0, 0));
				return (INT_PTR)eDullRedBrush;
			}
			else if (CtrlID == IDC_COLOR_BOX)
			{

				if (eGenStatusColor == "G")
				{
					// Color Green. This is the "Ready to give next waveform" color. During this color you can also press esc to exit.
					SetTextColor(hdcStatic, RGB(255, 255, 255));
					SetBkColor(hdcStatic, RGB(0, 120, 0));
					return (INT_PTR)eGreenBrush;
				}
				else if (eGenStatusColor == "Y")
				{
					// Color Yellow. This is the "Working" Color.
					SetTextColor(hdcStatic, RGB(255, 255, 255));
					SetBkColor(hdcStatic, RGB(104, 104, 0));
					return (INT_PTR)eYellowBrush;
				}
				else if (eGenStatusColor == "R")
				{
					// Color Red. This is a big visual signifier for when the program exited with error.
					SetTextColor(hdcStatic, RGB(255, 255, 255));
					SetBkColor(hdcStatic, RGB(120, 0, 0));
					return (INT_PTR)eRedBrush;
				}
				else
				{
					// color Blue. This is the default, ready for user input color.
					SetTextColor(hdcStatic, RGB(255, 255, 255));
					SetBkColor(hdcStatic, RGB(0, 0, 120));
					return (INT_PTR)blueBrush;
				}
			}
			else
			{
				return DefWindowProc(thisWindow, msg, wParam, lParam);
			}

			break;
		}
		/// Handle Close
		case WM_CLOSE:
		case WM_DESTROY:
		{
			if (eHorizontalScriptSaved == false)
			{
				// check if the user wants to save
				int cont = fileManage::checkSaveScript("horizontal", eHorizontalScriptEditHandle, thisWindow, eHorizontalCurrentParentScriptName, eHorizontalScriptSavedIndicatorHandle, eHorizontalScriptSaved,
													   eHorizontalParentScriptPathString, eHorizontalScriptNameTextHandle);
				// this occurs if the user presses cancel. Just break, don't open.
				if (cont == 0)
				{
					break;
				}
			}
			if (eVerticalScriptSaved == false)
			{
				// check if the user wants to save
				int cont = fileManage::checkSaveScript("vertical", eVerticalScriptEditHandle, thisWindow, eVerticalCurrentParentScriptName, eVerticalScriptSavedIndicatorHandle, eVerticalScriptSaved,
													   eHorizontalParentScriptPathString, eVerticalScriptNameTextHandle);
				// this occurs if the user presses cancel. Just break, don't open.
				if (cont == 0)
				{
					break;
				}
			}
			if (eExperimentSaved == false)
			{
				// check if the user wants to save
				int cont = fileManage::checkExperimentSave(thisWindow);
				// this occurs if the user presses cancel. Just break, don't open.
				if (cont == 0)
				{
					break;
				}
			}
			std::string exitQuestion = "Are you sure you want to exit?\n\nThis will stop all output of the arbitrary waveform generator.";
			int areYouSure = MessageBox(NULL, exitQuestion.c_str(), "Exit", MB_OKCANCEL | MB_ICONWARNING);
			switch (areYouSure)
			{
				case IDOK:
					PostQuitMessage(1);
					break;
				case IDCANCEL:
					break;
				default:
					break;
			}
			break;
		}
		case WM_TIMER:
		{
			if (!eVerticalSyntaxColorIsCurrent)
			{
				// preserve saved state
				bool tempSaved = false;
				if (eVerticalScriptSaved == true)
				{
					tempSaved = true;
				}
				DWORD x1 = 1, x2 = 1;
				int initScrollPos, finScrollPos;
				SendMessage(eVerticalScriptEditHandle, EM_GETSEL, (WPARAM)&x1, (LPARAM)&x2);
				initScrollPos = GetScrollPos(eVerticalScriptEditHandle, SB_VERT);
				// color syntax
				colorScript(eVerticalScriptEditHandle, "NIAWG", eVerticalMinChange, eVerticalMaxChange, eVerticalViewCombo.hwnd, eCurrentVerticalViewIsParent);
				eVerticalMaxChange = 0;
				eVerticalMinChange = ULONG_MAX;
				eVerticalSyntaxColorIsCurrent = true;
				SendMessage(eVerticalScriptEditHandle, EM_SETSEL, (WPARAM)x1, (LPARAM)x2);
				finScrollPos = GetScrollPos(eVerticalScriptEditHandle, SB_VERT);
				SendMessage(eVerticalScriptEditHandle, EM_LINESCROLL, 0, -(finScrollPos - initScrollPos));
				if (tempSaved == true)
				{
					eVerticalScriptSaved = true;
					SendMessage(eVerticalScriptSavedIndicatorHandle, BM_SETCHECK, BST_CHECKED, NULL);
				}
			}
			if (!eHorizontalSyntaxColorIsCurrent)
			{
				// preserve saved state
				bool tempSaved = false;
				if (eHorizontalScriptSaved == true)
				{
					tempSaved = true;
				}
				DWORD x1 = 1, x2 = 1;
				int initScrollPos, finScrollPos;

				SendMessage(eHorizontalScriptEditHandle, EM_GETSEL, (WPARAM)&x1, (LPARAM)&x2);
				initScrollPos = GetScrollPos(eHorizontalScriptEditHandle, SB_VERT);

				// color syntax
 				colorScript(eHorizontalScriptEditHandle, "NIAWG", eHorizontalMinChange, eHorizontalMaxChange, eHorizontalViewCombo.hwnd, eCurrentHorizontalViewIsParent);

				eHorizontalSyntaxColorIsCurrent = true;
				eHorizontalMaxChange = 0;
				eHorizontalMinChange = ULONG_MAX;
				SendMessage(eHorizontalScriptEditHandle, EM_SETSEL, (WPARAM)x1, (LPARAM)x2);
				finScrollPos = GetScrollPos(eHorizontalScriptEditHandle, SB_VERT);
				SendMessage(eHorizontalScriptEditHandle, EM_LINESCROLL, 0, -(finScrollPos - initScrollPos));
				if (tempSaved == true)
				{
					eHorizontalScriptSaved = true;
					SendMessage(eHorizontalScriptSavedIndicatorHandle, BM_SETCHECK, BST_CHECKED, NULL);
				}
			}
			if (!eIntensitySyntaxColorIsCurrent)
			{
				// preserve saved state
				bool tempSaved = false;
				if (eIntensityScriptSaved == true)
				{
					tempSaved = true;
				}
				DWORD x1 = 1, x2 = 1;
				int initScrollPos, finScrollPos;
				SendMessage(eIntensityScriptEditHandle, EM_GETSEL, (WPARAM)&x1, (LPARAM)&x2);
				initScrollPos = GetScrollPos(eIntensityScriptEditHandle, SB_VERT);

				// color syntax
				colorScript(eIntensityScriptEditHandle, "AGILENT", eIntensityMinChange, eIntensityMaxChange, eIntensityViewCombo.hwnd, eCurrentIntensityViewIsParent);
				eIntensitySyntaxColorIsCurrent = true;
				eIntensityMaxChange = 0;
				eIntensityMinChange = ULONG_MAX;
				SendMessage(eIntensityScriptEditHandle, EM_SETSEL, (WPARAM)x1, (LPARAM)x2);
				finScrollPos = GetScrollPos(eIntensityScriptEditHandle, SB_VERT);
				SendMessage(eIntensityScriptEditHandle, EM_LINESCROLL, 0, -(finScrollPos - initScrollPos));
				if (tempSaved == true)
				{
					eIntensityScriptSaved = true;
					SendMessage(eIntensityScriptSavedIndicatorHandle, BM_SETCHECK, BST_CHECKED, NULL);
				}
			}
			break;
		}
		/// Handle Everything Else...///////////////////////
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_INTENSITY_SCRIPT_VIEW_COMBO:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						// ask to save current script
						// load the new intensity script
						// intensity script predefined scripts are not currently incorporated.
						MessageBox(0, "Intensity (only intensity) predefined scripts are not currently incorporated into the program.", 0, 0);
					}
					break;
				}
				case IDC_VERTICAL_SCRIPT_VIEW_COMBO:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						if (eVerticalScriptSaved == false)
						{
							// ask to save current script
							if (fileManage::checkSaveScript("Vertical", eVerticalScriptEditHandle, thisWindow, eVerticalCurrentViewScriptName,
								eVerticalScriptSavedIndicatorHandle, eVerticalScriptSaved, eVerticalViewScriptPathString, eVerticalScriptNameTextHandle) == 0)
							{
								break;
							}
						}
						// check what was selected.
						int selection = SendMessage(eVerticalViewCombo.hwnd, CB_GETCURSEL, 0, 0);
						TCHAR selectedText[256];
						SendMessage(eVerticalViewCombo.hwnd, CB_GETLBTEXT, selection, (LPARAM)selectedText);
						if (std::string(selectedText) == "Parent Script")
						{
							eCurrentVerticalViewIsParent = true;
							// load the parent script
							fileManage::openScript(thisWindow, eVerticalParentScriptPathString, eVerticalCurrentParentScriptName, eVerticalScriptEditHandle,
												   eVerticalScriptSavedIndicatorHandle, eVerticalScriptNameTextHandle, eVerticalScriptSaved, false, false, 
												   eCurrentVerticalViewIsParent);
						}
						else
						{
							eCurrentVerticalViewIsParent = false;
							// construct the view names
							eVerticalViewScriptPathString = eCurrentCategoryFolder + "\\" + std::string(selectedText) + ".script";
							strcpy_s(eVerticalCurrentViewScriptName, selectedText);
							fileManage::openScript(thisWindow, eVerticalViewScriptPathString, eVerticalCurrentViewScriptName, eVerticalScriptEditHandle,
												   eVerticalScriptSavedIndicatorHandle, eVerticalScriptNameTextHandle, eVerticalScriptSaved, false, false, 
												   eCurrentVerticalViewIsParent);
							// load the selected script.
						}
					}
					break;
				}
				case IDC_HORIZONTAL_SCRIPT_VIEW_COMBO:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						if (eHorizontalScriptSaved == false)
						{
							// ask to save current script
							if (fileManage::checkSaveScript("Horizontal", eHorizontalScriptEditHandle, thisWindow, eHorizontalCurrentViewScriptName,
								eHorizontalScriptSavedIndicatorHandle, eHorizontalScriptSaved, eHorizontalViewScriptPathString, eHorizontalScriptNameTextHandle) == 0)
							{
								break;
							}
						}
						// check what was selected.
						int selection = SendMessage(eHorizontalViewCombo.hwnd, CB_GETCURSEL, 0, 0);
						TCHAR selectedText[256];
						SendMessage(eHorizontalViewCombo.hwnd, CB_GETLBTEXT, selection, (LPARAM)selectedText);
						if (std::string(selectedText) == "Parent Script")
						{
							eCurrentHorizontalViewIsParent = true;
							// load the parent script
							fileManage::openScript(thisWindow, eHorizontalParentScriptPathString, eHorizontalCurrentParentScriptName, eHorizontalScriptEditHandle,
								eHorizontalScriptSavedIndicatorHandle, eHorizontalScriptNameTextHandle, eHorizontalScriptSaved, false, false, eCurrentHorizontalViewIsParent);
						}
						else
						{
							eCurrentHorizontalViewIsParent = false;
							// construct the view names
							eHorizontalViewScriptPathString = eCurrentCategoryFolder + "\\" + std::string(selectedText) + ".script";
							strcpy_s(eHorizontalCurrentViewScriptName, selectedText);
							fileManage::openScript(thisWindow, eHorizontalViewScriptPathString, eHorizontalCurrentViewScriptName, eHorizontalScriptEditHandle,
								eHorizontalScriptSavedIndicatorHandle, eHorizontalScriptNameTextHandle, eHorizontalScriptSaved, false, false, eCurrentHorizontalViewIsParent);
							// load the selected script.
						}
					}
					break;
				}
				/// vertical Script File
				case IDC_VERTICAL_SCRIPT_EDIT:
				{
					// Gets called whenever the user makes changes to this edit control.
					if (HIWORD(wParam) == EN_CHANGE)
					{
						DWORD begin, end;
						SendMessage(eVerticalScriptEditHandle, EM_GETSEL, (WPARAM)&begin, (LPARAM)&end);
						if (begin < eVerticalMinChange)
						{
							eVerticalMinChange = begin;
						}
						if (end > eVerticalMaxChange)
						{
							eVerticalMaxChange = end;
						}
						eVerticalSyntaxColorIsCurrent = false;
						eVerticalScriptSaved = false;
						SendMessage(eVerticalScriptSavedIndicatorHandle, BM_SETCHECK, BST_UNCHECKED, NULL);
						// initialize syntax timer
						SetTimer(eScriptingWindowHandle, SYNTAX_TIMER_ID, SYNTAX_TIMER_LENGTH, (TIMERPROC)NULL);
					}
					break;
				}
				/// horizontal Script File
				case IDC_HORIZONTAL_SCRIPT_EDIT:
				{
					// Gets called whenever the user makes changes to this edit control.
					if (HIWORD(wParam) == EN_CHANGE)
					{
						DWORD begin, end;
						SendMessage(eHorizontalScriptEditHandle, EM_GETSEL, (WPARAM)&begin, (LPARAM)&end);
						if (begin < eHorizontalMinChange)
						{
							eHorizontalMinChange = begin;
						}
						if (end > eHorizontalMinChange)
						{
							eHorizontalMaxChange = end;
						}
						eHorizontalSyntaxColorIsCurrent = false;
						eHorizontalScriptSaved = false;
						SendMessage(eHorizontalScriptSavedIndicatorHandle, BM_SETCHECK, BST_UNCHECKED, NULL);
						// initialize syntax timer
						SetTimer(eScriptingWindowHandle, SYNTAX_TIMER_ID, SYNTAX_TIMER_LENGTH, (TIMERPROC)NULL);
					}
					break;
				}
				/// Intensity Script File
				case IDC_INTENSITY_SCRIPT_EDIT:
				{
					// Gets called whenever the user makes changes to this edit control.
					if (HIWORD(wParam) == EN_CHANGE)
					{
						DWORD begin, end;
						SendMessage(eIntensityScriptEditHandle, EM_GETSEL, (WPARAM)&begin, (LPARAM)&end);
						if (begin < eIntensityMinChange)
						{
							eIntensityMinChange = begin;
						}
						if (end > eIntensityMaxChange)
						{
							eIntensityMaxChange = end;
						}
						eIntensitySyntaxColorIsCurrent = false;
						eIntensityScriptSaved = false;
						SendMessage(eIntensityScriptSavedIndicatorHandle, BM_SETCHECK, BST_UNCHECKED, NULL);
						// initialize syntax timer
						SetTimer(eScriptingWindowHandle, SYNTAX_TIMER_ID, SYNTAX_TIMER_LENGTH, (TIMERPROC)NULL);
					}
					break;
				}
				/// Begin System
				case ID_FILE_MY_RUN:
				case ID_ACCELERATOR_F5:
				case ID_FILE_MY_WRITE_WAVEFORMS:
				{
					menuAndAcceleratorFunctions::startSystem(thisWindow, wParam);
					break;
				}
				case ID_ACCELERATOR_ESC:
				case ID_FILE_ABORT_GENERATION:
				{				
					menuAndAcceleratorFunctions::abortSystem(thisWindow);
					break;
				}
				/// Menu Items
				case ID_FILE_SAVEALL:
				{
					menuAndAcceleratorFunctions::saveAll(thisWindow);
					break;
				}
				case ID_FILE_MY_EXIT: 
				{	
					menuAndAcceleratorFunctions::exitProgram(thisWindow);
					break;
				}
				case ID_FILE_MY_INTENSITY_NEW:
				{
					menuAndAcceleratorFunctions::newIntensityScript(thisWindow);
					break;
				}
				case ID_FILE_MY_INTENSITY_OPEN:
				{
					menuAndAcceleratorFunctions::openIntensityScript(thisWindow);
					break;
				}
				case ID_FILE_MY_INTENSITY_SAVE:
				{
					menuAndAcceleratorFunctions::saveIntensityScript(thisWindow);
					break;
				}
				case ID_FILE_MY_INTENSITY_SAVEAS:
				{
					menuAndAcceleratorFunctions::saveIntensityScriptAs(thisWindow);
					break;
				}				
				case ID_FILE_MY_VERTICAL_NEW:
				{
					menuAndAcceleratorFunctions::newVerticalScript(thisWindow);
					break;
				}
				case ID_FILE_MY_VERTICAL_OPEN:
				{
					menuAndAcceleratorFunctions::openVerticalScript(thisWindow);
					break;
				}
				case ID_FILE_MY_VERTICAL_SAVE:
				{
					menuAndAcceleratorFunctions::saveVerticalScript(thisWindow);
					break;
				}
				case ID_FILE_MY_VERTICAL_SAVEAS:
				{
					menuAndAcceleratorFunctions::saveVerticalScriptAs(thisWindow);
					break;
				}

				case ID_FILE_MY_HORIZONTAL_NEW:
				{
					menuAndAcceleratorFunctions::newHorizontalScript(thisWindow);
					break;
				}
				case ID_FILE_MY_HORIZONTAL_OPEN:
				{
					menuAndAcceleratorFunctions::openHorizontalScript(thisWindow);
					break;
				}
				case ID_FILE_MY_HORIZONTAL_SAVE:
				{
					menuAndAcceleratorFunctions::saveHorizontalScript(thisWindow);
					break;
				}
				case ID_FILE_MY_HORIZONTAL_SAVEAS:
				{
					menuAndAcceleratorFunctions::saveHorizontalScriptAs(thisWindow);
					break;
				}
				case ID_PROFILE_SAVE_PROFILE:
				{
					menuAndAcceleratorFunctions::saveProfile(thisWindow);
					break;
				}
				case ID_HELP_SCRIPT:
				{
					menuAndAcceleratorFunctions::helpWindow(thisWindow);
					break;
				}
				case ID_HELP_GENERALINFORMATION:
				{
					break;
				}
				case ID_NIAWG_RELOADDEFAULTWAVEFORMS:
				{
					menuAndAcceleratorFunctions::reloadNIAWGDefaults(thisWindow);
					break;
				}

				case ID_EXPERIMENT_NEW_EXPERIMENT_TYPE:
				{
					menuAndAcceleratorFunctions::newExperimentType(thisWindow);
					break;
				}
				case ID_CATEGORY_NEW_CATEGORY:
				{
					menuAndAcceleratorFunctions::newCategory(thisWindow);
					break;
				}
				case ID_CONFIGURATION_SAVE_CONFIGURATION_AS:
				{
					menuAndAcceleratorFunctions::saveConfigurationAs(thisWindow);
					break;
				}
				case ID_EXPERIMENT_RENAME_CURRENT_EXPERIMENT:
				{
					menuAndAcceleratorFunctions::renameCurrentExperimentType(thisWindow);
					break;
				}
				case ID_EXPERIMENT_DELETE_CURRENT_EXPERIMENT:
				{
					menuAndAcceleratorFunctions::deleteCurrentExperimentType(thisWindow);
					break;
				}
				case ID_CATEGORY_RENAME_CURRENT_CATEGORY:
				{
					menuAndAcceleratorFunctions::renameCurrentCategory(thisWindow);
					break;
				}
				case ID_CATEGORY_DELETE_CURRENT_CATEGORY:
				{
					menuAndAcceleratorFunctions::deleteCurrentCategory(thisWindow);
					break;
				}
				case ID_CONFIGURATION_NEW_CONFIGURATION:
				{
					menuAndAcceleratorFunctions::newConfiguration(thisWindow);
					break;
				}
				case ID_CONFIGURATION_RENAME_CURRENT_CONFIGURATION:
				{
					menuAndAcceleratorFunctions::renameCurrentConfiguration(thisWindow);
					break;
				}
				case ID_CONFIGURATION_DELETE_CURRENT_CONFIGURATION:
				{
					menuAndAcceleratorFunctions::deleteCurrentConfiguration(thisWindow);
					break;
				}

			}
			break;
		}
	}

	return DefWindowProc(thisWindow, msg, wParam, lParam);
}
