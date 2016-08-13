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

ConfigurationFileSystem eProfile(PROFILES_PATH);
NoteSystem eNotes;
VariableSystem eVariables;
int scriptIDs = 110000;
Script eVerticalNIAWGScript("Vertical NIAWG", scriptIDs), eHorizontalNIAWGScript("Horizontal NIAWG", scriptIDs), eIntensityAgilentScript("Agilent", scriptIDs);

// Agilent Stuff
double eCurrentAgilentLow = std::stod(AGILENT_DEFAULT_DC);
double eCurrentAgilentHigh = std::stod(AGILENT_DEFAULT_DC);

bool eWaitError = false;

DWORD eIntensityMinChange = ULONG_MAX, eIntensityMaxChange = 0;
DWORD eHorizontalMinChange = ULONG_MAX, eHorizontalMaxChange = 0;
DWORD eVerticalMinChange = ULONG_MAX, eVerticalMaxChange = 0;
// Default NIAWG Wavefunction Info.
ViReal64* eDefault_hConfigMixedWaveform;
std::string eDefault_hConfigWaveformName;
long eDefault_hConfigMixedSize;
ViChar* eDefault_hConfigScript;

ViReal64* eDefault_vConfigMixedWaveform;
std::string eDefault_vConfigWaveformName;
long eDefault_vConfigMixedSize;
ViChar* eDefault_vConfigScript;
//
HINSTANCE eGlobalInstance;
ViStatus eError = VI_SUCCESS;

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
//std::vector<std::string> eVariableNames;
//std::vector<std::string> eVerticalVarFileNames;

bool eCurrentVerticalViewIsParent = true;
bool eCurrentHorizontalViewIsParent = true;
bool eCurrentIntensityViewIsParent = true;

bool eSystemIsRunning;
bool eVerticalScriptSaved;
bool eHorizontalScriptSaved;
bool eIntensityScriptSaved;
bool eDontActuallyGenerate;
bool eConnectToMaster;
bool eGetVarFilesFromMaster;
bool eLogScriptAndParams;
bool eUseDummyVariables;
bool eOutputCorrTime;
bool eProgramIntensityOption;
bool eOutputReadStatus;
bool eOutputWriteStatus;
bool eSyntaxTimerIsActive;
bool eAbortSystemFlag = false;
bool eHorizontalSyntaxColorIsCurrent = true, eVerticalSyntaxColorIsCurrent = true, eIntensitySyntaxColorIsCurrent = true;
bool eOutputRunInfo = false;
int eDummyNum;

unsigned int eAccumulations = 0;
// thread messages
unsigned int eGreenMessageID, eStatusTextMessageID, eErrorTextMessageID, eFatalErrorMessageID, eVariableStatusMessageID, eNormalFinishMessageID,
			 eColoredEditMessageID;

char eVerticalCurrentParentScriptName[_MAX_FNAME];
char eHorizontalCurrentParentScriptName[_MAX_FNAME];
char eIntensityCurrentParentScriptName[_MAX_FNAME];
char eVerticalCurrentViewScriptName[_MAX_FNAME];
char eHorizontalCurrentViewScriptName[_MAX_FNAME];
char eIntensityCurrentViewScriptName[_MAX_FNAME];

std::string eGenStatusColor;
// A variable which records what script is currently being executed by the NIAWG.
std::string eCurrentScript;

ViSession eSessionHandle;

HWND mainToolTipClass;
HWND tooltipTest;
// Colors
HBRUSH eYellowBrush = CreateSolidBrush(RGB(104, 104, 0));
HBRUSH eGreenBrush = CreateSolidBrush(RGB(0, 120, 0));
HBRUSH blueBrush = CreateSolidBrush(RGB(0, 0, 120));
HBRUSH eRedBrush = CreateSolidBrush(RGB(120, 0, 0));
HBRUSH eDarkBlueBrush = CreateSolidBrush(RGB(0, 0, 75));
HBRUSH eTealBrush = CreateSolidBrush(RGB(0, 60, 80));
HBRUSH eVeryDarkBlueBrush = CreateSolidBrush(RGB(25, 25, 35));
HBRUSH eNearBlackBlueBrush = CreateSolidBrush(RGB(15, 15, 20));
HBRUSH eDarkPurpleBrush = CreateSolidBrush(RGB(32, 0, 65));
HBRUSH eDullRedBrush = CreateSolidBrush(RGB(25, 0, 0));
HBRUSH eDarkRedBrush = CreateSolidBrush(RGB(75, 0, 0));
HBRUSH eNearBlackRedBrush = CreateSolidBrush(RGB(30, 25, 25));
HBRUSH eGreyRedBrush = CreateSolidBrush(RGB(50, 45, 45));

/// Objects for drawings
RECT eRampCanvasRECT;
HDC eRampsRectangleHDC;

HANDLE eWaitingForNIAWGEvent;
HANDLE eExperimentThreadHandle;
HANDLE eNIAWGWaitThreadHandle;

/// //////////////////////////////////////////////////////////////////////////////////////////
///
///			Global API Handle Declarations (Main Window)
/// 
// Main
HWND eMainWindowHandle;

// Static Handles
HWND eStaticIntensityTitleHandle;
HWND eIntensityNameHandle;
HWND eExperimentTypeLabelHandle2;
HWND eErrorStatusTextDisplay;
HWND eDebuggingOptionsDisplayHandle;
HWND eVariablesLabelTextHandle;
HWND eSetupParametersTextHandle;
HWND eNotesLabelTextHandle;

// Code-Edited Edit Handles
HWND eExperimentTypeTextHandle2;
HWND eSystemErrorTextHandle;
HWND eColoredStatusEdit;
// Check Box Handles
HWND eIntensityScriptSavedIndicatorHandle;
HWND eOutputReadStatusButton;
HWND eOutputWriteStatusButton;
HWND eProgramIntensityOptionButton;
HWND eOutputMoreInfoCheckButton;

// User-Edited Edit Handles
HWND eIntensityScriptEditHandle;
// Button Handles
HWND eScriptErrorClearButtonHandle;
// edit handles
HWND eVariableNamesEditHandle;

HWND eLoadConfigButtonHandle;
HWND eAccumulationsButtonHandle;
HWND eScriptStatusClearButtonHandle;
HWND eScriptDebugClearButtonHandle;
HWND eConnectToMasterHandle;
HWND eReceiveVariableFiles;
HWND eLogScriptAndParamsButton;
HWND eDummyVariableSelectHandle;
HWND eDummyNumEditHandle;
HWND eDummyNumTextHandle;
HWND eDummyVariableButtonHandle;
HWND eVariableSetButtonHandle;
HWND eOutputCorrTimeButton;

HWND eVar1NameTextHandle;
HWND eVar2NameTextHandle;
HWND eVar3NameTextHandle;
HWND eVar4NameTextHandle;
HWND eVar5NameTextHandle;
HWND eVar6NameTextHandle;
HWND eAccumulationsEditHandle;
HWND eAccumulationsTextHandle;

HWND eExperimentStatusTextHandle;
HWND eDebugStatusTextHandle;

HWND eSystemStatusTextHandle;
HWND eSystemDebugTextHandle;

/// //////////////////////////////////////////////////////////////////////////////////////////
///
/// Global API Handle Declarations (Scripting Window)
/// 
HWND eScriptingWindowHandle;

HWND eStaticVerticalEditHandle;
HWND eStaticHorizontalEditHandle;

HWND eVerticalScriptEditHandle;
HWND eHorizontalScriptEditHandle;

HWND eVerticalScriptSavedIndicatorHandle;
HWND eHorizontalScriptSavedIndicatorHandle;
HWND eConfigurationSavedIndicatorHandle;

HWND eConfigurationDisplayInScripting;

HWND eVerticalScriptNameTextHandle;
HWND eHorizontalScriptNameTextHandle;
HWND scriptSaveButtonHandle;
HWND eConfigurationTextInScripting;

HWND onlyWriteWaveformsButtonHandle;

HWND eColorBox;
/// Beginning Settings Dialog
HWND eBeginDialogRichEdit;
/// Temp!
/// View Combos and texts
Control eHorizontalViewCombo, eVerticalViewCombo, eIntensityViewCombo, eHorizontalViewText, eVerticalViewText,
		eIntensityViewText;
/// Sequence Combo and Display
Control eSequenceText, eSequenceCombo, eSequenceDisplay;