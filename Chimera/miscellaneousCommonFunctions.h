#pragma once
#include "Control.h"
#include "afxwin.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include "Richedit.h"
#include "afxcmn.h"
#include "CommCtrl.h"
#include "externals.h"

// shows error message if it exists. Could be function but for consistency with other ERR_X Macros...
#define ERR_POP(string) {if (string != ""){errBox(string);}}
// shows error message and exits given function with error.
#define ERR_POP_RETURN(string) {if (string != ""){errBox(string); return;}}

/*
 * This functions appends the text "newText" to the edit control corresponding to textIDC.
 */
//void appendText(std::string newText, int textIDC, HWND parentWindow);
void appendText(std::string newText, CEdit& edit);
void appendText(std::string newText, Control<CRichEditCtrl>& edit);

//
#define idVerify(idSet, ...)	verifyIdsMatch(idSet, {__VA_ARGS__}, __FILE__, __LINE__)

/// a set of functions that take more arbitrary things to strings that str (which is also rather wordy for such a simple 
/// function...

// this function takes any argument, converts it to a string, and displays it on the screen. It can be useful for debuging.
template <typename T> void errBox( T msg )
{
	MessageBox( eMainWindowHwnd, cstr( msg ), "ERROR!", MB_ICONERROR | MB_SYSTEMMODAL );
}


// this function takes any argument, converts it to a string, and displays it on the screen. It can be useful for debuging.
template <typename T> void infoBox( T msg )
{
	MessageBox( eMainWindowHwnd, cstr( msg ), "Info", MB_ICONWARNING );
}

template <typename T> int promptBox( T msg, UINT promptStyle )
{
	return MessageBox( eMainWindowHwnd, cstr( msg ), "Prompt", promptStyle | MB_SYSTEMMODAL );
}

template <typename ControlType> void verifyIdsMatch(Control<ControlType>& control, std::vector<UINT> ids,
													 const char *file, int line)
{
	std::string idString;
	for (auto id : ids)
	{
		if (control.GetDlgCtrlID() == id)
		{
			return;
		}
		idString += str(id) + ", ";
	}

	// if it reaches here, it didn't
	errBox("ERROR: the following ID: " + str(control.GetDlgCtrlID()) + " should match one of \"" + idString + "\". "
		   "Please change the second ID in order to make them match. This occured at file \"" + str(file) + "\" "
		   "line " + str(line));

	throw;
}


ULONG getNextFileIndex( std::string fileBase, std::string ext );
