#include "stdafx.h"
#include "winProcMain.h"

#include "namePromptDialogProc.h"

#include "scriptWriteHelpProc.h"
#include "fonts.h"
#include "fileManage.h"
#include "menuAndAcceleratorFunctions.h"

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
			if (eHorizontalScriptSaved == false)
			{
				// check if the user wants to save
				int cont = fileManage::checkSaveScript("horizontal", eHorizontalScriptEditHandle, thisWindow, eHorizontalCurrentParentScriptName, eHorizontalScriptSavedIndicatorHandle, eHorizontalScriptSaved, eHorizontalParentScriptPathString,
					eHorizontalScriptNameTextHandle);
				// this occurs if the user presses cancel. Just break, don't open.
				if (cont == 0)
				{
					break;
				}
			}
			if (eVerticalScriptSaved == false)
			{
				// check if the user wants to save
				int cont = fileManage::checkSaveScript("vertical", eVerticalScriptEditHandle, thisWindow, eVerticalCurrentParentScriptName, eVerticalScriptSavedIndicatorHandle, eVerticalScriptSaved, eVerticalParentScriptPathString,
					eVerticalScriptNameTextHandle);
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
			if (CtrlID == IDC_CONFIG_NOTES)
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
			if (CtrlID == IDC_STATIC_INTENSITY_TITLE_HANDLE || CtrlID == IDC_EXPERIMENT_TYPE_LABEL || CtrlID == IDC_STATIC_2_TEXT 
				|| CtrlID == IDC_STATIC_3_TEXT || CtrlID == IDC_STATIC_6_TEXT || CtrlID == IDC_STATIC_4_TEXT || CtrlID == IDC_NOTES_TEXT
				|| CtrlID == IDC_CONFIGURATION_COMBO_LABEL || CtrlID == IDC_ORIENTATION_COMBO_LABEL || CtrlID == IDC_SUB_CONFIG_COMBO_LABEL
				|| CtrlID == IDC_DEBUG_OPTION_DISPLAY_TEXT)
			{
				SetTextColor(hdcStatic, RGB(255, 255, 255));
				SetBkColor(hdcStatic, RGB(0,0,75));
				return (INT_PTR)eDarkBlueBrush;
			}
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

			else if (CtrlID == IDC_STATIC_5_TEXT)
			{
				SetTextColor(hdcStatic, RGB(255,255,255));
				SetBkColor(hdcStatic, RGB(0, 60, 80));
				return (INT_PTR)eTealBrush;
			}
			else if (CtrlID == IDC_DUMMY_NUM_TEXT || CtrlID == IDC_ACCUMULATIONS_TEXT || CtrlID == IDC_VAR_NAME_1_TEXT || CtrlID == IDC_VAR_NAME_2_TEXT 
					 || CtrlID == IDC_VAR_NAME_3_TEXT || CtrlID == IDC_VAR_NAME_4_TEXT || CtrlID == IDC_VAR_NAME_5_TEXT || CtrlID == IDC_VAR_NAME_6_TEXT 
					 || CtrlID == IDC_ORIENTATION_COMBO || CtrlID == IDC_EXPERMENT_CONFIGURATION_LIST_COMBO)
			{
				SetTextColor(hdcStatic, RGB(255, 255, 255));
				SetBkColor(hdcStatic, RGB(25, 25, 35));
				return (INT_PTR)eVeryDarkBlueBrush;
			}
			else if (CtrlID == IDC_ERROR_STATUS_TEXT_DISPLAY)
			{
				SetTextColor(hdcStatic, RGB(255, 255, 255));
				SetBkColor(hdcStatic, RGB(32, 0, 65));
				return (INT_PTR)eDarkPurpleBrush;
			}
			else if (CtrlID == IDC_GUI_STAT_TEXT || CtrlID == IDC_NIAWG_STAT_TEXT || CtrlID == IDC_AGILENT_STAT_TEXT)
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
				SetTextColor(hdcStatic, RGB(255, 255, 255));
				SetBkColor(hdcStatic, RGB(0, 60, 80));
				return (INT_PTR)eTealBrush;
			}
		}
		/// Everything Else
		case WM_COMMAND:
		{
			case IDC_CONFIG_NOTES:
			{
				if (HIWORD(wParam) == EN_CHANGE)
				{
					eExperimentSaved = false;
					SendMessage(eExperimentSavedIndicatorHandle, BM_SETCHECK, BST_UNCHECKED, NULL);
				}
			}
			switch (LOWORD(wParam))
			{
				case IDC_EXPERIMENT_COMBO:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
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
						// If the user makes a selection from the list:
						// Send CB_GETCURSEL message to get the index of the selected list item.
						long long itemIndex = SendMessage(eExperimentTypeCombo, CB_GETCURSEL, 0, 0);
						if (itemIndex == -1)
						{
							// This means that the oreintation combo was set before the Configuration list combo was set so that the configuration list combo
							// is blank. just break out, this is fine.
							break;
						}
						TCHAR experimentConfigToOpen[256];
						// Send CB_GETLBTEXT message to get the item.
						SendMessage(eExperimentTypeCombo, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)experimentConfigToOpen);
						eCurrentExperimentName = std::string(experimentConfigToOpen);
						eCurrentExperimentFolder = EXPERIMENT_CONFIGURATION_FILES_FOLDER_PATH + "\\" + std::string(experimentConfigToOpen);
						eExperimentConfigPathString = eCurrentExperimentFolder + ".experimentConfig";
						// Attempt to load experiment configuration.
						fileManage::openExperimentConfig(thisWindow, std::string(experimentConfigToOpen));

						/// Load configurations into configuration combobox.
						fileManage::reloadCombo(eCategoryCombo, eCurrentExperimentFolder, "*", "__NONE__");
						SendMessage(eConfigurationCombo, CB_RESETCONTENT, 0, 0);
						eExperimentSaved = true;
						SendMessage(eExperimentSavedIndicatorHandle, BM_SETCHECK, BST_CHECKED, NULL);
					}
					break;
				}
				case IDC_ORIENTATION_COMBO:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						long long ItemIndex = SendMessage(eOrientationCombo, CB_GETCURSEL, 0, 0);
						TCHAR orientation[256];
						SendMessage(eOrientationCombo, (UINT)CB_GETLBTEXT, (WPARAM)ItemIndex, (LPARAM)orientation);
						eCurrentOrientation = std::string(orientation);

						/// continue...
						if (!SAFEMODE)
						{
							/// Set the default accordingly
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
						if (eCurrentOrientation == "Horizontal")
						{
							if (!SAFEMODE)
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
						else if (eCurrentOrientation == "Vertical")
						{
							if (!SAFEMODE)
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
						if (!SAFEMODE)
						{
							// Initiate Generation.
							if (myNIAWG::NIAWG_CheckWindowsError(niFgen_InitiateGeneration(eSessionHandle)))
							{
								return DefWindowProc(thisWindow, msg, wParam, lParam);
							}
						}
					}
					break;
				}
				case IDC_CATEGORY_COMBO:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						// If the user makes a selection from the list:
						// Send CB_GETCURSEL message to get the index of the selected list item.
						// Send CB_GETLBTEXT message to get the item.
						long long itemIndex = SendMessage(eCategoryCombo, CB_GETCURSEL, 0, 0);
						if (itemIndex == -1)
						{
							// This means that the oreintation combo was set before the Configuration list combo was set so that the configuration list combo
							// is blank. just break out, this is fine.
							break;
						}
						TCHAR  configurationToOpen[256];
						SendMessage(eCategoryCombo, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)configurationToOpen);
						eCurrentCategoryName = configurationToOpen;
						eCurrentCategoryFolder = eCurrentExperimentFolder + "\\" + std::string(configurationToOpen);

						/// Load configurations into configuration combobox.
						// not used
						std::vector<std::string> noVec;
						std::vector<std::string> subConfigurationStrings;
						if (eCurrentOrientation == "Horizontal")
						{
							subConfigurationStrings = fileManage::searchForFiles(eCurrentCategoryFolder, "*.hSubConfig");
						}
						else if (eCurrentOrientation == "Vertical")
						{
							subConfigurationStrings = fileManage::searchForFiles(eCurrentCategoryFolder, "*.vSubConfig");
						}
						SendMessage(eConfigurationCombo, CB_RESETCONTENT, 0, 0);
						for (int configurationsInc = 0; configurationsInc < subConfigurationStrings.size(); configurationsInc++)
						{
							SendMessage(eConfigurationCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(subConfigurationStrings[configurationsInc].c_str()));
						}
						fileManage::reloadCombo(eSequenceCombo.hwnd, eCurrentCategoryFolder, "*.seq", "__NONE__");
						SendMessage(eSequenceCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"NO SEQUENCE");
						int count = SendMessage(eSequenceCombo.hwnd, CB_GETCOUNT, 0, 0);
						TCHAR sequenceText[256];
						for (int comboInc = 0; comboInc < count; comboInc++)
						{
							SendMessage(eSequenceCombo.hwnd, CB_GETLBTEXT, comboInc, (LPARAM)sequenceText);
							if (std::string(sequenceText) == "NO SEQUENCE")
							{
								SendMessage(eSequenceCombo.hwnd, CB_SETCURSEL, comboInc, 0);
								break;
							}
						}
					}
					break;
				}
				case IDC_CONFIGURATION_COMBO:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
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
						fileManage::openSubConfig(thisWindow, eCurrentConfigurationLocation);
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
					// Continue to sequence combo. This makes sure that he configuration combobox gets reset.
				}
				case IDC_SEQUENCE_COMBO:
				{
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						// get the name
						long long itemIndex = SendMessage(eSequenceCombo.hwnd, CB_GETCURSEL, 0, 0);
						TCHAR sequenceName[256];
						SendMessage(eSequenceCombo.hwnd, (UINT)CB_GETLBTEXT, (WPARAM)itemIndex, (LPARAM)sequenceName);
						if (itemIndex == -1)
						{
							// This means that the oreintation combo was set before the Configuration list combo was set so that the configuration list combo
							// is blank. just break out, this is fine.
							break;
						}
						if (std::string(sequenceName) == "NO SEQUENCE")
						{
							eCurrentSequenceName = "NO SEQUENCE";
							// only current configuration loaded
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
								// change edit
								SendMessage(eSequenceDisplay.hwnd, WM_SETTEXT, 256, (LPARAM)"Sequence of Configurations to Run:\r\n");
								appendText("1. " + eSequenceFileNames[0] + "\r\n", IDC_SEQUENCE_DISPLAY, eMainWindowHandle);
							}
							else
							{
								SendMessage(eSequenceDisplay.hwnd, WM_SETTEXT, 256, (LPARAM)"Sequence of Configurations to Run:\r\n");
								appendText("No Configuration Loaded\r\n", IDC_SEQUENCE_DISPLAY, eMainWindowHandle);
							}

							break;
						}
						// try to open the file
						std::fstream sequenceFile(eCurrentCategoryFolder + "\\" + sequenceName + ".seq");
						if (!sequenceFile.is_open())
						{
							MessageBox(0, ("ERROR: sequence file failed to open! Make sure the sequence with address" + eCurrentCategoryFolder + "\\" + sequenceName + ".seq exists.").c_str(), 0, 0);
							return -1;
						}
						eCurrentSequenceName = std::string(sequenceName);
						// read the file
						eSequenceFileNames.clear();
						std::string tempName;
						getline(sequenceFile, tempName);
						while (sequenceFile)
						{
							eSequenceFileNames.push_back(tempName);
							getline(sequenceFile, tempName);
						}
						// update the edit
						SendMessage(eSequenceDisplay.hwnd, WM_SETTEXT, 256, (LPARAM)"Configuration Sequence:\r\n");
						for (int sequenceInc = 0; sequenceInc < eSequenceFileNames.size(); sequenceInc++)
						{
							appendText(std::to_string(sequenceInc + 1) + ". " + eSequenceFileNames[sequenceInc] + "\r\n", IDC_SEQUENCE_DISPLAY, eMainWindowHandle);
						}
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
					// finish the abort.
					menuAndAcceleratorFunctions::abortSystem(thisWindow);
					break;
				}
				/// Status Controls
				case IDC_SCRIPT_STATUS_CLEAR_BUTTON:
				{
					// send blank message
					SendMessage(eSystemStatusTextHandle, WM_SETTEXT, NULL, (LPARAM)"*********************\r\n");
					break;
				}
				case IDC_SCRIPT_DEBUG_CLEAR_BUTTON:
				{
					// send blank message
					SendMessage(eSystemDebugTextHandle, WM_SETTEXT, NULL, (LPARAM)"**********************\r\n");
					break;
				}
				case IDC_SCRIPT_ERROR_CLEAR_BUTTON:
				{
					// send blank message
					SendMessage(eSystemErrorTextHandle, WM_SETTEXT, NULL, (LPARAM)"**********************\r\n");
					break;
				}
				/// Buttons
				case IDC_DUMMY_NUM_BUTTON:
				{
					char tempChar[5];
					int charNum = GetWindowText(eDummyNumEditHandle, &tempChar[0], 5);
					if (charNum == 0)
					{
						char errMsg[100];
						sprintf_s(errMsg, "ERROR: couldn't read dummy variable number. value in edit control is %s", &tempChar[0]);
						MessageBox(NULL, errMsg, "ERROR", MB_OK);
						break;
					}
					bool digitCheck = true;
					for (int dumNumInc = 0; dumNumInc < charNum; dumNumInc++)
					{
						if (!isdigit(tempChar[dumNumInc]))
						{
							MessageBox(NULL, "Please enter a number.", "NULL", MB_OK);
							digitCheck = false;
							break;
						}
					}
					if (digitCheck == false)
					{
						break;
					}
					eDummyNum = boost::lexical_cast<int>(tempChar);

					SetWindowText(eDummyNumTextHandle, &tempChar[0]);
					eExperimentSaved = false;
					SendMessage(eExperimentSavedIndicatorHandle, BM_SETCHECK, BST_UNCHECKED, NULL);
					break;
				}
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
					eExperimentSaved = false;
					SendMessage(eExperimentSavedIndicatorHandle, BM_SETCHECK, BST_UNCHECKED, NULL);
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
					eExperimentSaved = false;
					SendMessage(eExperimentSavedIndicatorHandle, BM_SETCHECK, BST_UNCHECKED, NULL);
					break;
				}
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
					eExperimentSaved = false;
					SendMessage(eExperimentSavedIndicatorHandle, BM_SETCHECK, BST_UNCHECKED, NULL);
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
					eExperimentSaved = false;
					SendMessage(eExperimentSavedIndicatorHandle, BM_SETCHECK, BST_UNCHECKED, NULL);
					break;
				}
				case IDC_OUTPUT_MORE_RUN_INFO:
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
					eExperimentSaved = false;
					SendMessage(eExperimentSavedIndicatorHandle, BM_SETCHECK, BST_UNCHECKED, NULL);
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
					eExperimentSaved = false;
					SendMessage(eExperimentSavedIndicatorHandle, BM_SETCHECK, BST_UNCHECKED, NULL);
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
					eExperimentSaved = false;
					SendMessage(eExperimentSavedIndicatorHandle, BM_SETCHECK, BST_UNCHECKED, NULL);
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
					eExperimentSaved = false;
					SendMessage(eExperimentSavedIndicatorHandle, BM_SETCHECK, BST_UNCHECKED, NULL);
					break;
				}
				case IDC_OUTPUT_CORR_TIME_BUTTON:
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
					eExperimentSaved = false;
					SendMessage(eExperimentSavedIndicatorHandle, BM_SETCHECK, BST_UNCHECKED, NULL);
					break;
				}
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
					eExperimentSaved = false;
					SendMessage(eExperimentSavedIndicatorHandle, BM_SETCHECK, BST_UNCHECKED, NULL);
					break;
				}
				/// File Management 
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

				case ID_SEQUENCE_ADD_TO_SEQUENCE:
				{
					menuAndAcceleratorFunctions::addToSequence(thisWindow);
					break;
				}
				case ID_SEQUENCE_SAVE_SEQUENCE:
				{
					menuAndAcceleratorFunctions::saveSequence(thisWindow);
					break;
				}
				case ID_SEQUENCE_NEW_SEQUENCE:
				{
					menuAndAcceleratorFunctions::newSequence(thisWindow);
					break;
				}
				case ID_SEQUENCE_RESET_SEQUENCE:
				{
					menuAndAcceleratorFunctions::resetSequence(thisWindow);
					break;
				}
				case ID_SEQUENCE_DELETE_SEQUENCE:
				{
					menuAndAcceleratorFunctions::deleteSequence(thisWindow);
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
				if (!SAFEMODE)
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
				if (eCurrentOrientation == "Horizontal")
				{
					if (!SAFEMODE)
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
				else if (eCurrentOrientation == "Vertical")
				{
					if (!SAFEMODE)
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
				if (!SAFEMODE)
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
				if (!SAFEMODE)
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
				if (eCurrentOrientation == "Horizontal")
				{
					if (!SAFEMODE)
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
				else if (eCurrentOrientation == "Vertical")
				{
					if (!SAFEMODE)
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
				if (!SAFEMODE)
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
