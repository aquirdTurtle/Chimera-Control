#pragma once
#include "Control.h"

struct mainOptions
{
	bool connectToMaster;
	bool getVariables;
	bool programIntensity;
};

class MainOptionsControl
{
	public:
		void initialize(int& idStart, POINT& loc, CWnd* parent);
		bool handleEvent(HWND parent, UINT msg, WPARAM wParam, LPARAM lParam, MainWindow* mainWin);
		mainOptions getOptions();
		void setOptions(mainOptions options);
	private:
		Control<CStatic> header;
		Control<CButton> connectToMaster;
		Control<CButton> getVariables;
		Control<CButton> controlIntensity;
		mainOptions currentOptions;
};