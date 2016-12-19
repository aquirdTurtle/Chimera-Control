#pragma once
// This header file includes all of the extern declarations of all of my external (global) variables.
#include "Windows.h"
#include <string>
#include <vector>
#include "niFgen.h"
#include "Control.h"
#include "ConfigurationFileSystem.h"
#include "NoteSystem.h"
#include "VariableSystem.h"
#include "Script.h"
#include "GUI_Debugger.h"
#include "MainWindow.h"
#include "ScriptingWindow.h"
#include "DebuggingOptionsControl.h"

//extern UINT REDRAW_INDICATORS;

extern CFont eNormalFont;
extern CFont eSmallFont;
extern CFont eCodeFont;
extern CFont eCodeFont;
extern CFont eSmallCodeFont;
extern CFont eHeadingFont;
extern CFont eLargeHeadingFont;

extern ViStatus eError;
extern bool eWaitError;
//extern NoteSystem eNotes;
//extern VariableSystem eVariables;
//extern Script eVerticalNIAWGScript, eHorizontalNIAWGScript, eIntensityAgilentScript;

// Default NIAWG Wavefunction Info.
extern ViReal64* eDefault_hConfigMixedWaveform;
extern std::string eDefault_hConfigWaveformName;
extern long eDefault_hConfigMixedSize;
extern ViChar* eDefault_hConfigScript;
extern ViReal64* eDefault_vConfigMixedWaveform;
extern std::string eDefault_vConfigWaveformName;
extern long eDefault_vConfigMixedSize;
extern ViChar* eDefault_vConfigScript;

extern HINSTANCE eGlobalInstance;
// stuff for syntax coloring
extern DWORD eIntensityMinChange, eIntensityMaxChange;
extern DWORD eHorizontalMinChange, eHorizontalMaxChange;
extern DWORD eVerticalMinChange, eVerticalMaxChange;

// Global parameters
// Agilent things
extern double eCurrentAgilentLow;
extern double eCurrentAgilentHigh;

extern std::string eCurrentFolderDialogType;

// These Describe the scripts

extern std::string eMostRecentVerticalScriptNames;
extern std::string eMostRecentHorizontalScriptNames;
extern std::string eMostRecentIntensityScriptNames;
/// Global Options Variables

// A variable which records what script is currently being executed by the NIAWG.
extern std::string eCurrentScript;

extern unsigned int eAccumulations;

extern UINT eGreenMessageID, eStatusTextMessageID, eErrorTextMessageID, 
					eFatalErrorMessageID, eVariableStatusMessageID, eNormalFinishMessageID, eColoredEditMessageID, 
					eDebugMessageID;

extern bool eCurrentVerticalViewIsParent;
extern bool eCurrentHorizontalViewIsParent;
extern bool eCurrentIntensityViewIsParent;

extern bool eExperimentIsRunning;
extern bool eDontActuallyGenerate;
extern bool eSyntaxTimerIsActive;
extern bool eAbortSystemFlag;

extern bool eHorizontalSyntaxColorIsCurrent, eVerticalSyntaxColorIsCurrent, eIntensitySyntaxColorIsCurrent;

extern ViSession eSessionHandle;

/// Colors

extern HBRUSH eYellowBrush;
extern HBRUSH eGreenBrush;
extern HBRUSH blueBrush;
extern HBRUSH eRedBrush;
extern HBRUSH eDarkBlueBrush;
extern HBRUSH eTealBrush;
extern HBRUSH eVeryDarkBlueBrush;
extern HBRUSH eNearBlackBlueBrush;
extern HBRUSH eDarkPurpleBrush;
extern HBRUSH eDullRedBrush;
extern HBRUSH eDarkRedBrush;
extern HBRUSH eNearBlackRedBrush;
extern HBRUSH eGreyRedBrush;

/// Other Global Handles
extern HANDLE eWaitingForNIAWGEvent;
extern HANDLE eExperimentThreadHandle;
extern HANDLE eNIAWGWaitThreadHandle;

/// Global API Handles (Main Window)
// main
extern HWND eMainWindowHandle;
/// Global API Handles (Scripting Window)
// main
extern HWND eScriptingWindowHandle;
/// Beginning Settings Dialog
extern HWND eBeginDialogRichEdit;

/// Temp!
/// view Combos & texts


