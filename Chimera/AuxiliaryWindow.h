#pragma once
#include <unordered_map>
#include <string>

#include "constants.h"
#include "MasterManager.h" 
#include "DioSystem.h"
#include "DacSystem.h"
#include "VariableSystem.h"
#include "Script.h"
#include "RunInfo.h"
#include "RichEditControl.h"
#include "Repetitions.h"
#include "SocketWrapper.h"
#include "RhodeSchwarz.h"
#include "GpibFlume.h"
#include "MasterConfiguration.h"
#include "Agilent.h"
#include "commonTypes.h"
#include "StatusControl.h"
#include "TektronicsControl.h"

enum agilentNames
{
	TopBottom,
	Axial,
	Flashing,
	Microwave
};


// The Device window houses most of the controls for seeting individual devices, other than the camera which gets its 
// own control. It also houses a couple auxiliary things like variables and the SMS texting control.
class AuxiliaryWindow : public CDialog
{
	DECLARE_DYNAMIC(AuxiliaryWindow);

	public:
		AuxiliaryWindow();
		BOOL OnInitDialog();
		void handleOpeningConfig(std::ifstream& configFile, int versionMajor, int versionMinor );
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
		void OnTimer( UINT_PTR eventID );
		void handleTtlPush(UINT id);
		void handlTtlHoldPush();
		void ViewOrChangeTTLNames();
		void ViewOrChangeDACNames();
		void Exit();
		void passRoundToDac();
		void loadFriends(MainWindow* mainWin, ScriptingWindow* scriptWin, CameraWindow* camWin);
		std::string getSystemStatusMsg();
		std::array<std::array<std::string, 16>, 4> getTtlNames();
		std::array<std::string, 24> getDacNames();

		void updateAgilent( agilentNames name );
		void newAgilentScript( agilentNames name );
		void openAgilentScript( agilentNames name, CWnd* parent );
		void saveAgilentScript( agilentNames name );
		void saveAgilentScriptAs( agilentNames name, CWnd* parent );
		void handleAgilentEditChange( UINT id );

		void drawVariables(UINT id, NMHDR* pNMHDR, LRESULT* pResultf);
		void handleEnter();
		void fillMasterThreadInput(MasterThreadInput* input);
		void changeBoxColor(systemInfo<char> colors);
		void DacEditChange(UINT id);
		void SetDacs();
		
		fontMap getFonts();

		void handleAbort();
		void zeroTtls();
		void zeroDacs();

		void handleAgilentOptions( UINT id );

		void loadMotSettings(MasterThreadInput* input);
		void handleTektronicsButtons(UINT id);

		void sendErr(std::string msg);
		void sendStatus(std::string msg);

		std::vector<variableType> getAllVariables();

		void GlobalVarDblClick(NMHDR * pNotifyStruct, LRESULT * result);
		void GlobalVarRClick(NMHDR * pNotifyStruct, LRESULT * result);
		void ConfigVarsColumnClick(NMHDR * pNotifyStruct, LRESULT * result);
		void clearVariables();
		void addVariable(std::string name, bool constant, double value, int item);
		void ConfigVarsDblClick(NMHDR * pNotifyStruct, LRESULT * result);
		void ConfigVarsRClick(NMHDR * pNotifyStruct, LRESULT * result);

		UINT getTotalVariationNumber();
		void handleNewConfig( std::ofstream& newFile );
		void handleSaveConfig(std::ofstream& saveFile);
		std::pair<UINT, UINT> getTtlBoardSize();
		UINT getNumberOfDacs();
		void setVariablesActiveState(bool active);
		void passTopBottomTekProgram();
		void passEoAxialTekProgram();
		Agilent& whichAgilent( UINT id );
		void handleAgilentCombo( UINT id );

	private:
		DECLARE_MESSAGE_MAP();		

		MainWindow* mainWindowFriend;
		ScriptingWindow* scriptingWindowFriend;
		CameraWindow* cameraWindowFriend;

		CMenu menu;
		std::string title;
		
		toolTipTextMap toolTipText;

		/// control system classes
		RhodeSchwarz RhodeSchwarzGenerator;
		// 
		std::array<Agilent, 4> agilents;
		
 		DioSystem ttlBoard;
 		DacSystem dacBoards;
 		MasterConfiguration masterConfig{ MASTER_CONFIGURATION_FILE_ADDRESS };
		TektronicsControl topBottomTek, eoAxialTek;

		ColorBox boxes;
		VariableSystem configVariables;
		VariableSystem globalVariables;

		ColorBox statusBox;
};
