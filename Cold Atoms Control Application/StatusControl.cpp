#include "stdafx.h"
#include "StatusControl.h"
#include "appendText.h"

//
void StatusControl::initialize(POINT &loc, CWnd* parent, int& id, unsigned int size, std::string headerText, COLORREF textColor)
{
	// set formatting for these scripts
	header.ID = id++;
	header.position = { loc.x, loc.y, loc.x + 380, loc.y + 20 };
	header.Create(headerText.c_str(), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, header.position, parent, header.ID);
	header.SetFont(&eHeadingFont);
	//
	clearButton.ID = id++;
	if (clearButton.ID != IDC_MAIN_STATUS_BUTTON && clearButton.ID != IDC_ERROR_STATUS_BUTTON && clearButton.ID != IDC_DEBUG_STATUS_BUTTON)
	{
		throw;
	}
	clearButton.position = { loc.x + 380, loc.y, loc.x + 480, loc.y + 20 };
	clearButton.Create("Clear", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, clearButton.position, parent, clearButton.ID);
	clearButton.SetFont(&eNormalFont);
	loc.y += 20;
	//
	edit.ID = id++;
	edit.position = { loc.x, loc.y, loc.x + 480, loc.y + long(size)};
	edit.Create(WS_CHILD | WS_VISIBLE | ES_READONLY | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER, edit.position, parent, edit.ID);
	edit.SetFont(&eCodeFont);
	edit.SetBackgroundColor(0, RGB(15, 15, 20));
	this->setDefaultColor(textColor);
	loc.y += size;
	return;
}

//
void StatusControl::setDefaultColor(COLORREF color)
{
	this->defaultColor = color;
	CHARFORMAT myCharFormat;
	memset(&myCharFormat, 0, sizeof(CHARFORMAT));
	myCharFormat.cbSize = sizeof(CHARFORMAT);
	myCharFormat.dwMask = CFM_COLOR;
	myCharFormat.crTextColor = defaultColor;
	edit.SetDefaultCharFormat(myCharFormat);
	return;
}

void StatusControl::addStatusText(std::string text)
{
	appendText(text, this->edit);
	return;
}

void StatusControl::clear() 
{
	this->edit.SetWindowTextA("*********************\r\n");
	return;
}

void StatusControl::appendTimebar()
{																			 
	time_t time_obj = time(0);   // get time now
	struct tm currentTime;
	localtime_s(&currentTime, &time_obj);
	std::string timeStr = "(" + std::to_string(currentTime.tm_year + 1900) + ":" + std::to_string(currentTime.tm_mon + 1) + ":"
		+ std::to_string(currentTime.tm_mday) + ")" + std::to_string(currentTime.tm_hour) + ":"
		+ std::to_string(currentTime.tm_min) + ":" + std::to_string(currentTime.tm_sec);
	CHARFORMAT myCharFormat;
	memset(&myCharFormat, 0, sizeof(CHARFORMAT));
	myCharFormat.cbSize = sizeof(CHARFORMAT);
	myCharFormat.dwMask = CFM_COLOR;
	// white
	myCharFormat.crTextColor = RGB(255, 255, 255);
	edit.SetDefaultCharFormat(myCharFormat);
	addStatusText("\r\n**********" + timeStr + "**********\r\n");
	myCharFormat.crTextColor = defaultColor;
	edit.SetDefaultCharFormat(myCharFormat);
	return;
}

