// created by Mark O. Brown
#include "stdafx.h"
#include "StatusControl.h"
#include <PrimaryWindows/IChimeraQtWindow.h>

void StatusControl::initialize (QPoint& loc, IChimeraQtWindow* parent, long size,
	std::string headerText, std::vector<std::string> textColors) {
	// fixed size que.
	int& px = loc.rx (), & py = loc.ry ();
	if (textColors.size () == 0) {
		thrower ("Need to set a nonzero number of colors for status control!");
	}

	colors = textColors;
	header = new QLabel (headerText.c_str (), parent);
	header->setGeometry(px, py, 215, 25);

	debugLevelLabel = new QLabel ("Dbg Lvl", parent);
	debugLevelLabel->setGeometry (px + 215, py, 60, 25);

	debugLevelEdit = new CQLineEdit (parent);
	debugLevelEdit->setGeometry (px + 275, py, 50, 25);
	debugLevelEdit->setText ("-1");

	parent->connect (debugLevelEdit, &QLineEdit::textChanged, [this]() {
		try { 
			currentLevel = boost::lexical_cast<unsigned>(str (debugLevelEdit->text ())); 
		}
		catch (boost::bad_lexical_cast&) { 
			currentLevel = 0; 
		}
		addStatusText ("Changed Debug Level to \"" + str (currentLevel) + "\"\n");
		});

	redrawBtn = new QPushButton("Redraw", parent);
	redrawBtn->setGeometry(px + 325, py, 80, 25);
	redrawBtn->connect(redrawBtn, &QPushButton::pressed, [this]() {
			redrawControl();
		});

	clearBtn = new QPushButton ("Clear", parent);
	clearBtn->setGeometry(px + 405, py, 75, 25);

	py += 25;
	edit = new QTextEdit (parent);
	edit->move (px, py);
	edit->setFixedSize (480, size);
	edit->setReadOnly (true);
	//edit->setStyleSheet ("QPlainTextEdit { color: " + qstr (textColors[0]) + "; }");
	edit->setObjectName (qstr(headerText));
	py += size;
	parent->connect (clearBtn, &QPushButton::released, [this]() {clear (); });
}

void StatusControl::addStatusToQue(statusMsg newMsg) {
	statusMsg test = { "test\t\n!",0 };
	msgQue.push_back(newMsg);
	if (msgQue.size() > maxQueSize) {
		msgQue.pop_front();
	}
}

void StatusControl::redrawControl() {
	edit->clear();
	for (auto msg : msgQue) {
		addColoredStatusTextInner(msg);
	}
}

void StatusControl::addStatusText(statusMsg msg) {
	addStatusToQue(msg);
	addPlainStatusTextInner(msg);
}

void StatusControl::addPlainStatusTextInner(statusMsg msg) {
	addPlainStatusTextInner(str(msg.msg), msg.baseLevel);
}

void StatusControl::addColoredStatusTextInner(statusMsg msg) {
	addColoredStatusTextInner(str(msg.msg), msg.baseLevel);
}

void StatusControl::addPlainStatusTextInner (std::string text, unsigned level){
	if (currentLevel >= level) {
		for (auto lvl : range (level)) {
			// visual indication of what level a message is.
			text = ">" + text;
		}
		addPlainText(text);
	}
}

void StatusControl::addColoredStatusTextInner(std::string text, unsigned level) {
	if (colors.size() == 0) {
		return;
	}
	if (currentLevel >= level) {
		for (auto lvl : range(level)) {
			// visual indication of what level a message is.
			text = ">" + text;
		}
		addStatusTextColored(text, level > colors.size() ? colors.back() : colors[level]);
	}
}

void StatusControl::addStatusTextColored(std::string text, std::string color){
	QString htmlTxt = ("<font color = \"" + color + "\">" + text + "</font>").c_str();
	htmlTxt.replace ("\r", "");
	htmlTxt.replace ("\n", "<br/>");
	htmlTxt.replace("\t", "&nbsp;&nbsp;&nbsp;&nbsp;");
 	//e.g. <font color = "red">This is some text!< / font>
 	//edit->appendHtml (htmlTxt);
	edit->moveCursor (QTextCursor::End);
	edit->textCursor ().insertHtml (htmlTxt);
	//edit->insertHtml (htmlTxt);
	//edit->insertHtml(qstr(text));
	edit->moveCursor (QTextCursor::End);
}

void StatusControl::addPlainText(std::string text) {
	if (!edit) {
		return;
	}
	edit->insertPlainText(qstr(text));
	edit->moveCursor(QTextCursor::End);
}

void StatusControl::clear() {
	edit->clear ();
	addStatusTextColored("**************CLEARED****************\r\n", "#FFFFFF");
}


void StatusControl::appendTimebar() {
	time_t time_obj = time(0);   // get time now
	struct tm currentTime;
	localtime_s(&currentTime, &time_obj);
	std::string timeStr = "(" + str(currentTime.tm_year + 1900) + ":" + str(currentTime.tm_mon + 1) + ":"
		+ str(currentTime.tm_mday) + ") " + str(currentTime.tm_hour) + ":"
		+ str(currentTime.tm_min) + ":" + str(currentTime.tm_sec);
	statusMsg timebarMsg;
	timebarMsg.msg = qstr("\r\n**********" + timeStr + "**********\r\n");
	addStatusText(timebarMsg);
}

