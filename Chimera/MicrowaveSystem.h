// created by Mark O. Brown
#pragma once

#include "GpibFlume.h"
#include "MyListCtrl.h"
#include "Control.h"
#include "MicrowaveCore.h"
#include <vector>
#include <string>
#include <unordered_map>

class AuxiliaryWindow;


/**/
class MicrowaveSystem
{
	public:
		MicrowaveSystem();
		void initialize( POINT& pos, cToolTips& toolTips, AuxiliaryWindow* parentWin, int& id );
		void setInfoDisp( UINT variation );
		std::string getIdentity();
		void rearrange(UINT width, UINT height, fontMap fonts);
		MicrowaveCore& getCore ();
	private:
		MicrowaveCore core;
		Control<CStatic> header;
		// this is a read-only control that shows the user how the RSG actually gets programmed in the end.
		Control<MyListCtrl> infoControl;
		//Control<CListCtrl> infoControl;
};

