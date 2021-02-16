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
#include <queue>

class StatusControl{
	public:
		void initialize(QPoint& loc, IChimeraQtWindow* parent, long size, std::string headerText,
			std::vector<std::string> textColors);

		// void addStatusText(std::string text, unsigned level = 0);
		void addStatusTextColored(std::string text, std::string colorStr);
		void addPlainText(std::string text);
		void addStatusText (statusMsg newMsg);
		void addStatusToQue(statusMsg newMsg);
		void clear();
		void appendTimebar();
		void redrawControl();
		
	private:
		void addPlainStatusTextInner(statusMsg newMsg);
		void addPlainStatusTextInner(std::string text, unsigned level = 0);
		void addColoredStatusTextInner(statusMsg newMsg);
		void addColoredStatusTextInner(std::string text, unsigned level = 0);
		QLabel* header=nullptr;
		QLabel* debugLevelLabel = nullptr;
		CQLineEdit* debugLevelEdit = nullptr;
		QTextEdit* edit = nullptr;
		unsigned currentLevel=-1;
		QPushButton* clearBtn = nullptr;
		QPushButton* redrawBtn = nullptr;
		std::vector<std::string> colors;
		std::deque<statusMsg> msgQue;
		const unsigned maxQueSize=100000;
};