// created by Mark O. Brown
#pragma once
// This header file includes all of the extern declarations of all of my external (global) variables.
#include "NIAWG/NiawgStructures.h"
#include "Control.h"
#include "afxWin.h"
#include <string>
#include <vector>
#include "niFgen.h"

extern double infernoMap[256][3];

extern bool eWaitError;
/// Global Options Variables
extern bool eAbortNiawgFlag;
/// Other Global Handles
extern HANDLE eWaitingForNIAWGEvent;
extern HANDLE eNIAWGWaitThreadHandle;

