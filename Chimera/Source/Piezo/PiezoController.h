#pragma once
#include "PiezoCore.h"
#include "Control.h"
#include "CustomMfcControlWrappers/DoubleEdit.h"
#include "ConfigurationSystems/Version.h"
#include <fstream>
#include "CustomMfcControlWrappers/myButton.h"
#include "Piezo/PiezoType.h"

class PiezoController
{
	public:
		PiezoController ( PiezoType piezoControllerType, std::string sn, std::string delim);
		void initialize ( POINT& pos, cToolTips& toolTips, CWnd* parent, int& id, LONG width, UINT buttonID,
						  piezoChan<std::string> names, UINT ctrlButtonID );
		std::string getDeviceInfo ( );
		void updateCurrentValues ( );
		void handleOpenConfig ( std::ifstream& configFile, Version ver );
		void handleSaveConfig ( std::ofstream& configFile );
		PiezoCore& getCore ( );
		void updateCtrl ( );
		std::string getPiezoDeviceList ( );
		void handleProgramNowPress ( );
		void rearrange ( UINT width, UINT height, fontMap fonts);
		std::string getConfigDelim ( );

	private:
		PiezoCore core;
		Control<CButton> programNowButton;
		Control<CleanCheck> ctrlButton;
		piezoChan<Control<CStatic>> labels;
		piezoChan<Control<CEdit>> edits;
		piezoChan<Control<CStatic>> currentVals;		
};