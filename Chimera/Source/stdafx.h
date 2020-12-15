// Initially created by visual studio, but I modify this sometimes. 
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
//#include "LowLevel/targetver.h"
/// MFC Stuff
// some CString constructors will be explicit
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      
// Exclude rarely-used stuff from Windows headers
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            
#endif
#include "GeneralUtilityFunctions/miscCommonFunctions.h"
#include <stdio.h>
#include <tchar.h>
// My stuff
#include "LowLevel/constants.h"
#include "LowLevel/resource.h"
#include "GeneralUtilityFunctions/Thrower.h"
#include "GeneralUtilityFunctions/range.h"

#include <fstream>
#include <string>
#include <iostream>
#include <cmath>
#include <vector>
#include <array>
