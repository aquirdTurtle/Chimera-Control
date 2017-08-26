#pragma once
// This header file includes all of the extern declarations of all of my external (global) variables.
#include "Windows.h"
#include <string>
#include <vector>
#include "niFgen.h"
#include "Control.h"
#include "ProfileSystem.h"
#include "NoteSystem.h"
#include "VariableSystem.h"
#include "Script.h"
#include "GUI_Debugger.h"
#include "MainWindow.h"
#include "ScriptingWindow.h"
#include "DebuggingOptionsControl.h"
#include "NiawgController.h"

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
extern DWORD eIntensityMinChange, eIntensityMaxChange;
extern DWORD eHorizontalMinChange, eHorizontalMaxChange;
extern DWORD eVerticalMinChange, eVerticalMaxChange;


/// Global Options Variables
extern UINT eGreenMessageID, eStatusTextMessageID, eErrorTextMessageID, eFatalErrorMessageID, eVariableStatusMessageID,
			eNormalFinishMessageID, eColoredEditMessageID, eDebugMessageID, eCameraFinishMessageID, 
			eCameraProgressMessageID, eRepProgressMessageID;

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

