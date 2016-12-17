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

#include <sstream>
#include <fstream>

#include "boost/lexical_cast.hpp"
#include <boost/algorithm/string.hpp>
#include "commonMessages.h"
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
/*
LRESULT CALLBACK winProcScripts(HWND thisWindow, UINT msg, WPARAM wParam, LPARAM lParam)
{
	/// Handle incoming messages.
	switch (msg)
	{
		case WM_CREATE: 
		{
			initializeScriptingWindow(thisWindow);
			horizontalNIAWGScript.updateSavedStatus(true);
			verticalNIAWGScript.updateSavedStatus(true);
			intensityAgilentScript.updateSavedStatus(true);
			break;
		}
		/// Hanlde Colors
		case WM_CTLCOLOREDIT:
		{
			// not sure if this edit-coloring section is necessary. Pretty sure rich edits are entirely uneffected by this stuff.
			int initialID, finalID;
			DWORD CtrlID = GetDlgCtrlID((HWND)lParam);
			// first check all of the script controls
			eVerticalNIAWGScript.getControlIDRange(initialID, finalID);
			if (initialID <= CtrlID && CtrlID <= finalID)
			{
				return eVerticalNIAWGScript.colorControl(lParam, wParam);
			}
			eHorizontalNIAWGScript.getControlIDRange(initialID, finalID);
			if (initialID <= CtrlID && CtrlID <= finalID)
			{
				return eHorizontalNIAWGScript.colorControl(lParam, wParam);
			}
			eIntensityAgilentScript.getControlIDRange(initialID, finalID);
			if (initialID <= CtrlID && CtrlID <= finalID)
			{
				return eIntensityAgilentScript.colorControl(lParam, wParam);
			}
			HDC hdcStatic = (HDC)wParam;
			if (CtrlID == IDC_HORIZONTAL_SCRIPT_EDIT || CtrlID == IDC_VERTICAL_SCRIPT_EDIT)
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
			int initialID, finalID;
			DWORD CtrlID = GetDlgCtrlID((HWND)lParam); 
			// first check all of the script controls
			eVerticalNIAWGScript.getControlIDRange(initialID, finalID);
			if (initialID <= CtrlID && CtrlID <= finalID)
			{
				return eVerticalNIAWGScript.colorControl(lParam, wParam);
				
			}
			eHorizontalNIAWGScript.getControlIDRange(initialID, finalID);
			if (initialID <= CtrlID && CtrlID <= finalID)
			{
				return eHorizontalNIAWGScript.colorControl(lParam, wParam);
				
			}
			eIntensityAgilentScript.getControlIDRange(initialID, finalID);
			if (initialID <= CtrlID && CtrlID <= finalID)
			{
				return eIntensityAgilentScript.colorControl(lParam, wParam);
				
			}
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
			if (eProfile.checkSaveEntireProfile())
			{
				break;
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
			if (eVerticalNIAWGScript.coloringIsNeeded())
			{
				eVerticalNIAWGScript.handleTimerCall();
			}
			if (eHorizontalNIAWGScript.coloringIsNeeded())
			{
				eHorizontalNIAWGScript.handleTimerCall();
			}
			if (eIntensityAgilentScript.coloringIsNeeded())
			{
				eIntensityAgilentScript.handleTimerCall();
			}
			break;
		}
		/// Handle Everything Else...///////////////////////
		case WM_COMMAND:
		{
			if (!eVerticalNIAWGScript.handleEditChange(wParam, lParam))
			{
				break;
			}
			else if (!eVerticalNIAWGScript.childComboChangeHandler(wParam, lParam))
			{
				break;
			}
			else if (!eHorizontalNIAWGScript.handleEditChange(wParam, lParam))
			{
				break;
			}
			else if (!eHorizontalNIAWGScript.childComboChangeHandler(wParam, lParam))
			{
				break;
			}
			else if (!eIntensityAgilentScript.handleEditChange(wParam, lParam))
			{
				break;
			}
			else if (!eIntensityAgilentScript.childComboChangeHandler(wParam, lParam))
			{
				break;
			}
			switch (LOWORD(wParam))
			{
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
					commonMessages::handleCommonMessage(thisWindow, msg, wParam, lParam);
					break;
				}
			}
			break;
		}
	}

	return DefWindowProc(thisWindow, msg, wParam, lParam);
}
*/