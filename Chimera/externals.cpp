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

HANDLE eWaitingForNIAWGEvent;
HANDLE eNIAWGWaitThreadHandle;

