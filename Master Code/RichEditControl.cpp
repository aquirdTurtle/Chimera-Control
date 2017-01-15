#include "stdafx.h"
#include "constants.h"
#include "RichEditControl.h"
#include "Richedit.h"
#include "appendText.h"
#include <unordered_map>

RichEditControl::RichEditControl(int& idStart) 
{
	// initialize control ids
	this->richEdit.ID = idStart;
	this->clearButton.ID = idStart + 1;
	if (clearButton.ID != ID_ERROR_CLEAR && clearButton.ID != ID_STATUS_CLEAR)
	{
		errBox("ERROR: the clearButton.ID didn't match any clear button IDs. The id was: " + std::to_string(clearButton.ID));
	}
	this->title.ID = idStart + 2;
	idStart += 3;
}

std::string RichEditControl::getText()
{
	CString text;
	this->richEdit.GetWindowText(text);
	return text.GetBuffer();
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
		myCharFormat.crTextColor = this->defaultTextColor;
		richEdit.SetSelectionCharFormat(myCharFormat);
	}
	return true;
}


bool RichEditControl::clear()
{
	// clear the edit
	richEdit.SetWindowTextA("");
	return true;
}

bool RichEditControl::initialize(RECT editSize, HWND windowHandle, std::string titleText, COLORREF defaultColor)
{
	this->defaultTextColor = defaultColor;
	AfxInitRichEdit();
	InitCommonControls();
	LoadLibrary(TEXT("Msftedit.dll"));
	CHARFORMAT myCharFormat;
	memset(&myCharFormat, 0, sizeof(CHARFORMAT));
	myCharFormat.cbSize = sizeof(CHARFORMAT);
	myCharFormat.dwMask = CFM_COLOR;
	myCharFormat.crTextColor = defaultTextColor;
	RECT position;	
	// title
	position = this->title.position = { editSize.left, editSize.top, editSize.right - 80, editSize.top + 20 };
	this->title.Create(titleText.c_str(), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER | ES_READONLY, position, CWnd::FromHandle(windowHandle),
		title.ID);
	// button
	position = this->clearButton.position = { editSize.right - 80, editSize.top, editSize.right, editSize.top + 20 };
	this->clearButton.Create("Clear", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, position, CWnd::FromHandle(windowHandle), clearButton.ID);
	// Edit
	position = richEdit.position = { editSize.left, editSize.top + 20, editSize.right, editSize.bottom };
	richEdit.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | WS_HSCROLL | ES_READONLY, position, CWnd::FromHandle(windowHandle), richEdit.ID);
	richEdit.SetBackgroundColor(0, RGB(15,15,15));
	richEdit.SetEventMask(ENM_CHANGE);
	richEdit.SetDefaultCharFormat(myCharFormat);
	return true;
}

INT_PTR RichEditControl::handleColorMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, std::unordered_map<std::string, HBRUSH> brushes)
{
	DWORD controlID = GetDlgCtrlID((HWND)lParam);
	HDC hdcStatic = (HDC)wParam;
	if (controlID == this->title.ID)
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
