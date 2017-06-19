// This file includes the decalarations of all of my external (global) variables. Declaring them here is a nice way of including all of the extenals in a file
// without making it look like a normal variable.
#include "stdafx.h"
#include "externals.h"
#include <string>
#include <vector>
#include "Windows.h"
#include "constants.h"
#include "ConfigurationFileSystem.h"
#include "VariableSystem.h"
#include "DebuggingOptionsControl.h"

int scriptIDs = 110000;
int debugID = 111000;
std::vector<std::string> WAVEFORM_NAME_FILES = std::vector<std::string>(4 * MAX_NIAWG_SIGNALS);
std::vector<std::string> WAVEFORM_TYPE_FOLDERS = std::vector<std::string>(4 * MAX_NIAWG_SIGNALS);

/// some globals for niawg stuff, only for niawg stuff so I keep it here...?
const std::array<int, 2> AXES = { Vertical, Horizontal };
// the following is used to receive the index of whatever axis is not your current axis.
const std::array<int, 2> ALT_AXES = { Horizontal, Vertical };
const std::array<std::string, 2> AXES_NAMES = { "Vertical", "Horizontal" };
const niawgPair<std::string> ORIENTATION = { VERTICAL_ORIENTATION, HORIZONTAL_ORIENTATION };

// Agilent Stuff
double eCurrentAgilentLow = std::stod(AGILENT_DEFAULT_DC);
double eCurrentAgilentHigh = std::stod(AGILENT_DEFAULT_DC);

bool eWaitError = false;

DWORD eIntensityMinChange = ULONG_MAX, eIntensityMaxChange = 0;
DWORD eHorizontalMinChange = ULONG_MAX, eHorizontalMaxChange = 0;
DWORD eVerticalMinChange = ULONG_MAX, eVerticalMaxChange = 0;
//
HINSTANCE eGlobalInstance;

std::string eExperimentConfigPathString;
std::string eVerticalParentScriptPathString;
std::string eVerticalViewScriptPathString;
std::string eHorizontalParentScriptPathString;
std::string eHorizontalViewScriptPathString;
std::string eIntensityParentScriptPathString;
std::string eIntensityViewScriptPathString;
std::string eMostRecentVerticalScriptNames;
std::string eMostRecentHorizontalScriptNames;
std::string eMostRecentIntensityScriptNames;
std::vector<std::string> eSequenceFileNames;


bool eAbortNiawgFlag = false;

// thread messages
// register messages for main window.
UINT eVariableStatusMessageID = RegisterWindowMessage( "ID_THREAD_VARIABLE_STATUS" );
UINT eGreenMessageID = RegisterWindowMessage( "ID_THREAD_GUI_GREEN" );
UINT eStatusTextMessageID = RegisterWindowMessage( "ID_THREAD_STATUS_MESSAGE" );
UINT eDebugMessageID = RegisterWindowMessage( "ID_THREAD_DEBUG_MESSAGE" );
UINT eErrorTextMessageID = RegisterWindowMessage( "ID_THREAD_ERROR_MESSAGE" );
UINT eFatalErrorMessageID = RegisterWindowMessage( "ID_THREAD_FATAL_ERROR_MESSAGE" );
UINT eNormalFinishMessageID = RegisterWindowMessage( "ID_THREAD_NORMAL_FINISH_MESSAGE" );
UINT eColoredEditMessageID = RegisterWindowMessage( "ID_VARIABLE_VALUES_MESSAGE" );
UINT eCameraFinishMessageID = RegisterWindowMessage( "ID_CAMERA_FINISH_MESSAGE" );
UINT eCameraProgressMessageID = RegisterWindowMessage( "ID_CAMERA_PROGRESS_MESSAGE" );


// A variable which records what script is currently being executed by the NIAWG.
std::string eCurrentScript;

// Colors
HBRUSH eDullRedBrush = CreateSolidBrush(RGB(25, 0, 0));
HBRUSH eDarkRedBrush = CreateSolidBrush(RGB(75, 0, 0));
HBRUSH eGreyRedBrush = CreateSolidBrush(RGB(50, 45, 45));

HANDLE eWaitingForNIAWGEvent;
HANDLE eExperimentThreadHandle;
HANDLE eNIAWGWaitThreadHandle;

/// Beginning Settings Dialog
HWND eBeginDialogRichEdit;
