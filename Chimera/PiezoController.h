#pragma once
#include "PiezoCore.h"
#include "Control.h"
#include "DoubleEdit.h"
#include "Version.h"
#include <fstream>

class PiezoController
{
	public:
		PiezoController ( bool safemode, std::string sn, std::string delim);
		void initialize ( POINT& pos, cToolTips& toolTips, CWnd* parent, int& id, LONG width, UINT buttonID,
						  piezoChan<std::string> names );
		std::string getDeviceInfo ( );
		void updateCurrentValues ( );
		void handleOpenConfig ( std::ifstream& configFile, Version ver );
		void handleSaveConfig ( std::ofstream& configFile );
		PiezoCore& getCore ( );
		std::string getPiezoDeviceList ( );
		void handleProgramNowPress ( );
		void rearrange ( UINT width, UINT height, fontMap fonts);
		std::string getConfigDelim ( );
	private:
		
		PiezoCore core;
		Control<CButton> programNowButton;
		piezoChan<Control<CStatic>> labels;
		piezoChan<Control<CEdit>> edits;
		piezoChan<Control<CStatic>> currentVals;		
};