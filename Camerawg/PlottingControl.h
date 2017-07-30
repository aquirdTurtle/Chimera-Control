#pragma once

#include "Control.h"

struct cameraPositions;

class PlottingControl
{
	public:
		void initialize(cameraPositions& pos, int& id, CWnd* parent);
		void handleClick();
		void addPlot();
		void openPlotter();
	private:
		Control<CStatic> updateFrequencyLabel1;
		Control<CStatic> updateFrequencyLabel2;
		Control<CEdit> updateFrequencyEdit;
		Control<CStatic> header;
		Control<CListCtrl> plotListview;
		CDialog plotDesigner;
};