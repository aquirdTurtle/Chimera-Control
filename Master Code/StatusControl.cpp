#include "stdafx.h"
#include "StatusControl.h"
#include "constants.h"
#include "appendText.h"

void StatusControl::rearrange(int width, int height, fontMap fonts)
{
	header.rearrange("", "", width, height, fonts);
	edit.rearrange("", "", width, height, fonts);
	clearButton.rearrange("", "", width, height, fonts);
}

std::string StatusControl::getText()
{
	CString text;
	edit.GetWindowText(text);
	return text.GetBuffer();
}

void StatusControl::deleteChars(int num)
{
	//int nLength = richEdit.GetWindowTextLength();
	CString text;
	edit.GetWindowTextA(text);
	std::string shorterText(text);
	shorterText = shorterText.substr(0, shorterText.size() - num);
	edit.SetWindowTextA(shorterText.c_str());

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

//
void StatusControl::initialize(POINT &loc, CWnd* parent, int& id, long width, long height, std::string headerText,
							   COLORREF textColor, fontMap fonts, std::vector<CToolTipCtrl*>& tooltips)
{
	// set formatting for these scripts
	header.sPos = { loc.x, loc.y, loc.x + width-100, loc.y + 20 };
	header.Create(headerText.c_str(), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, header.sPos, parent, id++);
	header.fontType = HeadingFont;
	//
	clearButton.sPos = { loc.x + width-100, loc.y, loc.x + width, loc.y + 20 };
	clearButton.Create("Clear", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, clearButton.sPos, parent, id++);
	if (clearButton.GetDlgCtrlID() != ID_ERROR_CLEAR && clearButton.GetDlgCtrlID() != ID_STATUS_CLEAR)
	{
		throw;
	}
	loc.y += 20;
	//
	edit.sPos = { loc.x, loc.y, loc.x + width, loc.y + height - 20 };
	edit.Create(WS_CHILD | WS_VISIBLE | ES_READONLY | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER, edit.sPos,
				parent, id++);
	edit.fontType = CodeFont;
	edit.SetBackgroundColor(0, RGB(50,50,50));
	setDefaultColor(textColor);
	loc.y += height - 20;
}

//
void StatusControl::setDefaultColor(COLORREF color)
{
	defaultColor = color;
	setColor();
	CHARFORMAT myCharFormat;
	edit.GetDefaultCharFormat(myCharFormat);
	myCharFormat.cbSize = sizeof(CHARFORMAT);
	myCharFormat.dwMask = CFM_COLOR;
	myCharFormat.crTextColor = defaultColor;
	edit.SetDefaultCharFormat(myCharFormat);
}

void StatusControl::addStatusText(std::string text)
{
	addStatusText(text, false);
}

void StatusControl::addStatusText(std::string text, bool noColor)
{
	if (!noColor)
	{
		setColor();
	}
	else
	{
		setColor(RGB(255, 255, 255));
	}
	appendText(text, edit);
}

void StatusControl::setColor(COLORREF color)
{
	CHARFORMAT myCharFormat, t2;
	memset(&myCharFormat, 0, sizeof(CHARFORMAT));
	//edit.GetDefaultCharFormat(myCharFormat);
	myCharFormat.cbSize = sizeof(CHARFORMAT);
	myCharFormat.dwMask = CFM_COLOR;
	myCharFormat.crTextColor = color;
	edit.SetSel(edit.GetTextLength(), edit.GetTextLength());
	//edit.SetDefaultCharFormat(myCharFormat);
	edit.SetSelectionCharFormat(myCharFormat);
	edit.GetDefaultCharFormat(t2);
	//errBox(str(t2.bCharSet));
}

void StatusControl::setColor()
{
	setColor(defaultColor);
}


void StatusControl::clear()
{
	edit.SetWindowTextA("");
	setColor(RGB(255, 255, 255));
	addStatusText("******************************\r\n", true);
}


void StatusControl::appendTimebar()
{
	time_t time_obj = time(0);   // get time now
	struct tm currentTime;
	localtime_s(&currentTime, &time_obj);
	std::string timeStr = "(" + str(currentTime.tm_year + 1900) + ":" + str(currentTime.tm_mon + 1) + ":"
		+ str(currentTime.tm_mday) + ")" + str(currentTime.tm_hour) + ":"
		+ str(currentTime.tm_min) + ":" + str(currentTime.tm_sec);
	setColor(RGB(255, 255, 255));
	addStatusText("\r\n**********" + timeStr + "**********\r\n", true);
}