#pragma once
//#include <windows.h>
#include "afxwin.h"
#include "ftd2xx.h"
#include <vector>
#include <array>
#include "Thrower.h"
#include "ftdiFlume.h"
#include "Expression.h"
#include "functionCombo.h"
#include "VariableStructures.h"
#include "Control.h"
#include "afxcview.h"
#include "MyListCtrl.h"
#include "Version.h"
#include <string>
#include <unordered_map>
#include "DdsSystemStructures.h"

class Script;
class MainWindow;
class AuxiliaryWindow;

class DdsSystem
{
	private:
		ddsConnectionType::type connType;
		ftdiFlume ftFlume;
		Control<CStatic> ddsHeader;
		Control<MyListCtrl> rampListview;
		bool controlActive = true;
		std::vector<ddsRampInfoForm> currentRamps;

		const UINT MSGLENGTH = 7;
		const unsigned char WBWRITE = (unsigned char) 161;
		const unsigned char WBWRITE_ARRAY = (unsigned char) 2; //Add 2 to WBWRITE
		const double INTERNAL_CLOCK = ( double ) 500.0; //Internal clock in MHz

		void longUpdate ( );
		void lockPLLs ( );
		// get (frequency/amplitude) ? word
		INT getFTW ( double freq );
		UINT getATW ( double amp );
		INT getRepsFromTime ( double time );
		INT get32bitATW ( double amp );
		double calcDeltaFreq ( double freq1, double freq2, int reps );
		double calcDeltaAmp ( double amp1, double amp2, int reps );
		void channelSelect ( UINT8 device, UINT8 channel );
		void writeFreq ( UINT8 device, UINT8 channel, double freq );
		void writeAmp ( UINT8 device, UINT8 channel, double freq );

		void writeArrResetFreq ( UINT8 device, UINT8 channel, double freq );
		void writeArrResetAmp ( UINT8 device, UINT8 channel, double amp );
		void writeArrReps ( UINT8 index, UINT16 reps );
		void writeArrDeltaFreq ( UINT8 device, UINT8 channel, UINT8 index, double deltafreq );
		void writeArrDeltaAmp ( UINT8 device, UINT8 channel, UINT8 index, double deltaamp );
		void writeDDS ( UINT8 device, UINT16 address, UINT8 dat1, UINT8 dat2, UINT8 dat3, UINT8 dat4 );

	public:
		DdsSystem(bool ftSafemode);
		void redrawListview ( );
		void handleSaveConfig ( );
		void handleOpenConfig ( );
		void handleNewConfig ( );
		void connectasync();
		void disconnect();
		void writeOneRamp( ddsBox<ddsRampInfo> boxRamp, UINT8 rampIndex );
		void writeExperiment( std::vector<ddsBox<ddsRampInfo>> ddsIndexInfo );

		std::vector<ddsBox<ddsRampInfo>> evaluateDdsInfoForm( std::vector<ddsBox<ddsRampInfoForm>> rampInfoForm);
		void initialize(POINT& pos, cToolTips& toolTips, CWnd* master, int& id, std::string title );
		void rearrange(UINT width, UINT height, fontMap fonts);
		void handleRampClick();
		void deleteRampVariable();
};

