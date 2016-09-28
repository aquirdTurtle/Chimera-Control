// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once
#include "targetver.h"
// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN
// Windows Header Files:
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string>
#include <vector>
// my headers that I want everywhere.
#include "Resource.h"
#include "externals.h"
#include "constants.h"
#include "errBox.h"
#include "doubleToString.h"
#include "appendText.h"

// shows error message if it exists. Could be function but for consistency with other ERR_X Macros...
#define ERR_POP(string) {if (string != ""){errBox(string);}}
// shows error message and exits given function with error.
#define ERR_POP_RETURN(string) {if (string != ""){errBox(string); return true;}}
