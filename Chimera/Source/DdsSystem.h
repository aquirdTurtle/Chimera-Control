#pragma once
#include "DdsCore.h"

#include "VariableStructures.h"
#include "Control.h"
#include "MyListCtrl.h"
#include "Version.h"
#include "ftdiFlume.h"
#include "Expression.h"
#include "functionCombo.h"
#include "DdsSystemStructures.h"
#include "myButton.h"
#include "ExpWrap.h"
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
		DdsSystem(bool ftSafemode);
		void redrawListview ( );
		void handleSaveConfig ( std::ofstream& file );
		void handleOpenConfig ( std::ifstream& file, Version ver );
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

