#pragma once
// Created by Mark Brown
#include "PrimaryWindows/IChimeraQtWindow.h"
#include <qlabel.h>
#include <qlineedit.h>
#include <CustomQtControls/AutoNotifyCtrls.h>
#include <qslider.h>

// a QSlider integrated with an edit to display the current number and a header for some text.

class LongQSlider{
	public:
		void initialize ( QPoint& loc, IChimeraQtWindow* parent, int width, int height, std::string headerText );
		void handleSlider( int nPos );
		void handleEdit();
		double getValue ( );
		void setValue (int value, bool updateEdit=true );
		void reposition ( QPoint loc, long totalheight );
		void show ( int visible );
		QSlider* slider = nullptr;
		CQLineEdit* edit = nullptr;
		QLabel* header = nullptr;
	private:
		double currentValue;
		double maxVal = 1000;
		double minVal = 0;
};
