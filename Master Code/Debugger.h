#pragma once
#include "Control.h"


struct debuggingOptions
{
	bool showTtls;
	bool showDacs;
	ULONG sleepTime;
};


class Debugger
{
	public:
		void initialize( POINT& pos, MasterWindow* master, std::vector<CToolTipCtrl*> tooltips , int& id);
		debuggingOptions getOptions();
		HBRUSH handleColorMessage(CWnd* window, brushMap brushes, rgbMap rgbs, CDC* cDC);
		void handlePress(UINT id);
		void rearrange(UINT width, UINT height, fontMap fonts);
	private:
		debuggingOptions currentOptions;
		Control<CStatic> heading;
		Control<CButton> showTtlsButton;
		Control<CButton> showDacsButton;
		Control<CStatic> pauseText;
		Control<CEdit> pauseEdit;
};