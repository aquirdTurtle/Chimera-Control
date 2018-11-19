#pragma once
#include "Control.h"
#include "externals.h"
#include "ErrDialog.h"
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


// used currently with the python API
// shows error message if it exists. Could be function but for consistency with other ERR_X Macros...
#define ERR_POP(string) {if (string != ""){errBox(string);}}
// shows error message and exits given function with error.
#define ERR_POP_RETURN(string) {if (string != ""){errBox(string); return;}}

/*
 * This functions appends the text "newText" to the edit control corresponding to textIDC.
 */
void appendText(std::string newText, CEdit& edit);
void appendText(std::string newText, Control<CRichEditCtrl>& edit);

/// a set of functions that take more arbitrary things to strings that str (which is also rather wordy for such a simple 
/// function...

// this function takes any argument, converts it to a string, and displays it on the screen. It can be useful for debuging.
template <typename T> void errBox( T msg )
{
	ErrDialog dlg( cstr ( msg ), ErrDialog::type::error );
	dlg.DoModal ( );
}


// this function takes any argument, converts it to a string, and displays it on the screen. It can be useful for debuging.
template <typename T> void infoBox( T msg )
{
	ErrDialog dlg ( cstr ( msg ), ErrDialog::type::info );
	dlg.DoModal ( );
	//MessageBox( eMainWindowHwnd, cstr( msg ), "Info", MB_ICONWARNING );
}

template <typename T> int promptBox( T msg, UINT promptStyle )
{
	return MessageBox( eMainWindowHwnd, cstr( msg ), "Prompt", promptStyle | MB_SYSTEMMODAL );
}

ULONG getNextFileIndex( std::string fileBase, std::string ext );

