// created by Mark O. Brown
#pragma once

#include "GeneralFlumes/GpibFlume.h"
#include "CustomMfcControlWrappers/MyListCtrl.h"
#include "CustomMfcControlWrappers/Control.h"
#include "Microwave/MicrowaveCore.h"
#include "Microwave/MicrowaveSettings.h"
#include "CustomMfcControlWrappers/myButton.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
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
		void handleListviewRClick ();
		std::string getIdentity();
		void rearrange(UINT width, UINT height, fontMap fonts);
		MicrowaveCore& getCore ();
		void handleSaveConfig (ConfigStream& saveFile);
		static const std::string delim;
		void setMicrowaveSettings (microwaveSettings settings);
		void programNow (std::vector<parameterType> constants);
	private:
		MicrowaveCore core;
		Control<CStatic> header;
		// this is a read-only control that shows the user how the RSG actually gets programmed in the end.
		Control<MyListCtrl> uwListListview;
		Control<CleanCheck> controlOptionCheck;
		Control<CleanPush> programNowPush;
		std::vector<microwaveListEntry> currentList;
		void refreshListview ();
};

