#include "stdafx.h"
#include "miscellaneousCommonFunctions.h"
#include <Windows.h>
#include <string>


// for mfc edits
void appendText(std::string newText, CEdit& edit)
{
	// get the initial text length
	int nLength = edit.GetWindowTextLength();
	// put the selection at the end of text
	edit.SetSel(nLength, nLength);
	// replace the selection
	edit.ReplaceSel(cstr(newText));
	edit.LineScroll(INT_MAX);
}

// for mfc rich edits
void appendText(std::string newText, Control<CRichEditCtrl>& edit)
{
	// get the initial text length
	int nLength = edit.GetWindowTextLength();
	// put the selection at the end of text
	edit.SetSel(nLength, nLength);
	// replace the selection
	edit.ReplaceSel(cstr(newText));
	edit.SetFocus();
	nLength = edit.GetWindowTextLength();
	// put the selection at the end of text
	edit.SetSel(nLength, nLength);
	//	edit.LineScroll(INT_MAX);
}


std::string doubleToString( double number, long precision )
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision( precision ) << number;
	return stream.str();
}

