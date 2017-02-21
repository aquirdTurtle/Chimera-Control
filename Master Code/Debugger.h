#pragma once
#include "Control.h"


struct debuggingOptions
{
	bool showTtls;
	bool showDacs;
};


class Debugger
{
	public:
		void initialize( POINT& pos, MasterWindow* master, std::vector<CToolTipCtrl*> tooltips , int& id);
		debuggingOptions getOptions();
		void handlePress(UINT id);

	private:
		
		debuggingOptions currentOptions;

		Control<CStatic> heading;
		Control<CButton> showTtlsButton;
		Control<CButton> showDacsButton;

};