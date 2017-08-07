#include "stdafx.h"
#include "constants.h"
#include "RichEditControl.h"
#include "Richedit.h"
#include "appendText.h"
#include <unordered_map>
#include "MasterWindow.h"


void RichEditControl::rearrange(UINT width, UINT height, fontMap fonts)
{
	title.rearrange("", "", width, height, fonts);
	richEdit.rearrange("", "", width, height, fonts);
	clearButton.rearrange("", "", width, height, fonts);
}

std::string RichEditControl::getText()
{
	CString text;
	richEdit.GetWindowText(text);
	return text.GetBuffer();
}

void RichEditControl::deleteChars( int num )
{
	//int nLength = richEdit.GetWindowTextLength();
	CString text;
	richEdit.GetWindowTextA( text );
	std::string shorterText( text );
	shorterText = shorterText.substr( 0, shorterText.size() - num );
	richEdit.SetWindowTextA( shorterText.c_str() );
	
	// put the selection at the end of text
	/*
	CHARRANGE test;
	int first = nLength - num;
	richEdit.SetSel( HIWORD(first), nLength );
	richEdit.
	richEdit.GetSel( test );
	richEdit.Clear();
	*/
}

// can expand this to take arbitrary RGB if more colors are wanted. Right now, 0 = default color, 1 = white.
bool RichEditControl::appendText(std::string text, int color)
{	
	if (color == 0)
	{
		// default color
		int nLength = richEdit.GetWindowTextLength();
		// put the selection at the end of text
		richEdit.SetSel(nLength, nLength);
		// replace the selection
		richEdit.ReplaceSel(text.c_str());		
	}
	else if (color == 1)
	{
		// white
		int nLength = richEdit.GetWindowTextLength();
		// put the selection at the end of text
		richEdit.SetSel(nLength, nLength);
		CHARFORMAT myCharFormat;
		memset(&myCharFormat, 0, sizeof(CHARFORMAT));
		myCharFormat.cbSize = sizeof(CHARFORMAT);
		myCharFormat.dwMask = CFM_COLOR;
		myCharFormat.crTextColor = RGB(255, 255, 255);
		richEdit.SetSelectionCharFormat(myCharFormat);
		// replace the selection
		richEdit.ReplaceSel(text.c_str());
		myCharFormat.crTextColor = defaultTextColor;
		richEdit.SetSelectionCharFormat(myCharFormat);
	}
	return true;
}


void RichEditControl::clear()
{
	// clear the edit
	richEdit.SetWindowTextA("");
}

void RichEditControl::initialize( RECT editSize, std::string titleText, COLORREF defaultColor, MasterWindow* master, 
								  int& id)
{
	defaultTextColor = defaultColor;
	AfxInitRichEdit();
	InitCommonControls();
	LoadLibrary(TEXT("Msftedit.dll"));
	CHARFORMAT myCharFormat;
	memset(&myCharFormat, 0, sizeof(CHARFORMAT));
	myCharFormat.cbSize = sizeof(CHARFORMAT);
	myCharFormat.dwMask = CFM_COLOR;
	myCharFormat.crTextColor = defaultTextColor;
	// title
	title.sPos = { editSize.left, editSize.top, editSize.right - 80, editSize.top + 20 };
	title.Create( titleText.c_str(), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER | ES_READONLY, title.sPos, 
				  master, id++);
	// Clear Button
	clearButton.sPos = { editSize.right - 80, editSize.top, editSize.right, editSize.top + 20 };
	if ( clearButton.GetDlgCtrlID() != ID_ERROR_CLEAR && clearButton.GetDlgCtrlID() != ID_STATUS_CLEAR )
	{
		throw;
	}
	clearButton.Create( "Clear", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, clearButton.sPos, master, 
						id++ );
	// Edit
	richEdit.sPos = { editSize.left, editSize.top + 20, editSize.right, editSize.bottom };
	richEdit.Create( WS_CHILD | WS_VISIBLE | ES_READONLY | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER, 
					 richEdit.sPos, master, id++);
	
	richEdit.SetBackgroundColor(0, RGB(15,15,15));
	richEdit.SetEventMask(ENM_CHANGE);
	richEdit.SetDefaultCharFormat(myCharFormat);
}

INT_PTR RichEditControl::handleColorMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, brushMap brushes)
{
	DWORD controlID = GetDlgCtrlID((HWND)lParam);
	HDC hdcStatic = (HDC)wParam;
	if (controlID == title.GetDlgCtrlID())
	{
		SetTextColor(hdcStatic, RGB(218, 165, 32));
		SetBkColor(hdcStatic, RGB(30, 30, 30));
		return (LRESULT)brushes["Medium Grey"];
	}
	else
	{
		return NULL;
	}
}
