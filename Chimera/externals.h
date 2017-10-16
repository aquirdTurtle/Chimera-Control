#pragma once
// This header file includes all of the extern declarations of all of my external (global) variables.
#include "ProfileSystem.h"
#include "NoteSystem.h"
#include "VariableSystem.h"
#include "Script.h"
#include "MainWindow.h"
#include "ScriptingWindow.h"
#include "DebugOptionsControl.h"
#include "NiawgController.h"
#include "Control.h"
#include "Windows.h"
#include <string>
#include <vector>
#include "niFgen.h"
extern std::vector<std::string> WAVEFORM_NAME_FILES;
extern std::vector<std::string> WAVEFORM_TYPE_FOLDERS;

extern HWND eMainWindowHwnd;

/// some globals for niawg stuff, only for niawg stuff so I keep it here...?
extern const std::array<int, 2> AXES;
// the following is used to receive the index of whatever axis is not your current axis.
extern const std::array<int, 2> ALT_AXES;
extern const std::array<std::string, 2> AXES_NAMES;
extern const niawgPair<std::string> ORIENTATION;


extern bool eWaitError;

// stuff for syntax coloring

/// Global Options Variables
extern UINT eStatusTextMessageID, eErrorTextMessageID, eFatalErrorMessageID, eNormalFinishMessageID, 
			eColoredEditMessageID, eDebugMessageID, eCameraFinishMessageID, eCameraProgressMessageID, 
			eRepProgressMessageID, eNoAtomsAlertMessageID;

extern bool eAbortNiawgFlag;

/// Colors
extern HBRUSH eDullRedBrush;
extern HBRUSH eDarkRedBrush;
extern HBRUSH eGreyRedBrush;

/// Other Global Handles
extern HANDLE eWaitingForNIAWGEvent;
extern HANDLE eExperimentThreadHandle;
extern HANDLE eNIAWGWaitThreadHandle;

/// Beginning Settings Dialog
extern HWND eBeginDialogRichEdit;

