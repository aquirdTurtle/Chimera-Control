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
		void addStatusText(statusMsg newMsg);
		void clear();
		void appendTimebar();
		void redrawControl();
		
	private:
		void addStatusToQue(statusMsg newMsg);
		void addHtmlStatusText(std::string text, std::string colorStr);
		void addPlainText(std::string text);
		void addPlainStatusTextInner(statusMsg newMsg);
		void addHtmlStatusTextInner(statusMsg newMsg);
		QLabel* header=nullptr;
		QTextEdit* edit = nullptr;
		StatusControlOptions opts;
		QPushButton* clearBtn = nullptr;
		QPushButton* redrawBtn = nullptr;
		QPushButton* options = nullptr;

		std::vector<std::string> colors;
		std::deque<statusMsg> msgHistory;
};