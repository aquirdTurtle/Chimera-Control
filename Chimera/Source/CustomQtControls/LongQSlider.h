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
		int getSliderId ( );
		void reposition ( QPoint loc, LONG totalheight );
		unsigned getEditId ( );
		void hide ( int hideornot );
		QSlider* slider;
		CQLineEdit* edit;
		QLabel* header;
	private:
		double currentValue;
		const double maxVal = 1000;
		const double minVal = 0;
};
