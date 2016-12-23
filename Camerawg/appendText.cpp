#include "stdafx.h"
#include "appendText.h"
#include <Windows.h>
#include <string>

/*
 * This functions appends the text "newText" to the edit control corresponding to textIDC. It's legacy from working directly with Win32
 */
/*
void appendText(std::string newText, int textIDC, HWND parentWindow)
{
	// get edit control from dialog
	HWND hwndOutput = GetDlgItem(parentWindow, textIDC);
	// get length
	int index = GetWindowTextLength(hwndOutput);
	// set selection to end of text.
	SendMessage(hwndOutput, EM_SETSEL, (WPARAM)index, (LPARAM)index);
	// convert to tchar
	TCHAR* newTextTCHARVersion = (TCHAR*)newText.c_str();
	// post text
	SendMessage(hwndOutput, EM_REPLACESEL, 0, (LPARAM)newTextTCHARVersion);
	// scroll to bottom
	SendMessage(hwndOutput, WM_VSCROLL, SB_BOTTOM, 0L);

	return;
}
*/

// for mfc edits
void appendText(std::string newText, CEdit& edit)
{
	// get the initial text length
	int nLength = edit.GetWindowTextLength();
	// put the selection at the end of text
	edit.SetSel(nLength, nLength);
	// replace the selection
	edit.ReplaceSel(newText.c_str());
	return;
}

// for mfc rich edits
void appendText(std::string newText, Control<CRichEditCtrl>& edit)
{
	// get the initial text length
	int nLength = edit.GetWindowTextLength();
	// put the selection at the end of text
	edit.SetSel(nLength, nLength);
	// replace the selection
	edit.ReplaceSel(newText.c_str());
	return;
}