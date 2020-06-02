// created by Mark O. Brown
#include "stdafx.h"
#include "StatusControl.h"
#include <PrimaryWindows/IChimeraWindowWidget.h>

void StatusControl::initialize(POINT &loc, IChimeraWindowWidget* parent, long size, std::string headerText, std::string textColor)
{
	defaultColor = textColor;
	header = new QLabel (headerText.c_str(), parent);
	header->setFixedSize (380, 25);
	header->move (loc.x, loc.y);
	clearBtn = new QPushButton (parent);
	clearBtn->setText ("Clear");
	clearBtn->setFixedSize (100, 25);
	clearBtn->move (loc.x + 380, loc.y);
	loc.y += 25;
	edit = new QTextEdit (parent);
	edit->move (loc.x, loc.y);
	edit->setFixedSize (480, size);
	edit->setReadOnly (true);
	loc.y += size;
	parent->connect (clearBtn, &QPushButton::released, [this]() {clear (); });
}

//
void StatusControl::setDefaultColor(std::string color)
{
	defaultColor = color;
}

void StatusControl::addStatusText (std::string text)
{
	addStatusText (text, defaultColor);
}

void StatusControl::addStatusText(std::string text, std::string color)
{
	QString htmlTxt = ("<font color = \"" + color + "\">" + text + "</font>").c_str();
	htmlTxt.replace ("\r", ""); 
	htmlTxt.replace ("\n", "<br/>");
	//e.g. <font color = "red">This is some text!< / font>
	edit->moveCursor (QTextCursor::End);
	edit->insertHtml (htmlTxt);
	edit->moveCursor (QTextCursor::End);
}

void StatusControl::clear() 
{
	edit->clear ();
	addStatusText("******************************\r\n", "#FFFFFF");
}


void StatusControl::appendTimebar()
{
	time_t time_obj = time(0);   // get time now
	struct tm currentTime;
	localtime_s(&currentTime, &time_obj);
	std::string timeStr = "(" + str(currentTime.tm_year + 1900) + ":" + str(currentTime.tm_mon + 1) + ":"
		+ str(currentTime.tm_mday) + ")" + str(currentTime.tm_hour) + ":"
		+ str(currentTime.tm_min) + ":" + str(currentTime.tm_sec);
	addStatusText("\r\n**********" + timeStr + "**********\r\n", "#FFFFFF");
}

