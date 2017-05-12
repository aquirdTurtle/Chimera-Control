#include "stdafx.h"
#include "StatusControl.h"


void StatusControl::rearrange(int width, int height, std::unordered_map<std::string, CFont*> fonts)
{
	header.rearrange("", "", width, height, fonts);
	edit.rearrange("", "", width, height, fonts);
	clearButton.rearrange("", "", width, height, fonts);
}

//
void StatusControl::initialize(POINT &loc, CWnd* parent, int& id, unsigned int size, std::string headerText, 
							   COLORREF textColor, std::unordered_map<std::string, CFont*> fonts, std::vector<CToolTipCtrl*>& tooltips)
{
	// set formatting for these scripts
	header.ID = id++;
	header.sPos = { loc.x, loc.y, loc.x + 380, loc.y + 20 };
	header.Create(headerText.c_str(), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, header.sPos, parent, header.ID);
	header.SetFont(fonts["Heading Font"]);
	//
	clearButton.ID = id++;
	if (clearButton.ID != IDC_MAIN_STATUS_BUTTON && clearButton.ID != IDC_ERROR_STATUS_BUTTON && clearButton.ID != IDC_DEBUG_STATUS_BUTTON)
	{
		throw;
	}
	clearButton.sPos = { loc.x + 380, loc.y, loc.x + 480, loc.y + 20 };
	clearButton.Create("Clear", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, clearButton.sPos, parent, clearButton.ID);
	clearButton.SetFont(fonts["Normal Font"]);
	loc.y += 20;
	//
	edit.ID = id++;
	edit.sPos = { loc.x, loc.y, loc.x + 480, loc.y + long(size)};
	edit.Create(WS_CHILD | WS_VISIBLE | ES_READONLY | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER, edit.sPos, parent, edit.ID);
	edit.SetFont(fonts["Code Font"]);
	edit.SetBackgroundColor(0, RGB(15, 15, 20));
	this->setDefaultColor(textColor);
	loc.y += size;
	return;
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
	appendText(text, this->edit);
	return;
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
	addStatusText("\r\n******************************\r\n", true);
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

