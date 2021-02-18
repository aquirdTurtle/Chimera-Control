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
#include <ExperimentMonitoringAndStatus/StatusControlOptions.h>

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
		void addColoredStatusTextInner(statusMsg newMsg);
		QLabel* header=nullptr;
		QTextEdit* edit = nullptr;
		StatusControlOptions opts;
		QPushButton* clearBtn = nullptr;
		QPushButton* redrawBtn = nullptr;
		QPushButton* options = nullptr;

		std::vector<std::string> colors;
		std::deque<statusMsg> msgQue;
		const unsigned maxQueSize=100000;
};