#pragma once
#include "MasterManager.h"
#include <unordered_map>
#include <string>
#include "TtlSystem.h"
#include "DacSystem.h"
#include "VariableSystem.h"
#include "Script.h"
#include "NoteSystem.h"
#include "ConfigurationFileSystem.h"
#include "constants.h"
#include "RunInfo.h"
#include "RichEditControl.h"
#include "Repetitions.h"
#include "SocketWrapper.h"
#include "RhodeSchwarz.h"
#include "GPIB.h"
#include "MasterConfiguration.h"
#include "ExperimentLogger.h"
#include "ExperimentManager.h"
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
		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		void OnCancel();
		void OnSize(UINT nType, int cx, int cy);
		void passCommonCommand(UINT id);
		void handleMasterConfigSave(std::stringstream& configStream);
		void handleMasterConfigOpen(std::stringstream& configStream, double version);
		// the master needs to handle tooltip stuff.
		std::vector<CToolTipCtrl*> toolTips;
		BOOL PreTranslateMessage(MSG* pMsg);
		/// Message Map Functions
		void handleTtlPush(UINT id);
		void handlTtlHoldPush();
		void ViewOrChangeTTLNames();
		void ViewOrChangeDACNames();
		void SaveMasterConfig();
		void Exit();
		void EditChange();
		void OnTimer(UINT TimerVal);
		void passRoundToDac();
		void getFriends(MainWindow* mainWin, ScriptingWindow* scriptWin, CameraWindow* camWin);
		std::array<std::array<std::string, 16>, 4> getTtlNames();
		std::array<std::string, 24> getDacNames();

		/*
		void ConfigVarsDblClick(NMHDR * pNotifyStruct, LRESULT * result);
		void ConfigVarsRClick(NMHDR * pNotifyStruct, LRESULT * result);
		void ConfigVarsColumnClick(NMHDR * pNotifyStruct, LRESULT * result);

		void GlobalVarDblClick( NMHDR * pNotifyStruct, LRESULT * result );
		void GlobalVarRClick( NMHDR * pNotifyStruct, LRESULT * result );
		void GlobalVarClick( NMHDR * pNotifyStruct, LRESULT * result );
		*/

		void drawVariables(UINT id, NMHDR* pNMHDR, LRESULT* pResultf);
		void handleEnter();
		/*
		void SaveMasterScript();
		void SaveMasterScriptAs();
		void NewMasterScript();
		void NewMasterFunction();
		void OpenMasterScript();
		void SaveMasterFunction();
		void DeleteConfiguration();
		void NewConfiguration();
		void SaveEntireProfile();
		void SaveConfiguration();
		void SaveConfigurationAs();
		void RenameConfiguration();
		void SaveCategory();
		void SaveCategoryAs();
		void RenameCategory();
		void DeleteCategory();
		void NewCategory();
		void NewExperiment();
		void DeleteExperiment();
		void SaveExperiment();
		void SaveExperimentAs();
		void AddToSequence();
		void DeleteSequence();
		void ResetSequence();
		void RenameSequence();
		void NewSequence();
		
		void OnExperimentChanged();
		void OnCategoryChanged();
		void OnConfigurationChanged();
		void OnSequenceChanged();
		void OnOrientationChanged();
		void ClearError();
		void ClearGeneral();
		*/
		void DacEditChange(UINT id);
		void SetDacs();
		void LogSettings();
		/*
		void ConfigurationNotesChange();
		void CategoryNotesChange();
		void ExperimentNotesChange();
		*/
		void StartExperiment();
		// void HandleFunctionChange();
		
		rgbMap getRGBs();
		brushMap getBrushes();
		fontMap getFonts();

		void HandlePause();
		void HandleAbort();
		void zeroTtls();
		void zeroDacs();
		//void handleOptionsPress( UINT id );
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

		/// Friends (many friends)
		//5friend void MasterManager::loadMotSettings(DeviceWindow* deviceWin);
		//friend void MasterManager::startExperimentThread(DeviceWindow* deviceWin);
		/*
		friend void ExperimentManager::startExperimentThread(DeviceWindow* master);
		friend void ExperimentManager::loadMotSettings(DeviceWindow* master);

		friend void ExperimentLogger::generateLog(DeviceWindow* master);

		// configuration system friends
		friend void ConfigurationFileSystem::orientationChangeHandler(DeviceWindow* Master);
		friend void ConfigurationFileSystem::newConfiguration(DeviceWindow* Master);
		friend void ConfigurationFileSystem::openConfiguration(std::string configurationNameToOpen, DeviceWindow* Master);
		friend void ConfigurationFileSystem::openExperiment(std::string experimentToOpen, DeviceWindow* Master);
		friend void ConfigurationFileSystem::experimentChangeHandler(DeviceWindow* Master);
		friend void ConfigurationFileSystem::saveConfiguration(DeviceWindow* Master);
		friend void ConfigurationFileSystem::saveConfigurationAs(DeviceWindow* Master);
		friend void ConfigurationFileSystem::renameConfiguration(DeviceWindow* Master);
		friend void ConfigurationFileSystem::saveCategory(DeviceWindow* Master);
		friend void ConfigurationFileSystem::newCategory(DeviceWindow* Master);
		friend void ConfigurationFileSystem::saveCategoryAs(DeviceWindow* Master);
		friend void ConfigurationFileSystem::openCategory(std::string categoryToOpen, DeviceWindow* Master);
		friend void ConfigurationFileSystem::categoryChangeHandler(DeviceWindow* Master);
		friend void ConfigurationFileSystem::saveExperiment(DeviceWindow* Master);
		friend void ConfigurationFileSystem::saveExperimentAs(DeviceWindow* Master);
		friend void ConfigurationFileSystem::newExperiment(DeviceWindow* Master);
		friend void ConfigurationFileSystem::addToSequence(DeviceWindow* Master);
		friend void ConfigurationFileSystem::loadNullSequence(DeviceWindow* Master);
		friend void ConfigurationFileSystem::saveSequence(DeviceWindow* Master);
		friend void ConfigurationFileSystem::saveSequenceAs(DeviceWindow* Master);
		friend void ConfigurationFileSystem::renameSequence(DeviceWindow* Master);
		friend void ConfigurationFileSystem::newSequence(DeviceWindow* Master);
		friend void ConfigurationFileSystem::openSequence(std::string sequenceName, DeviceWindow* Master);
		friend void ConfigurationFileSystem::configurationChangeHandler(DeviceWindow* Master);
		// friend bool ConfigurationFileSystem::initialize(POINT& topLeftPosition, DeviceWindow& Master);
		*/
};
