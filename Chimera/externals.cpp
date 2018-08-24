// This file includes the decalarations of all of my external (global) variables. Declaring them here is a nice way of including all of the extenals in a file
// without making it look like a normal variable.
#include "stdafx.h"
#include "externals.h"
#include <string>
#include <vector>
#include "Windows.h"
#include "constants.h"
#include "ProfileSystem.h"
#include "ParameterSystem.h"
#include "DebugOptionsControl.h"

std::vector<std::string> WAVEFORM_NAME_FILES = std::vector<std::string>(4 * MAX_NIAWG_SIGNALS);
std::vector<std::string> WAVEFORM_TYPE_FOLDERS = std::vector<std::string>(4 * MAX_NIAWG_SIGNALS);

HWND eMainWindowHwnd;

/// some globals for niawg stuff, only for niawg stuff so I keep it here...?
const std::array<int, 2> AXES = { Axes::Vertical, Axes::Horizontal };
// the following is used to receive the index of whatever axis is not your current axis.
const std::array<int, 2> ALT_AXES = { Axes::Horizontal, Axes::Vertical };
const std::array<std::string, 2> AXES_NAMES = { "Vertical", "Horizontal" };

bool eWaitError = false;
bool eAbortNiawgFlag = false;

// thread messages
// register messages for main window.
UINT eMachineOptRoundFinMsgID = RegisterWindowMessage ( "ID_MACHINE_OPT_ROUND_FIN_MSG" );
UINT eMotNumCalFinMsgID = RegisterWindowMessage("ID_MOT_CAL_FIN_MSG" );
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
UINT eLogVoltsMessageID = RegisterWindowMessage( "ID_LOG_VOLTS_MESSAGE" );
UINT eAutoServoMessage = RegisterWindowMessage( "ID_AUTO_SERVO_MESSAGE" );
UINT eCameraCalProgMessageID = RegisterWindowMessage( "ID_CAMERA_CAL_PROGRESS_MESSAGE" );
UINT eCameraCalFinMessageID = RegisterWindowMessage( "ID_CAMERA_CAL_FIN_MESSAGE" );
UINT ACE_PIC_READY = RegisterWindowMessage ( "ACE_PIC_READY" );
UINT eBaslerFinMessageId = RegisterWindowMessage ( "ID_BASLER_FINISH_MESSAGE" );
UINT eNoMotAlertMessageID = RegisterWindowMessage ( "ID_NO_MOT_ALERT" );

HANDLE eWaitingForNIAWGEvent;
HANDLE eNIAWGWaitThreadHandle;

