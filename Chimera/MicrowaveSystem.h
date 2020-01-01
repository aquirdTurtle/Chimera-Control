// created by Mark O. Brown
#pragma once

#include "GpibFlume.h"
#include "MyListCtrl.h"
#include "Control.h"
#include "MicrowaveCore.h"
#include "MicrowaveSettings.h"
#include "myButton.h"
#include "Version.h"
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
		void handleListviewDblClick ();
		std::string getIdentity();
		void rearrange(UINT width, UINT height, fontMap fonts);
		MicrowaveCore& getCore ();
		void handleSaveConfig (std::ofstream& saveFile);
		static const std::string delim;
		static microwaveSettings getMicrowaveSettingsFromConfig (std::ifstream& openFile, Version ver);
		void setMicrowaveSettings (microwaveSettings settings);
	private:
		MicrowaveCore core;
		Control<CStatic> header;
		// this is a read-only control that shows the user how the RSG actually gets programmed in the end.
		Control<MyListCtrl> uwListListview;
		Control<CleanCheck> controlOption;
		Control<CleanPush> programNow;
		std::vector<microwaveListEntry> currentList;
		void refreshListview ();
};

