#include "stdafx.h"
#include "StatusControl.h"


void StatusControl::rearrange(int width, int height, fontMap fonts)
{
	header.rearrange("", "", width, height, fonts);
	edit.rearrange("", "", width, height, fonts);
	clearButton.rearrange("", "", width, height, fonts);
}

//
void StatusControl::initialize(POINT &loc, CWnd* parent, int& id, UINT size, std::string headerText, 
							   COLORREF textColor, fontMap fonts, cToolTips& tooltips)
{
	// set formatting for these scripts
	header.sPos = { loc.x, loc.y, loc.x + 380, loc.y + 20 };
	header.Create(cstr(headerText), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, header.sPos, parent, id++);
	header.SetFont(fonts["Heading Font"]);
	//
	clearButton.sPos = { loc.x + 380, loc.y, loc.x + 480, loc.y + 20 };
	clearButton.Create("Clear", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, clearButton.sPos, parent, id++);
	idVerify(clearButton, IDC_MAIN_STATUS_BUTTON, IDC_ERROR_STATUS_BUTTON, IDC_DEBUG_STATUS_BUTTON);
	clearButton.SetFont(fonts["Normal Font"]);
	loc.y += 20;
	//
	edit.sPos = { loc.x, loc.y, loc.x + 480, loc.y + long(size)};
	edit.Create( WS_CHILD | WS_VISIBLE | ES_READONLY | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | WS_BORDER, 
				 edit.sPos, parent, id++ );
	edit.SetFont(fonts["Code Font"]);
	edit.SetBackgroundColor(0, RGB(15, 15, 20));
	setDefaultColor(textColor);
	loc.y += size;
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
	appendText(text, edit);
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
	edit.SetSelectionCharFormat(myCharFormat);
	edit.GetDefaultCharFormat(t2);
}

void StatusControl::setColor()
{
	setColor(defaultColor);
}


void StatusControl::clear() 
{
	edit.SetWindowTextA("");
	setColor(RGB(255, 255, 255));
	addStatusText("\r\n******************************\r\n", true);
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

