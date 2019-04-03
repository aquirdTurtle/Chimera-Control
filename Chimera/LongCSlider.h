#pragma once
// Created by Mark Brown
#include "afxwin.h"
#include "Control.h"
#include "afxcmn.h"

// a CSliderCtrl customized to handle large numbers. Integrates an edit to display the current number and a header for 
// some text.

class LongCSlider
{
	public:
		void initialize ( POINT& loc, CWnd* parent, int& id, int width, int height, int editID, std::string headerText );
		void rearrange ( int width, int height, fontMap fonts );
		void handleSlider( UINT nPos );
		void handleEdit();
		double getValue ( );
		void setValue (double value, bool updateEdit=true );
		int getSliderId ( );
		void reposition ( POINT loc, LONG columnWidth, LONG blockheight, LONG totalheight );
		UINT getEditId ( );
		void hide ( int hideornot );
	private:
		Control<CSliderCtrl> slider;
		Control<CEdit> edit;
		Control<CStatic> header;
		double currentValue;
		const double maxVal = 65535;
		const double minVal = 0;
};
