#pragma once
#include <unordered_map>
#include <string>

#include "constants.h"
#include "MasterManager.h"
#include "TtlSystem.h"
#include "DacSystem.h"
#include "VariableSystem.h"
#include "Script.h"
#include "RunInfo.h"
#include "RichEditControl.h"
#include "Repetitions.h"
#include "SocketWrapper.h"
#include "RhodeSchwarz.h"
#include "GPIB.h"
#include "MasterConfiguration.h"
#include "ExperimentLogger.h"
#include "KeyHandler.h"
#include "Agilent.h"
#include "commonTypes.h"
#include "StatusControl.h"
#include "TektronicsControl.h"

// The Device window houses most of the controls for seeting individual devices, other than the camera which gets its 
// own control. It also houses a couple auxiliary things like variables and the SMS texting control.
class DeviceWindow : public CDialog
{
	DECLARE_DYNAMIC(DeviceWindow);

	public:
		DeviceWindow() : CDialog(), tektronics1(TEKTRONICS_AFG_1_ADDRESS), tektronics2(TEKTRONICS_AFG_2_ADDRESS) { }
		virtual BOOL OnInitDialog();
		void handleOpeningConfig(std::ifstream& configFile, double version);
		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		void OnCancel();
		void OnSize(UINT nType, int cx, int cy);
		void passCommonCommand(UINT id);
		void handleMasterConfigSave(std::stringstream& configStream);
		void handleMasterConfigOpen(std::stringstream& configStream, double version);
		// the master needs to handle tooltip stuff.
		cToolTips toolTips;
		BOOL PreTranslateMessage(MSG* pMsg);
		/// Message Map Functions
		void handleTtlPush(UINT id);
		void handlTtlHoldPush();
		void ViewOrChangeTTLNames();
		void ViewOrChangeDACNames();
		void Exit();
		void passRoundToDac();
		void getFriends(MainWindow* mainWin, ScriptingWindow* scriptWin, CameraWindow* camWin);
		std::string getSystemStatusMsg();
		std::array<std::array<std::string, 16>, 4> getTtlNames();
		std::array<std::string, 24> getDacNames();

		void drawVariables(UINT id, NMHDR* pNMHDR, LRESULT* pResultf);
		void handleEnter();
		void fillMasterThreadInput(MasterThreadInput* input);
		void changeBoxColor(systemInfo<char> colors);
		void DacEditChange(UINT id);
		void SetDacs();
		void LogSettings();
		void StartExperiment();
		
		fontMap getFonts();

		void handleAbort();
		void zeroTtls();
		void zeroDacs();

		void handleAgilentOptions( UINT id );
		void handleTopBottomAgilentCombo();
		void handleAxialUWaveAgilentCombo();
		void handleFlashingAgilentCombo();

		void loadMotSettings();
		void handleTektronicsButtons(UINT id);

		void sendErr(std::string msg);
		void sendStat(std::string msg);

		std::vector<variable> getAllVariables();

		void GlobalVarDblClick(NMHDR * pNotifyStruct, LRESULT * result);
		void GlobalVarRClick(NMHDR * pNotifyStruct, LRESULT * result);
		void ConfigVarsColumnClick(NMHDR * pNotifyStruct, LRESULT * result);
		void clearVariables();
		void addVariable(std::string name, bool timelike, bool singleton, double value, int item);
		void ConfigVarsDblClick(NMHDR * pNotifyStruct, LRESULT * result);
		void ConfigVarsRClick(NMHDR * pNotifyStruct, LRESULT * result);

		UINT getTotalVariationNumber();
		void handleSaveConfig(std::ofstream& saveFile);
		std::pair<UINT, UINT> getTtlBoardSize();
		UINT getNumberOfDacs();
		void setConfigActive(bool active);

	private:
		DECLARE_MESSAGE_MAP();		

		MainWindow* mainWindowFriend;
		ScriptingWindow* scriptingWindowFriend;
		CameraWindow* cameraWindowFriend;

		CMenu menu;
		std::string title;
		
		toolTipTextMap toolTipText;

		/// control system classes
		ExperimentLogger logger;
		MasterManager manager;
		SocketWrapper niawgSocket;
		RhodeSchwarz RhodeSchwarzGenerator;
		Gpib gpib;
		KeyHandler masterKey;
		Agilent topBottomAgilent, uWaveAxialAgilent, flashingAgilent;
 		TtlSystem ttlBoard;
 		DacSystem dacBoards;
 		MasterConfiguration masterConfig{ MASTER_CONFIGURATION_FILE_ADDRESS };
		TektronicsControl tektronics1, tektronics2;
		ColorBox boxes;
		VariableSystem configVariables;
		VariableSystem globalVariables;

		ColorBox statusBox;

};
