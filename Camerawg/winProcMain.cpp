#include "stdafx.h"
#include "winProcMain.h"

#include "textPromptDialogProcedure.h"

#include "scriptWriteHelpProc.h"
#include "fonts.h"
#include "commonMessages.h"

#include "boost/lexical_cast.hpp"
#include <boost/algorithm/string.hpp>
#include "time.h"
#include "Windows.h"
#include "initializeMainWindow.h"
#include "appendText.h"
#include "experimentThreadInputStructure.h"
#include "experimentProgrammingThread.h"
#include "myAgilent.h"
#include "myNIAWG.h"
#include "myErrorHandler.h"

#include "getFileName.h"

/**
 * This functin handles the creation and messages for the scripting window. Inputs are message information.
 */
LRESULT CALLBACK winProcMain(HWND thisWindow, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		/// Called at beginning of program.
		case WM_CREATE:
		{
			initializeMainWindow(thisWindow);
			break;
		}
		/// Called when window is closed
		case WM_CLOSE:
		case WM_DESTROY:
		{
			/*
			if (eHorizontalNIAWGScript.checkSave())
			{
				break;
			}
			if (eVerticalNIAWGScript.checkSave())
			{
				break;
			}
			if (eIntensityAgilentScript.checkSave())
			{
				break;
			}
			*/

			/*
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
			*/
			std::string exitQuestion = "Are you sure you want to exit?\n\nThis will stop all output of the arbitrary waveform generator.";
			int areYouSure = MessageBox(NULL, exitQuestion.c_str(), "Exit", MB_OKCANCEL | MB_ICONWARNING);
			switch (areYouSure)
			{
				case IDOK:
				{
					PostQuitMessage(1);
					break;
				}
				case IDCANCEL:
				{
					break;
				}
				default:
				{
					break;
				}
			}
			break;
		}
		case WM_CTLCOLOREDIT:
		{
			DWORD CtrlID = GetDlgCtrlID((HWND)lParam); // Window Control ID
			HDC hdcStatic = (HDC)wParam;
			if (false)
			{
				SetTextColor(hdcStatic, RGB(255, 255, 255));
				SetBkColor(hdcStatic, RGB(15, 15, 20));
				return (INT_PTR)eNearBlackBlueBrush;
			}
			else
			{
				SetTextColor(hdcStatic, RGB(255, 215, 0));
				SetBkColor(hdcStatic, RGB(0, 60, 80));
				return (INT_PTR)eTealBrush;
			}
			break;
		}
		case WM_CTLCOLORSTATIC:
		{
			// Get the control id of the control wanting color.
			DWORD CtrlID = GetDlgCtrlID((HWND)lParam); 
			HDC hdcStatic = (HDC)wParam;
			if (false)
			{
				SetTextColor(hdcStatic, RGB(255, 255, 255));
				SetBkColor(hdcStatic, RGB(0,0,75));
				return (INT_PTR)eDarkBlueBrush;
			}
			/*
			else if (CtrlID == IDC_SYSTEM_STATUS_TEXT)
			{
				SetTextColor(hdcStatic, RGB(50, 50, 250));
				SetBkColor(hdcStatic, RGB(15, 15, 20));
				return (INT_PTR)eNearBlackBlueBrush;
			}

			
			else if (CtrlID == IDC_SYSTEM_DEBUG_TEXT)
			{
				SetTextColor(hdcStatic, RGB(13, 152, 186));
				SetBkColor(hdcStatic, RGB(15, 15, 20));
				return (INT_PTR)eNearBlackBlueBrush;
			}

			else if (CtrlID == IDC_SYSTEM_ERROR_TEXT)
			{
				SetTextColor(hdcStatic, RGB(200, 0, 0));
				SetBkColor(hdcStatic, RGB(15, 15, 20));
				return (INT_PTR)eNearBlackBlueBrush;
			}
			*/

			else if (false)
			{
				SetTextColor(hdcStatic, RGB(255,255,255));
				SetBkColor(hdcStatic, RGB(0, 60, 80));
				return (INT_PTR)eTealBrush;
			}
			else if (false)
			{
				SetTextColor(hdcStatic, RGB(255, 255, 255));
				SetBkColor(hdcStatic, RGB(25, 25, 35));
				return (INT_PTR)eVeryDarkBlueBrush;
			}
			else if (false)
			{
				SetTextColor(hdcStatic, RGB(255, 255, 255));
				SetBkColor(hdcStatic, RGB(32, 0, 65));
				return (INT_PTR)eDarkPurpleBrush;
			}
			else if (false)
			{
				/*
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
				*/
			}
			else
			{
				SetTextColor(hdcStatic, RGB(255, 255, 255));
				SetBkColor(hdcStatic, RGB(0, 60, 80));
				return (INT_PTR)eTealBrush;
			}
		}
		case WM_NOTIFY:
		{
			int notifyMessage = ((LPNMHDR)lParam)->code;
			switch (notifyMessage)
			{
				case NM_DBLCLK:
				{
					///eVariables.updateVariableInfo(lParam);
					//eTextingHandler.updatePersonInfo(hWnd, lParam);
					break;
				}
				case NM_RCLICK:
				{
					//eVariables.deleteVariable(lParam);
					//eTextingHandler.deletePersonInfo(hWnd, lParam);
					break;
				}
			}
			break;
		}
		/// Everything Else
		case WM_COMMAND:
		{
			//if (eDebugger.handleEvent(thisWindow, msg, wParam, lParam))
			//{
			//	break;
			//}
			switch (LOWORD(wParam))
			{
				/*
				case IDC_EXPERIMENT_NOTES:
				{
					if (HIWORD(wParam) == EN_CHANGE)
					{
						//eProfile.updateExperimentSavedStatus(false);
						break;
					}
				}
				case IDC_CATEGORY_NOTES:
				{
					if (HIWORD(wParam) == EN_CHANGE)
					{
						//eProfile.updateCategorySavedStatus(false);
						break;
					}
				}
				
				case IDC_CONFIGURATION_NOTES:
				{
					if (HIWORD(wParam) == EN_CHANGE)
					{
						//eProfile.updateConfigurationSavedStatus(false);
					}
					break;
				}
				case IDC_EXPERIMENT_COMBO:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						//eProfile.experimentChangeHandler(thisWindow);
					}
					break;
				}
				case IDC_ORIENTATION_COMBO:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						//eProfile.orientationChangeHandler(thisWindow);
					}
					break;
				}
				case IDC_CATEGORY_COMBO:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						//eProfile.categoryChangeHandler(thisWindow);
					}
					break;
				}
				case IDC_CONFIGURATION_COMBO:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						///eProfile.configurationChangeHandler(thisWindow);
						break;
						/*
						if (eConfigurationSaved == false)
						{
							// check if the user wants to save
							int cont = fileManage::checkExperimentSave(thisWindow);
							// this occurs if the user presses cancel. Just break, don't open.
							if (cont == 0)
							{
								break;
							}
						}
						// If the user makes a selection from the list:
						// Send CB_GETCURSEL message to get the index of the selected list item.
						// Send CB_GETLBTEXT message to get the item.
						long long itemIndex = SendMessage(eConfigurationCombo, CB_GETCURSEL, 0, 0);
						TCHAR subConfiguration[256];
						SendMessage(eConfigurationCombo, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)subConfiguration);
						if (itemIndex == -1)
						{
							// This means that the oreintation combo was set before the Configuration list combo was set so that the configuration list combo
							// is blank. just break out, this is fine.
							break;
						}
						eCurrentConfigurationName = subConfiguration;
						if (eCurrentOrientation == "Horizontal")
						{
							eCurrentConfigurationLocation = eCurrentCategoryFolder + "\\" + std::string(subConfiguration) + ".hSubConfig";
						}
						else if (eCurrentOrientation == "Vertical")
						{
							eCurrentConfigurationLocation = eCurrentCategoryFolder + "\\" + std::string(subConfiguration) + ".vSubConfig";
						}
						// Load configuration
						fileManage::openConfiguration(thisWindow, eCurrentConfigurationLocation);
						eConfigurationSaved = true;
						SendMessage(eConfigurationSavedIndicatorHandle, BM_SETCHECK, BST_CHECKED, NULL);
						// check if no sequence selected.
						int selection = SendMessage(eSequenceCombo.hwnd, CB_GETCURSEL, 0, 0);
						TCHAR selectionText[256];
						SendMessage(eSequenceCombo.hwnd, CB_GETLBTEXT, selection, (LPARAM)selectionText);
						if (std::string(selectionText) == "NO SEQUENCE")
						{
							eSequenceFileNames.clear();
							if (eCurrentConfigurationName != "")
							{
								if (eCurrentOrientation == "Horizontal")
								{
									eSequenceFileNames.push_back(eCurrentConfigurationName + ".hSubConfig");
								}
								else if (eCurrentOrientation == "Vertical")
								{
									eSequenceFileNames.push_back(eCurrentConfigurationName + ".vSubConfig");
								}
							}
							// reset the edit
							SendMessage(eSequenceDisplay.hwnd, WM_SETTEXT, 256, (LPARAM)"Sequence of Configurations to Run:\r\n");
							appendText("1. " + eSequenceFileNames[0] + "\r\n", IDC_SEQUENCE_DISPLAY, eMainWindowHandle);
						}
						SendMessage(eConfigurationDisplayInScripting, WM_SETTEXT, 0, (LPARAM)eCurrentConfigurationName.c_str());
						
					}
					// Continue to sequence combo handling. This makes sure that he configuration combobox gets reset.
				}
				case IDC_SEQUENCE_COMBO:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						//eProfile.sequenceChangeHandler();
					}
					break;
				}
				*/
				/// All of the following messages are common to all windows in this program.
				case ID_FILE_MY_RUN:
				case ID_ACCELERATOR_F5:
				case ID_FILE_MY_WRITE_WAVEFORMS:
				case ID_ACCELERATOR_ESC:
				case ID_FILE_ABORT_GENERATION:
				case ID_FILE_SAVEALL:
				case ID_FILE_MY_EXIT:
				case ID_FILE_MY_INTENSITY_NEW:
				case ID_FILE_MY_INTENSITY_OPEN:
				case ID_FILE_MY_INTENSITY_SAVE:
				case ID_FILE_MY_INTENSITY_SAVEAS:
				case ID_FILE_MY_VERTICAL_NEW:
				case ID_FILE_MY_VERTICAL_OPEN:
				case ID_FILE_MY_VERTICAL_SAVE:
				case ID_FILE_MY_VERTICAL_SAVEAS:
				case ID_FILE_MY_HORIZONTAL_NEW:
				case ID_FILE_MY_HORIZONTAL_OPEN:
				case ID_FILE_MY_HORIZONTAL_SAVE:
				case ID_FILE_MY_HORIZONTAL_SAVEAS:
				case ID_SEQUENCE_ADD_TO_SEQUENCE:
				case ID_SEQUENCE_SAVE_SEQUENCE:
				case ID_SEQUENCE_NEW_SEQUENCE:
				case ID_SEQUENCE_RESET_SEQUENCE:
				case ID_SEQUENCE_DELETE_SEQUENCE:
				case ID_PROFILE_SAVE_PROFILE:
				case ID_HELP_SCRIPT:
				case ID_HELP_GENERALINFORMATION:
				case ID_NIAWG_RELOADDEFAULTWAVEFORMS:
				case ID_EXPERIMENT_NEW_EXPERIMENT_TYPE:
				case ID_CATEGORY_NEW_CATEGORY:
				case ID_CONFIGURATION_SAVE_CONFIGURATION_AS:
				case ID_EXPERIMENT_RENAME_CURRENT_EXPERIMENT:
				case ID_EXPERIMENT_DELETE_CURRENT_EXPERIMENT:
				case ID_CATEGORY_RENAME_CURRENT_CATEGORY:
				case ID_CATEGORY_DELETE_CURRENT_CATEGORY:
				case ID_CONFIGURATION_NEW_CONFIGURATION:
				case ID_CONFIGURATION_RENAME_CURRENT_CONFIGURATION:
				case ID_CONFIGURATION_DELETE_CURRENT_CONFIGURATION:
				case ID_EXPERIMENT_SAVEEXPERIMENTSETTINGS:
				case ID_EXPERIMENT_SAVEEXPERIMENTSETTINGSAS:
				case ID_CATEGORY_SAVECATEGORYSETTINGS:
				case ID_CONFIGURATION_SAVECONFIGURATIONSETTINGS:
				case ID_SEQUENCE_RENAMESEQUENCE:
				{
					//commonMessages::handleCommonMessage(thisWindow, msg, wParam, lParam);
					break;
				}
				/// Status Controls
				/*
				case IDC_SCRIPT_STATUS_CLEAR_BUTTON:
				{
					// send blank message
					//SendMessage(eSystemStatusTextHandle, WM_SETTEXT, NULL, (LPARAM)"*********************\r\n");
					break;
				}
				case IDC_SCRIPT_DEBUG_CLEAR_BUTTON:
				{
					// send blank message
					//SendMessage(eSystemDebugTextHandle, WM_SETTEXT, NULL, (LPARAM)"**********************\r\n");
					break;
				}
				case IDC_SCRIPT_ERROR_CLEAR_BUTTON:
				{
					// send blank message
					//SendMessage(eSystemErrorTextHandle, WM_SETTEXT, NULL, (LPARAM)"**********************\r\n");
					break;
				}
				/*
				case IDC_VAR_SET_BUTTON:
				{
					std::string varList;
					LRESULT varLength = SendMessage(eVariableNamesEditHandle, WM_GETTEXTLENGTH, 0, 0);
					if (varLength == 0)
					{
						// Happens if the user enters nothing into the save button and tries to enter. I interpret this as "get rid fo the variable".
						varList = "0";
					}
					char* buffer = new char[varLength + 1];
					SendMessage(eVariableNamesEditHandle, WM_GETTEXT, varLength + 1, (LPARAM)buffer);
					varList = buffer;
					delete[] buffer;
					// Gets called if the user enters either a '0' or '' (nothing) into the variable name before saving.
					if (varLength == 0)
					{
						SetWindowText(eVar1NameTextHandle, '\0');
						SetWindowText(eVar2NameTextHandle, '\0');
						SetWindowText(eVar3NameTextHandle, '\0');
						SetWindowText(eVar4NameTextHandle, '\0');
						SetWindowText(eVar5NameTextHandle, '\0');
						SetWindowText(eVar6NameTextHandle, '\0');
						eVariableNames.resize(0);
						eVerticalVarFileNames.resize(0);
						char tempChar2 = '\0';
						break;
					}
					// resize to zero in order to reload variable names.
					eVariableNames.resize(0);
					// split up the list.
					boost::split(eVariableNames, varList, boost::is_any_of(" ,;\t\r\n"), boost::token_compress_on);
					// Check the number of variables.
					if (eVariableNames.size() > 6)
					{
						MessageBox(NULL, "ERROR: This code only currently supports up to 6 variables, and it looks like you entered more.", NULL, MB_OK);
						break;
					}
					// Check the size of the names. Also check for forbidden characters. Also check for repeats.
					bool errCheck = false;
					for (int varInc = 0; varInc < eVariableNames.size(); varInc++)
					{
						// change the entry to lowercase. Uppercase variables are forbidden.
						boost::algorithm::to_lower(eVariableNames[varInc]);
						// run checks.
						if (eVariableNames[varInc].size() > 10)
						{
							MessageBox(NULL, "ERROR: One of your variable names is too long. The maximum size for the variable name is 10 characters.", NULL, MB_OK);
							errCheck = true;
							break;
						}
						if (eVariableNames[varInc] == "\'")
						{
							MessageBox(NULL, "ERROR: The character \' cannot be used as a variable! it is reserved by the code for other purposes.", NULL, MB_OK);
							errCheck = true;
							break;
						}
						if (eVariableNames[varInc] == "%")
						{
							MessageBox(NULL, "ERROR: The character % cannot be used as a variable! it is reserved by the code for other purposes.", NULL, MB_OK);
							errCheck = true;
							break;
						}
						if (eVariableNames[varInc][0] == '%')
						{
							MessageBox(NULL, "ERROR: The character % cannot be used as the first character of a variable! The code will read this variable as a "
									   "comment if you try.", NULL, MB_OK);
							errCheck = true;
							break;
						}
						if (eVariableNames[varInc][0] == ' ')
						{
							MessageBox(NULL, "ERROR: The character ' ' cannot be used as a variable!", NULL, MB_OK);
							errCheck = true;
							break;
						}
						if (eVariableNames[varInc] == "#")
						{
							MessageBox(NULL, "ERROR: The character ' ' cannot be used as a variable!", NULL, MB_OK);
							errCheck = true;
							break;
						}
						bool errCheckInner = false;
						// check if variable name is the same as other variable name.
						for (int varIncInner = 0; varIncInner < eVariableNames.size(); varIncInner++)
						{
							if (varIncInner == varInc)
							{
								continue;
							}
							else if (eVariableNames[varInc] == eVariableNames[varIncInner])
							{
								MessageBox(0, "ERROR: You entered the same variable twice! Note that you only need to enter a given variable once regardless "
										   "of how many times it's used in the scripts, and that variable names are not case-sensitive.", 0, 0);
								errCheckInner = true;
								break;
							}
						}
					}
					if (errCheck == true)
					{
						break;
					}
					// Load the variable names into the windows.
					if (eVariableNames.size() < 6)
					{
						SetWindowText(eVar6NameTextHandle, '\0');
					}
					else
					{
						SetWindowText(eVar6NameTextHandle, eVariableNames[5].c_str());
					}
					if (eVariableNames.size() < 5)
					{
						SetWindowText(eVar5NameTextHandle, '\0');
					}
					else
					{
						SetWindowText(eVar5NameTextHandle, eVariableNames[4].c_str());
					}
					if (eVariableNames.size() < 4)
					{
						SetWindowText(eVar4NameTextHandle, '\0');
					}
					else
					{
						SetWindowText(eVar4NameTextHandle, eVariableNames[3].c_str());
					}
					if (eVariableNames.size() < 3)
					{
						SetWindowText(eVar3NameTextHandle, '\0');
					}
					else
					{
						SetWindowText(eVar3NameTextHandle, eVariableNames[2].c_str());
					}
					if (eVariableNames.size() < 2)
					{
						SetWindowText(eVar2NameTextHandle, '\0');
					}
					else
					{
						SetWindowText(eVar2NameTextHandle, eVariableNames[1].c_str());
					}
					if (eVariableNames.size() < 1)
					{
						SetWindowText(eVar1NameTextHandle, '\0');
					}
					else
					{
						SetWindowText(eVar1NameTextHandle, eVariableNames[0].c_str());
						// Can't use adummy variable if you use real variables.
						CheckDlgButton(thisWindow, IDC_DUMMY_BUTTON, BST_UNCHECKED);
						eUseDummyVariables = false;
					}
					eProfile.updateExperimentSavedStatus(false);
					break;
				}
				
				// Debugging and window output options
				case IDC_RECEIVE_VAR_FILES_BUTTON:
				{
					BOOL checked = IsDlgButtonChecked(thisWindow, IDC_RECEIVE_VAR_FILES_BUTTON);
					if (checked) {
						CheckDlgButton(thisWindow, IDC_RECEIVE_VAR_FILES_BUTTON, BST_UNCHECKED);
						eGetVarFilesFromMaster = false;
					}
					else {
						CheckDlgButton(thisWindow, IDC_RECEIVE_VAR_FILES_BUTTON, BST_CHECKED);
						eGetVarFilesFromMaster = true;
					}
					//eProfile.updateExperimentSavedStatus(false);
					break;
				}
				/*
				case IDC_OUTPUT_READ_STATUS:
				{
					BOOL checked = IsDlgButtonChecked(thisWindow, IDC_OUTPUT_READ_STATUS);
					if (checked) 
					{
						CheckDlgButton(thisWindow, IDC_OUTPUT_READ_STATUS, BST_UNCHECKED);
						eOutputReadStatus = false;
					}
					else 
					{
						CheckDlgButton(thisWindow, IDC_OUTPUT_READ_STATUS, BST_CHECKED);
						eOutputReadStatus = true;
					}
					//eProfile.updateExperimentSavedStatus(false);
					break;
				}
				case IDC_OUTPUT_WRITE_STATUS:
				{
					BOOL checked = IsDlgButtonChecked(thisWindow, IDC_OUTPUT_WRITE_STATUS);
					if (checked) 
					{
						CheckDlgButton(thisWindow, IDC_OUTPUT_WRITE_STATUS, BST_UNCHECKED);
						eOutputWriteStatus = false;
					}
					else 
					{
						CheckDlgButton(thisWindow, IDC_OUTPUT_WRITE_STATUS, BST_CHECKED);
						eOutputWriteStatus = true;
					}
					//eProfile.updateExperimentSavedStatus(false);
					break;
				}
				case 
				
				:
				{
					BOOL checked = IsDlgButtonChecked(thisWindow, IDC_OUTPUT_MORE_RUN_INFO);
					if (checked)
					{
						CheckDlgButton(thisWindow, IDC_OUTPUT_MORE_RUN_INFO, BST_UNCHECKED);
						eOutputRunInfo = false;
					}
					else
					{
						CheckDlgButton(thisWindow, IDC_OUTPUT_MORE_RUN_INFO, BST_CHECKED);
						eOutputRunInfo = true;
					}
					//eProfile.updateExperimentSavedStatus(false);
					break;
				}
				
				case IDC_CONNECT_TO_MASTER_BUTTON:
				{
					BOOL checked = IsDlgButtonChecked(thisWindow, IDC_CONNECT_TO_MASTER_BUTTON);
					if (checked) 
					{
						CheckDlgButton(thisWindow, IDC_CONNECT_TO_MASTER_BUTTON, BST_UNCHECKED);
						eConnectToMaster = false;
					}
					else 
					{
						CheckDlgButton(thisWindow, IDC_CONNECT_TO_MASTER_BUTTON, BST_CHECKED);
						eConnectToMaster = true;
					}
					//eProfile.updateExperimentSavedStatus(false);
					break;
				}
				case IDC_LOG_SCRIPT_PARAMS:
				{
					BOOL checked = IsDlgButtonChecked(thisWindow, IDC_LOG_SCRIPT_PARAMS);
					if (checked) 
					{
						CheckDlgButton(thisWindow, IDC_LOG_SCRIPT_PARAMS, BST_UNCHECKED);
						eLogScriptAndParams = false;
					}
					else 
					{
						CheckDlgButton(thisWindow, IDC_LOG_SCRIPT_PARAMS, BST_CHECKED);
						eLogScriptAndParams = true;
					}
					//eProfile.updateExperimentSavedStatus(false);
					break;
				}
				case IDC_PROGRAM_INTENSITY_BOX:
				{
					BOOL checked = IsDlgButtonChecked(thisWindow, IDC_PROGRAM_INTENSITY_BOX);
					if (checked) 
					{
						CheckDlgButton(thisWindow, IDC_PROGRAM_INTENSITY_BOX, BST_UNCHECKED);
						eProgramIntensityOption = false;
					}
					else 
					{
						CheckDlgButton(thisWindow, IDC_PROGRAM_INTENSITY_BOX, BST_CHECKED);
						eProgramIntensityOption = true;
					}
					//eProfile.updateExperimentSavedStatus(false);
					break;
				}
				/*
				case 
				:
				{
					BOOL checked = IsDlgButtonChecked(thisWindow, IDC_OUTPUT_CORR_TIME_BUTTON);
					if (checked) 
					{
						CheckDlgButton(thisWindow, IDC_OUTPUT_CORR_TIME_BUTTON, BST_UNCHECKED);
						eOutputCorrTime = false;
					}
					else 
					{
						CheckDlgButton(thisWindow, IDC_OUTPUT_CORR_TIME_BUTTON, BST_CHECKED);
						eOutputCorrTime = true;
					}
					//eProfile.updateExperimentSavedStatus(false);
					break;
				}
				*/
				/*
				case IDC_DUMMY_BUTTON:
				{
					BOOL checked = IsDlgButtonChecked(thisWindow, IDC_DUMMY_BUTTON);
					if (checked) 
					{
						CheckDlgButton(thisWindow, IDC_DUMMY_BUTTON, BST_UNCHECKED);
						eUseDummyVariables = false;
					}
					else 
					{
						CheckDlgButton(thisWindow, IDC_DUMMY_BUTTON, BST_CHECKED);
						eUseDummyVariables = true;
						// You can't use normal variables if you use a dummy variable.
						eVariableNames.resize(0);
						eVerticalVarFileNames.resize(0);
						char tempChar2 = '\0';
						SetWindowText(eVar1NameTextHandle, &tempChar2);
						// If these weren't already empty, they are now, so...
						SetWindowText(eVar2NameTextHandle, &tempChar2);
						SetWindowText(eVar3NameTextHandle, &tempChar2);
						SetWindowText(eVar4NameTextHandle, &tempChar2);
						SetWindowText(eVar5NameTextHandle, &tempChar2);
						SetWindowText(eVar6NameTextHandle, &tempChar2);
					}
					eProfile.updateExperimentSavedStatus(false);
					break;
				}
				*/
				/// File Management 


			}
		}
		// handle thread messages
		default:
		{
			if (msg == eVariableStatusMessageID)
			{
				int currentOutput = (int)lParam;
				std::string msgText = "Outpitting Series #" + std::to_string(currentOutput) + ". \r\nWriting Varying Waveforms for Set # "
									  + std::to_string(currentOutput + 1) + "...\r\n";
				SetWindowText(eColoredStatusEdit, msgText.c_str());
				eGenStatusColor = "Y";
				// Redraw the three status windows.
				RedrawWindow(eColorBox, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
				RedrawWindow(eColoredStatusEdit, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
				break;
			}
			if (msg == eGreenMessageID)
			{
				eGenStatusColor = "G";
				RedrawWindow(eColoredStatusEdit, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
				RedrawWindow(eColorBox, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
				break;
			}
			if (msg == eStatusTextMessageID)
			{
				char* pointerToMessage = (char*)lParam;
				std::string statusMessage(pointerToMessage);
				delete[] pointerToMessage;
				appendText(statusMessage, IDC_SYSTEM_STATUS_TEXT, eMainWindowHandle);
				break;
			}
			if (msg == eErrorTextMessageID)
			{
				char* pointerToMessage = (char*)lParam;
				std::string statusMessage(pointerToMessage);
				delete[] pointerToMessage;
				appendText(statusMessage, IDC_SYSTEM_ERROR_TEXT, eMainWindowHandle);
				break;
			}
			if (msg == eFatalErrorMessageID)
			{
				myAgilent::agilentDefault();
				std::string msgText = "Exited with Error!\r\nPassively Outputting Default Waveform.";
				SetWindowText(eColoredStatusEdit, msgText.c_str());
				eGenStatusColor = "R";
				RedrawWindow(eColoredStatusEdit, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
				RedrawWindow(eColorBox, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
				if (!TWEEZER_COMPUTER_SAFEMODE)
				{
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_FALSE)))
					{
						return DefWindowProc(thisWindow, msg, wParam, lParam);
					}
					// Officially stop trying to generate anything.				
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_AbortGeneration(eSessionHandle)))
					{
						return DefWindowProc(thisWindow, msg, wParam, lParam);
					}
					// clear the memory
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ClearArbMemory(eSessionHandle)))
					{
						return DefWindowProc(thisWindow, msg, wParam, lParam);
					}
				}
				ViInt32 waveID;
				if (eProfile.getOrientation() == HORIZONTAL_ORIENTATION)
				{
					if (!TWEEZER_COMPUTER_SAFEMODE)
					{
						// create waveform (necessary?)
						if (myNIAWG::NIAWG_CheckWindowsError(niFgen_CreateWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigMixedSize, eDefault_hConfigMixedWaveform, &waveID)))
						{
							return DefWindowProc(thisWindow, msg, wParam, lParam);
						}
						// allocate waveform into the device memory
						if (myNIAWG::NIAWG_CheckWindowsError(niFgen_AllocateNamedWaveform(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigWaveformName.c_str(), eDefault_hConfigMixedSize / 2)))
						{
							return DefWindowProc(thisWindow, msg, wParam, lParam);
						}
						// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
						if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteNamedWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigWaveformName.c_str(), eDefault_hConfigMixedSize, eDefault_hConfigMixedWaveform)))
						{
							return DefWindowProc(thisWindow, msg, wParam, lParam);
						}
						// rewrite the script. default_hConfigScript should still be valid.
						if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteScript(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigScript)))
						{
							return DefWindowProc(thisWindow, msg, wParam, lParam);
						}
						// start generic waveform to maintain power output to AOM.
						if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_TRUE)))
						{
							return DefWindowProc(thisWindow, msg, wParam, lParam);
						}
						if (myNIAWG::NIAWG_CheckWindowsError(niFgen_SetAttributeViString(eSessionHandle, SESSION_CHANNELS, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultHConfigScript")))
						{
							return DefWindowProc(thisWindow, msg, wParam, lParam);
						}
					}
					eCurrentScript = "DefaultHConfigScript";

				}
				else if (eProfile.getOrientation() == VERTICAL_ORIENTATION)
				{
					if (!TWEEZER_COMPUTER_SAFEMODE)
					{

						// create waveform (necessary?)
						if (myNIAWG::NIAWG_CheckWindowsError(niFgen_CreateWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigMixedSize, eDefault_vConfigMixedWaveform, &waveID)))
						{
							return DefWindowProc(thisWindow, msg, wParam, lParam);
						}
						// allocate waveform into the device memory
						if (myNIAWG::NIAWG_CheckWindowsError(niFgen_AllocateNamedWaveform(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigWaveformName.c_str(), eDefault_vConfigMixedSize / 2)))
						{
							return DefWindowProc(thisWindow, msg, wParam, lParam);
						}
						// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
						if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteNamedWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigWaveformName.c_str(), eDefault_vConfigMixedSize, eDefault_vConfigMixedWaveform)))
						{
							return DefWindowProc(thisWindow, msg, wParam, lParam);
						}
						// rewrite the script. default_hConfigScript should still be valid.
						if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteScript(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigScript)))
						{
							return DefWindowProc(thisWindow, msg, wParam, lParam);
						}
						// start generic waveform to maintain power output to AOM.
						if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_TRUE)))
						{
							return DefWindowProc(thisWindow, msg, wParam, lParam);
						}
						if (myNIAWG::NIAWG_CheckWindowsError(niFgen_SetAttributeViString(eSessionHandle, SESSION_CHANNELS, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultVConfigScript")))
						{
							return DefWindowProc(thisWindow, msg, wParam, lParam);
						}
					}
					eCurrentScript = "DefaultVConfigScript";
				}
				if (!TWEEZER_COMPUTER_SAFEMODE)
				{

					// Initiate Generation.
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_InitiateGeneration(eSessionHandle)))
					{
						return DefWindowProc(thisWindow, msg, wParam, lParam);
					}
				}
				eSystemIsRunning = false;
				break;
			}
			if (msg == eNormalFinishMessageID) 
			{
				myAgilent::agilentDefault();
				std::string msgText = "Passively Outputting Default Waveform";
				SetWindowText(eColoredStatusEdit, msgText.c_str());
				eGenStatusColor = "B";
				RedrawWindow(eColorBox, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
				RedrawWindow(eColoredStatusEdit, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
				if (!TWEEZER_COMPUTER_SAFEMODE)
				{

					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_FALSE)))
					{
						return DefWindowProc(thisWindow, msg, wParam, lParam);
					}
					// Officially stop trying to generate anything.
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_AbortGeneration(eSessionHandle)))
					{
						return DefWindowProc(thisWindow, msg, wParam, lParam);
					}
					// clear the memory
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ClearArbMemory(eSessionHandle)))
					{
						return DefWindowProc(thisWindow, msg, wParam, lParam);
					}
				}
				ViInt32 waveID;
				if (eProfile.getOrientation() == HORIZONTAL_ORIENTATION)
				{
					if (!TWEEZER_COMPUTER_SAFEMODE)
					{

						// create waveform (necessary?)
						if (myNIAWG::NIAWG_CheckWindowsError(niFgen_CreateWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigMixedSize, eDefault_hConfigMixedWaveform, &waveID)))
						{
							return DefWindowProc(thisWindow, msg, wParam, lParam);
						}
						// allocate waveform into the device memory
						if (myNIAWG::NIAWG_CheckWindowsError(niFgen_AllocateNamedWaveform(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigWaveformName.c_str(), eDefault_hConfigMixedSize / 2)))
						{
							return DefWindowProc(thisWindow, msg, wParam, lParam);
						}
						// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
						if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteNamedWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigWaveformName.c_str(), eDefault_hConfigMixedSize, eDefault_hConfigMixedWaveform)))
						{
							return DefWindowProc(thisWindow, msg, wParam, lParam);
						}
						// rewrite the script. default_hConfigScript should still be valid.
						if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteScript(eSessionHandle, SESSION_CHANNELS, eDefault_hConfigScript)))
						{
							return DefWindowProc(thisWindow, msg, wParam, lParam);
						}
						// start generic waveform to maintain power output to AOM.
						if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_TRUE)))
						{
							return DefWindowProc(thisWindow, msg, wParam, lParam);
						}
						if (myNIAWG::NIAWG_CheckWindowsError(niFgen_SetAttributeViString(eSessionHandle, SESSION_CHANNELS, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultHConfigScript")))
						{
							return DefWindowProc(thisWindow, msg, wParam, lParam);
						}
					}
					eCurrentScript = "DefaultHConfigScript";

				}
				else if (eProfile.getOrientation() == VERTICAL_ORIENTATION)
				{
					if (!TWEEZER_COMPUTER_SAFEMODE)
					{

						// create waveform (necessary?)
						if (myNIAWG::NIAWG_CheckWindowsError(niFgen_CreateWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigMixedSize, eDefault_vConfigMixedWaveform, &waveID)))
						{
							return DefWindowProc(thisWindow, msg, wParam, lParam);
						}
						// allocate waveform into the device memory
						if (myNIAWG::NIAWG_CheckWindowsError(niFgen_AllocateNamedWaveform(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigWaveformName.c_str(), eDefault_vConfigMixedSize / 2)))
						{
							return DefWindowProc(thisWindow, msg, wParam, lParam);
						}
						// write named waveform. on the device. Now the device knows what "waveform0" refers to when it sees it in the script. 
						if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteNamedWaveformF64(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigWaveformName.c_str(), eDefault_vConfigMixedSize, eDefault_vConfigMixedWaveform)))
						{
							return DefWindowProc(thisWindow, msg, wParam, lParam);
						}
						// rewrite the script. default_hConfigScript should still be valid.
						if (myNIAWG::NIAWG_CheckWindowsError(niFgen_WriteScript(eSessionHandle, SESSION_CHANNELS, eDefault_vConfigScript)))
						{
							return DefWindowProc(thisWindow, msg, wParam, lParam);
						}
						// start generic waveform to maintain power output to AOM.
						if (myNIAWG::NIAWG_CheckWindowsError(niFgen_ConfigureOutputEnabled(eSessionHandle, SESSION_CHANNELS, VI_TRUE)))
						{
							return DefWindowProc(thisWindow, msg, wParam, lParam);
						}
						if (myNIAWG::NIAWG_CheckWindowsError(niFgen_SetAttributeViString(eSessionHandle, SESSION_CHANNELS, NIFGEN_ATTR_SCRIPT_TO_GENERATE, "DefaultVConfigScript")))
						{
							return DefWindowProc(thisWindow, msg, wParam, lParam);
						}
					}
					eCurrentScript = "DefaultVConfigScript";
				}
				if (!TWEEZER_COMPUTER_SAFEMODE)
				{

					// Initiate Generation.
					if (myNIAWG::NIAWG_CheckWindowsError(niFgen_InitiateGeneration(eSessionHandle)))
					{
						return DefWindowProc(thisWindow, msg, wParam, lParam);
					}
				}
				eSystemIsRunning = false;
				break; 
			}
			if (msg == eColoredEditMessageID)
			{
				char* pointerToMessage = (char*)lParam;
				std::string statusMessage(pointerToMessage);
				delete[] pointerToMessage;
				SetWindowText(eColoredStatusEdit, statusMessage.c_str());
				//appendText(statusMessage, IDC_GUI_STAT_TEXT, eMainWindowHandle);
				//MessageBox(0, "", 0, 0);
				break;
			}
			break;
		}
	}

	return DefWindowProc(thisWindow, msg, wParam, lParam);

}
