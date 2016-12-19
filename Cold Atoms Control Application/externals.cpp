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

//UINT REDRAW_INDICATOR = RegisterWindowMessage("REDRAW_INDICATOR");

CFont eNormalFont;
CFont eSmallFont;
CFont eCodeFont;
CFont eSmallCodeFont;
CFont eHeadingFont;
CFont eLargeHeadingFont;

//NoteSystem eNotes;
//VariableSystem eVariables;
int scriptIDs = 110000;
int debugID = 111000;
//DebuggingOptionsControl eDebugger;

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

bool eExperimentIsRunning;
bool eDontActuallyGenerate;
bool eSyntaxTimerIsActive;
bool eAbortSystemFlag = false;
bool eHorizontalSyntaxColorIsCurrent = true, eVerticalSyntaxColorIsCurrent = true, eIntensitySyntaxColorIsCurrent = true;

unsigned int eAccumulations = 0;
// thread messages
// register messages for main window.
UINT eVariableStatusMessageID = RegisterWindowMessage("ID_THREAD_VARIABLE_STATUS");
UINT eGreenMessageID = RegisterWindowMessage("ID_THREAD_GUI_GREEN");
UINT eStatusTextMessageID = RegisterWindowMessage("ID_THREAD_STATUS_MESSAGE");
UINT eDebugMessageID = RegisterWindowMessage("ID_THREAD_DEBUG_MESSAGE");
UINT eErrorTextMessageID = RegisterWindowMessage("ID_THREAD_ERROR_MESSAGE");
UINT eFatalErrorMessageID = RegisterWindowMessage("ID_THREAD_FATAL_ERROR_MESSAGE");
UINT eNormalFinishMessageID = RegisterWindowMessage("ID_THREAD_NORMAL_FINISH_MESSAGE");
UINT eColoredEditMessageID = RegisterWindowMessage("ID_VARIABLE_VALES_MESSAGE");

char eVerticalCurrentParentScriptName[_MAX_FNAME];
char eHorizontalCurrentParentScriptName[_MAX_FNAME];
char eIntensityCurrentParentScriptName[_MAX_FNAME];
char eVerticalCurrentViewScriptName[_MAX_FNAME];
char eHorizontalCurrentViewScriptName[_MAX_FNAME];
char eIntensityCurrentViewScriptName[_MAX_FNAME];

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

HWND eMainWindowHandle;
HWND eScriptingWindowHandle;
/// Beginning Settings Dialog
HWND eBeginDialogRichEdit;
