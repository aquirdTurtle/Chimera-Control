// stdafx.h
// include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "errBox.h"

/// MFC Stuff
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include <afx.h>
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
/// 
#include "afxcoll.h"
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
//#include "Windows.h"
#include "resource.h"
#include <string>
#include <vector>

/// MY MACROS
// these are all things that I use to make my code a tad simpler.
// simple macro to condense this style of error checking. I might change my usage of errors later.
#define ERRCHK(functionCall) if (!functionCall){return false;}
// verbose must be defined. This is meant to be used in the experiment thread.
#define VERBOSE(text) (if (verbose){input->status->appendText(text, 0);})
