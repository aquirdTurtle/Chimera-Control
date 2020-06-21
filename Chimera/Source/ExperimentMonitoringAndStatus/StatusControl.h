// created by Mark O. Brown
#pragma once
#include "CustomMfcControlWrappers/myButton.h"
#include "Control.h"
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <PrimaryWindows/IChimeraWindowWidget.h>

class StatusControl
{
	public:
		void initialize(POINT &topLeftCorner, IChimeraWindowWidget* parent, long size, std::string headerText, std::string textColor );
		void setDefaultColor(std::string color);
		void addStatusText(std::string text);
		void addStatusText (std::string text, std::string color);
		void clear();
		void appendTimebar();
	private:
		QLabel* header;
		QTextEdit* edit;
		QPushButton* clearBtn;
		
		std::string defaultColor;
};