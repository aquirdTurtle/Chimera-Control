#include "stdafx.h"
#include "StatusControl.h"
#include "constants.h"
#include "appendText.h"

void StatusControl::rearrange(int width, int height, std::unordered_map<std::string, CFont*> fonts)
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
	header.ID = id++;
	header.sPos = { loc.x, loc.y, loc.x + width-100, loc.y + 20 };
	header.Create(headerText.c_str(), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, header.sPos, parent, header.ID);
	header.SetFont(fonts["Heading Font"]);
	//
	clearButton.ID = id++;
	if (clearButton.ID != ID_ERROR_CLEAR && clearButton.ID != ID_STATUS_CLEAR)
	{
		throw;
	}
	clearButton.sPos = { loc.x + width-100, loc.y, loc.x + width, loc.y + 20 };
	clearButton.Create("Clear", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, clearButton.sPos, parent, clearButton.ID);
	clearButton.SetFont(fonts["Normal Font"]);
	loc.y += 20;
	//
	edit.ID = id++;
	edit.sPos = { loc.x, loc.y, loc.x + width, loc.y + height - 20 };
	edit.Create(WS_CHILD | WS_VISIBLE | ES_READONLY | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER, edit.sPos, parent, edit.ID);
	edit.SetFont(fonts["Code Font"]);
	edit.SetBackgroundColor(0, RGB(50,50,50));
	setDefaultColor(textColor);
	loc.y += height - 20;
}

//
void StatusControl::setDefaultColor(COLORREF color)
{
	this->defaultColor = color;
	this->setColor();
	CHARFORMAT myCharFormat;
	this->edit.GetDefaultCharFormat(myCharFormat);
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
	appendText(text, this->edit);
}

void StatusControl::setColor(COLORREF color)
{
	CHARFORMAT myCharFormat, t2;
	memset(&myCharFormat, 0, sizeof(CHARFORMAT));
	//this->edit.GetDefaultCharFormat(myCharFormat);
	myCharFormat.cbSize = sizeof(CHARFORMAT);
	myCharFormat.dwMask = CFM_COLOR;
	myCharFormat.crTextColor = color;
	edit.SetSel(edit.GetTextLength(), edit.GetTextLength());
	//edit.SetDefaultCharFormat(myCharFormat);
	edit.SetSelectionCharFormat(myCharFormat);
	edit.GetDefaultCharFormat(t2);
	//errBox(std::to_string(t2.bCharSet));
}

void StatusControl::setColor()
{
	setColor(this->defaultColor);
}


void StatusControl::clear()
{
	this->edit.SetWindowTextA("");
	setColor(RGB(255, 255, 255));
	addStatusText("******************************\r\n", true);
}


void StatusControl::appendTimebar()
{
	time_t time_obj = time(0);   // get time now
	struct tm currentTime;
	localtime_s(&currentTime, &time_obj);
	std::string timeStr = "(" + std::to_string(currentTime.tm_year + 1900) + ":" + std::to_string(currentTime.tm_mon + 1) + ":"
		+ std::to_string(currentTime.tm_mday) + ")" + std::to_string(currentTime.tm_hour) + ":"
		+ std::to_string(currentTime.tm_min) + ":" + std::to_string(currentTime.tm_sec);
	setColor(RGB(255, 255, 255));
	addStatusText("\r\n**********" + timeStr + "**********\r\n", true);
}