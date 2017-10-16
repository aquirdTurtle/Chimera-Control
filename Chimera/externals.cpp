// This file includes the decalarations of all of my external (global) variables. Declaring them here is a nice way of including all of the extenals in a file
// without making it look like a normal variable.
#include "stdafx.h"
#include "externals.h"
#include <string>
#include <vector>
#include "Windows.h"
#include "constants.h"
#include "ProfileSystem.h"
#include "VariableSystem.h"
#include "DebugOptionsControl.h"

std::vector<std::string> WAVEFORM_NAME_FILES = std::vector<std::string>(4 * MAX_NIAWG_SIGNALS);
std::vector<std::string> WAVEFORM_TYPE_FOLDERS = std::vector<std::string>(4 * MAX_NIAWG_SIGNALS);

HWND eMainWindowHwnd;

/// some globals for niawg stuff, only for niawg stuff so I keep it here...?
const std::array<int, 2> AXES = { Vertical, Horizontal };
// the following is used to receive the index of whatever axis is not your current axis.
const std::array<int, 2> ALT_AXES = { Horizontal, Vertical };
const std::array<std::string, 2> AXES_NAMES = { "Vertical", "Horizontal" };

bool eWaitError = false;
bool eAbortNiawgFlag = false;

// thread messages
// register messages for main window.
UINT eStatusTextMessageID = RegisterWindowMessage("ID_THREAD_STATUS_MESSAGE");
UINT eDebugMessageID = RegisterWindowMessage( "ID_THREAD_DEBUG_MESSAGE" );
UINT eErrorTextMessageID = RegisterWindowMessage( "ID_THREAD_ERROR_MESSAGE" );
UINT eFatalErrorMessageID = RegisterWindowMessage( "ID_THREAD_FATAL_ERROR_MESSAGE" );
UINT eNormalFinishMessageID = RegisterWindowMessage( "ID_THREAD_NORMAL_FINISH_MESSAGE" );
UINT eColoredEditMessageID = RegisterWindowMessage( "ID_VARIABLE_VALUES_MESSAGE" );
UINT eCameraFinishMessageID = RegisterWindowMessage( "ID_CAMERA_FINISH_MESSAGE" );
UINT eCameraProgressMessageID = RegisterWindowMessage( "ID_CAMERA_PROGRESS_MESSAGE" );
UINT eRepProgressMessageID = RegisterWindowMessage("ID_REPETITION_PROGRESS_MESSAGE");
UINT eNoAtomsAlertMessageID = RegisterWindowMessage( "ID_NO_ATOMS_ALERT_MESSAGE" );

HANDLE eWaitingForNIAWGEvent;
HANDLE eNIAWGWaitThreadHandle;

/// Beginning Settings Dialog
HWND eBeginDialogRichEdit;
