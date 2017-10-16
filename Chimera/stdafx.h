// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once
#include "targetver.h"
/// MFC Stuff
// some CString constructors will be explicit
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      
// Exclude rarely-used stuff from Windows headers
#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            
#endif
// MFC core and standard components
#include <afx.h>
#include <afxwin.h>         
// MFC extensions
#include <afxext.h>         
// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           
#endif
// MFC support for Windows Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     
#endif
///
// 
#include <afxcontrolbars.h>
// Contains functions and types used by the waveform generator.
// #include "niFgen.h"

// important!
#include "miscellaneousCommonFunctions.h"

#include <stdio.h>
#include <tchar.h>
// My stuff
#include "externals.h"
#include "constants.h"
#include "resource.h"

#include <fstream>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>

