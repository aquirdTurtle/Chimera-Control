#pragma once
// Created by Mark Brown
#include "afxwin.h"
#include "Control.h"
#include "afxcmn.h"
#include "PrimaryWindows/IChimeraQtWindow.h"
#include <qlabel.h>
#include <qlineedit.h>
#include <CustomQtControls/AutoNotifyCtrls.h>
#include <qslider.h>

// a QSlider integrated with an edit to display the current number and a header for some text.

class LongCSlider{
	public:
		void initialize ( POINT& loc, IChimeraQtWindow* parent, int width, int height, std::string headerText );
		void handleSlider( int nPos );
		void handleEdit();
		double getValue ( );
		void setValue (int value, bool updateEdit=true );
		int getSliderId ( );
		void reposition ( POINT loc, LONG totalheight );
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
