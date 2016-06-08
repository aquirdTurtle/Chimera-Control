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

extern ViStatus eError;
extern bool eWaitError;

extern ConfigurationFileSystem eProfile;
extern NoteSystem eNotes;
extern VariableSystem eVariables;

// Default NIAWG Wavefunction Info.
extern ViReal64* eDefault_hConfigMixedWaveform;
extern std::string eDefault_hConfigWaveformName;
extern long eDefault_hConfigMixedSize;
extern ViChar* eDefault_hConfigScript;
extern ViReal64* eDefault_vConfigMixedWaveform;
extern std::string eDefault_vConfigWaveformName;
extern long eDefault_vConfigMixedSize;
extern ViChar* eDefault_vConfigScript;
// asdfasdfasdfasdf
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
extern std::string eVerticalParentScriptPathString;
extern std::string eVerticalViewScriptPathString;
extern std::string eHorizontalParentScriptPathString;
extern std::string eHorizontalViewScriptPathString;
extern std::string eIntensityParentScriptPathString;
extern std::string eIntensityViewScriptPathString;
extern std::string eMostRecentVerticalScriptNames;
extern std::string eMostRecentHorizontalScriptNames;
extern std::string eMostRecentIntensityScriptNames;
//
//extern std::vector<std::string> eVariableNames;
//extern std::vector<std::string> eVerticalVarFileNames;
extern std::string eGenStatusColor;


/// Global Options Variables
extern char eVerticalCurrentParentScriptName[_MAX_FNAME];
extern char eVerticalCurrentViewScriptName[_MAX_FNAME];
extern char eHorizontalCurrentParentScriptName[_MAX_FNAME];
extern char eHorizontalCurrentViewScriptName[_MAX_FNAME];
extern char eIntensityCurrentParentScriptName[_MAX_FNAME];
extern char eIntensityCurrentViewScriptName[_MAX_FNAME];


// A variable which records what script is currently being executed by the NIAWG.
extern std::string eCurrentScript;

extern unsigned int eAccumulations;

extern unsigned int eGreenMessageID, eStatusTextMessageID, eErrorTextMessageID, 
					eFatalErrorMessageID, eVariableStatusMessageID, eNormalFinishMessageID, eColoredEditMessageID;

extern bool eCurrentVerticalViewIsParent;
extern bool eCurrentHorizontalViewIsParent;
extern bool eCurrentIntensityViewIsParent;


extern bool eSystemIsRunning;
extern bool eVerticalScriptSaved;
extern bool eHorizontalScriptSaved;
extern bool eIntensityScriptSaved;
extern bool eDontActuallyGenerate;
extern bool eConnectToMaster;
extern bool eGetVarFilesFromMaster;
extern bool eOutputReadStatus;
extern bool eOutputWriteStatus;
extern bool eLogScriptAndParams;
extern bool eUseDummyVariables;
extern bool eOutputCorrTime;
extern bool eProgramIntensityOption;
extern bool eSyntaxTimerIsActive;
extern bool eAbortSystemFlag;
extern bool eOutputRunInfo;

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
// Titles and Static Text Handles
extern HWND eExperimentTypeLabelHandle2;
extern HWND eErrorStatusTextDisplay;
extern HWND eDebuggingOptionsDisplayHandle;
extern HWND eExperimentStatusTextHandle;
extern HWND eDebugStatusTextHandle;
extern HWND eAccumulationsTextHandle;
extern HWND eSystemStatusTextHandle;
extern HWND eSystemErrorTextHandle;
extern HWND eVariablesLabelTextHandle;
extern HWND eSetupParametersTextHandle;
extern HWND eNotesLabelTextHandle;

// User-edited edit handles
extern HWND eAccumulationsEditHandle;
extern HWND eVariableNamesEditHandle;
extern HWND eSystemDebugTextHandle;
// Code-Dedited Edit Handles
extern HWND eVar1NameTextHandle;
extern HWND eVar2NameTextHandle;
extern HWND eVar3NameTextHandle;
extern HWND eVar4NameTextHandle;
extern HWND eVar5NameTextHandle;
extern HWND eVar6NameTextHandle;
extern HWND eExperimentTypeTextHandle2;
extern HWND eColoredStatusEdit;
// Checked Box Handles
extern HWND eOutputReadStatusButton;
extern HWND eOutputWriteStatusButton;
extern HWND eProgramIntensityOptionButton;

// Button Handles
extern HWND eScriptErrorClearButtonHandle;
extern HWND eAccumulationsButtonHandle;
extern HWND eScriptStatusClearButtonHandle;
extern HWND eScriptDebugClearButtonHandle;
extern HWND eLoadConfigButtonHandle;
// User-Edited Check-Boxes
extern HWND eConnectToMasterHandle;
extern HWND eReceiveVariableFiles;
extern HWND eLogScriptAndParamsButton;
extern HWND eDummyVariableSelectHandle;
extern HWND eDummyNumEditHandle;
extern HWND eDummyNumTextHandle;
extern HWND eDummyVariableButtonHandle;
extern HWND eVariableSetButtonHandle;
extern HWND eOutputCorrTimeButton;
extern HWND eOutputMoreInfoCheckButton;

/// Global API Handles (Scripting Window)
// main
extern HWND eScriptingWindowHandle;

// Titles and Static Text Handles
extern HWND eStaticIntensityTitleHandle;
extern HWND eIntensityNameHandle;
extern HWND eStaticVerticalEditHandle;
extern HWND eStaticHorizontalEditHandle;
extern HWND eConfigurationDisplayInScripting;

// User-Edited Edit Handles
extern HWND eIntensityScriptEditHandle;
extern HWND eVerticalScriptEditHandle;
extern HWND eHorizontalScriptEditHandle;
// Code-Edited Edit Handles
extern HWND eVerticalScriptNameTextHandle;
extern HWND eHorizontalScriptNameTextHandle;
extern HWND eConfigurationTextInScripting;

// Code-Edited Checked Boxes
extern HWND eVerticalScriptSavedIndicatorHandle;
extern HWND eHorizontalScriptSavedIndicatorHandle;
extern HWND eIntensityScriptSavedIndicatorHandle;
// Other
extern HWND eColorBox;

/// Beginning Settings Dialog
extern HWND eBeginDialogRichEdit;

/// Temp!
/// view Combos & texts
extern Control eHorizontalViewCombo, eVerticalViewCombo, eIntensityViewCombo, eHorizontalViewText, eVerticalViewText,
			   eIntensityViewText;
extern Control eSequenceText, eSequenceCombo, eSequenceDisplay;