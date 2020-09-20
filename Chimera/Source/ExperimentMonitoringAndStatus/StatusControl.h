// created by Mark O. Brown
#pragma once
#include "Control.h"
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit.h>
#include <CustomQtControls/AutoNotifyCtrls.h>
#include <PrimaryWindows/IChimeraQtWindow.h>
#include <qplaintextedit.h>

class StatusControl
{
	public:
		void initialize(POINT &topLeftCorner, IChimeraQtWindow* parent, long size, std::string headerText, 
			std::vector<std::string> textColors);
		void addStatusText(std::string text, unsigned level = 0);
		void addStatusText (std::string text, std::string color);
		void clear();
		void appendTimebar();
	private:
		QLabel* header;
		QLabel* debugLevelLabel;
		CQLineEdit* debugLevelEdit;
		QPlainTextEdit* edit;
		unsigned currentLevel=-1;
		QPushButton* clearBtn;
		std::vector<std::string> colors;
};