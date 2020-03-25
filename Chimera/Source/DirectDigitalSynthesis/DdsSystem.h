#pragma once
#include "DdsCore.h"

#include "ParameterSystem/ParameterSystemStructures.h"
#include "CustomMfcControlWrappers/Control.h"
#include "CustomMfcControlWrappers/MyListCtrl.h"
#include "ConfigurationSystems/Version.h"
#include "GeneralFlumes/ftdiFlume.h"
#include "ParameterSystem/Expression.h"
#include "CustomMfcControlWrappers/functionCombo.h"
#include "DirectDigitalSynthesis/DdsSystemStructures.h"
#include "CustomMfcControlWrappers/myButton.h"
#include "GeneralObjects/ExpWrap.h"
#include "ftd2xx.h"
#include "afxwin.h"
#include "afxcview.h"
#include <vector>
#include <array>
#include <string>
#include <unordered_map>

class Script;
class MainWindow;
class AuxiliaryWindow;

class DdsSystem
{
	public:
		// THIS CLASS IS NOT COPYABLE.
		DdsSystem& operator=(const DdsSystem&) = delete;
		DdsSystem (const DdsSystem&) = delete;

		DdsSystem(bool ftSafemode);
		void redrawListview ( );
		void handleSaveConfig ( std::ofstream& file );
		void handleOpenConfig (ScriptStream& file, Version ver );
		void initialize(POINT& pos, cToolTips& toolTips, CWnd* master, int& id, std::string title );
		void rearrange(UINT width, UINT height, fontMap fonts);
		void handleRampClick();
		void deleteRampVariable();
		void programNow ( );
		std::string getSystemInfo ( );
		std::string getDelim ( );
		DdsCore& getCore ( );
		
	private:
		
		Control<CStatic> ddsHeader;
		Control<MyListCtrl> rampListview;
		Control<CleanPush> programNowButton;
		bool controlActive = true;
		std::vector<ddsIndvRampListInfo> currentRamps;
		DdsCore core;
};

