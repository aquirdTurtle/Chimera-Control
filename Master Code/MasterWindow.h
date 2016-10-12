#pragma once
#include "TTL_System.h"
#include "DAC_System.h"
#include "VariableSystem.h"
#include "Script.h"
#include "NoteSystem.h"
#include "ConfigurationFileSystem.h"
#include <string>
#include "constants.h"
#include "RunInfo.h"
#include "RichEditControl.h"
#include "Repetitions.h"
#include "SocketWrapper.h"
#include "RhodeSchwarz.h"
#include "GPIB.h"
#include <unordered_map>
#include "MasterConfiguration.h"
#include "ExperimentLogger.h"
#include <unordered_map>
#include "ExperimentManager.h"
#include "KeyHandler.h"

class MasterWindow : public CDialog
{
//	using CDialog::CDialog;
	DECLARE_DYNAMIC(MasterWindow);
	public:
		//static MasterWindow* InitializeWindowInfo(HINSTANCE hInstance);
		MasterWindow(UINT IDD) : CDialog(IDD)
		{
			masterRGBs["Dark Grey"] = (RGB(15, 15, 15));
			masterRGBs["Medium Grey"] = (RGB(30, 30, 30));
			masterRGBs["Light Grey"] = (RGB(60, 60, 60));
			masterRGBs["Green"] = (RGB(50, 200, 50));
			masterRGBs["Red"] = (RGB(200, 50, 50));
			masterRGBs["Blue"] = RGB(50, 50, 200);
			masterRGBs["Gold"] = RGB(218, 165, 32);
			masterRGBs["White"] = RGB(255, 255, 255);
			masterRGBs["Light Red"] = (RGB(255, 100, 100));
			masterRGBs["Light Blue"] = RGB(100, 100, 255);
			masterRGBs["Forest Green"] = RGB(34, 139, 34);
			masterRGBs["Dull Red"] = RGB(107, 35, 35);
			masterRGBs["Dark Lavender"] = RGB(100, 100, 205);
			masterRGBs["Teal"] = RGB(0, 255, 255);
			masterRGBs["Tan"] = RGB(210, 180, 140); 
			masterRGBs["Purple"] = RGB(147, 112, 219);
			masterRGBs["Orange"] = RGB(255, 165, 0);
			masterRGBs["Brown"] = RGB(139, 69, 19);
			masterRGBs["Black"] = RGB(0, 0, 0);
			// there are less brushes because these are only used for backgrounds.
			masterBrushes["Dark Grey"] = CreateSolidBrush(masterRGBs["Dark Grey"]);
			masterBrushes["Medium Grey"] = CreateSolidBrush(masterRGBs["Medium Grey"]);
			masterBrushes["Light Grey"] = CreateSolidBrush(masterRGBs["Light Grey"]);
			masterBrushes["Green"] = CreateSolidBrush(masterRGBs["Green"]);
			masterBrushes["Red"] = CreateSolidBrush(masterRGBs["Red"]);
			masterBrushes["White"] = CreateSolidBrush(masterRGBs["White"]);
			
		}
		~MasterWindow();
		virtual BOOL MasterWindow::OnInitDialog();
		HBRUSH MasterWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		// the master needs to handle tooltip stuff.
		std::vector<CToolTipCtrl*> toolTips;
		BOOL MasterWindow::PreTranslateMessage(MSG* pMsg);
		/// Message Map Functions
		afx_msg void handleTTLPush(UINT id);
		afx_msg void handlTTLHoldPush();
		afx_msg void ViewOrChangeTTLNames();
		afx_msg void ViewOrChangeDACNames();
		afx_msg void SaveMasterConfig();
		afx_msg void Exit();
		afx_msg void EditChange();
		afx_msg void OnTimer(UINT TimerVal);
		afx_msg void ListViewDblClick(NMHDR * pNotifyStruct, LRESULT * result);
		afx_msg void ListViewRClick(NMHDR * pNotifyStruct, LRESULT * result);
		afx_msg void ListViewClick(NMHDR * pNotifyStruct, LRESULT * result);
		afx_msg void SaveMasterScript();
		afx_msg void SaveMasterScriptAs();
		afx_msg void NewMasterScript();
		afx_msg void OpenMasterScript();
		afx_msg void SaveMasterFunction();
		afx_msg void DeleteConfiguration();
		afx_msg void NewConfiguration();
		afx_msg void SaveEntireProfile();
		afx_msg void SaveConfiguration();
		afx_msg void SaveConfigurationAs();
		afx_msg void RenameConfiguration();
		afx_msg void SaveCategory();
		afx_msg void SaveCategoryAs();
		afx_msg void RenameCategory();
		afx_msg void DeleteCategory();
		afx_msg void NewCategory();
		afx_msg void NewExperiment();
		afx_msg void DeleteExperiment();
		afx_msg void SaveExperiment();
		afx_msg void SaveExperimentAs();
		afx_msg void AddToSequence();
		afx_msg void DeleteSequence();
		afx_msg void ResetSequence();
		afx_msg void RenameSequence();
		afx_msg void NewSequence();
		afx_msg void OnExperimentChanged();
		afx_msg void OnCategoryChanged();
		afx_msg void OnConfigurationChanged();
		afx_msg void OnSequenceChanged();
		afx_msg void OnOrientationChanged();
		afx_msg void SetRepetitionNumber();
		afx_msg void ClearError();
		afx_msg void ClearGeneral();
		afx_msg void DAC_EditChange(UINT id);
		afx_msg void SetDacs();
		afx_msg void LogSettings();
		afx_msg void ConfigurationNotesChange();
		afx_msg void CategoryNotesChange();
		afx_msg void ExperimentNotesChange();
		afx_msg void StartExperiment();
		afx_msg void HandleFunctionChange();
		std::unordered_map<std::string, COLORREF> getRGBs();
	private:
		DECLARE_MESSAGE_MAP();
		CMenu menu;
		LRESULT MasterWindowMessageHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		//static LRESULT CALLBACK MasterWindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		std::string title;
		int InitializeWindowControls();
		HWND masterWindowHandle;
		std::unordered_map<std::string, HBRUSH> masterBrushes;
		std::unordered_map<std::string, COLORREF> masterRGBs;
		std::unordered_map<HWND, std::string> toolTipText;
		/// control system classes
		ExperimentLogger logger;
		ExperimentManager manager;
		RunInfo systemRunningInfo;
		SocketWrapper niawgSocket;
		RhodeSchwarz RhodeSchwarzGenerator;
		GPIB gpibHandler;
		KeyHandler masterKey;

		int id = 1200;
		Script masterScript{ FUNCTIONS_FOLDER_LOCATION, "Master", id };
		ConfigurationFileSystem profile{ PROFILES_PATH, id };
		NoteSystem notes{ id };
		TTL_System ttlBoard{ id };
		RichEditControl generalStatus{ id };
		RichEditControl errorStatus{ id };
		VariableSystem variables{ id };
		DAC_System dacBoards{ id };
		Repetitions repetitionControl{ id };
		MasterConfiguration masterConfig{ MASTER_CONFIGURATION_FILE_ADDRESS };

		HINSTANCE programInstance;

		/// Friends (many friends)
		friend bool ExperimentManager::startExperimentThread(MasterWindow* master);

		friend bool ExperimentLogger::generateLog(MasterWindow* master);
		// configuration system friends
		friend bool ConfigurationFileSystem::orientationChangeHandler(MasterWindow* Master);
		friend bool ConfigurationFileSystem::newConfiguration(MasterWindow* Master);
		friend bool ConfigurationFileSystem::openConfiguration(std::string configurationNameToOpen, MasterWindow* Master);
		friend bool ConfigurationFileSystem::openExperiment(std::string experimentToOpen, MasterWindow* Master);
		friend bool ConfigurationFileSystem::experimentChangeHandler(MasterWindow* Master);
		friend bool ConfigurationFileSystem::saveConfigurationOnly(MasterWindow* Master);
		friend bool ConfigurationFileSystem::saveConfigurationAs(MasterWindow* Master);
		friend bool ConfigurationFileSystem::renameConfiguration(MasterWindow* Master);
		friend bool ConfigurationFileSystem::saveCategoryOnly(MasterWindow* Master);
		friend bool ConfigurationFileSystem::newCategory(MasterWindow* Master);
		friend bool ConfigurationFileSystem::saveCategoryAs(MasterWindow* Master);
		friend bool ConfigurationFileSystem::openCategory(std::string categoryToOpen, MasterWindow* Master);
		friend bool ConfigurationFileSystem::categoryChangeHandler(MasterWindow* Master);
		friend bool ConfigurationFileSystem::saveExperimentOnly(MasterWindow* Master);
		friend bool ConfigurationFileSystem::saveExperimentAs(MasterWindow* Master);
		friend bool ConfigurationFileSystem::newExperiment(MasterWindow* Master);
		friend bool ConfigurationFileSystem::addToSequence(MasterWindow* Master);
		friend bool ConfigurationFileSystem::loadNullSequence(MasterWindow* Master);
		friend bool ConfigurationFileSystem::saveSequence(MasterWindow* Master);
		friend bool ConfigurationFileSystem::saveSequenceAs(MasterWindow* Master);
		friend bool ConfigurationFileSystem::renameSequence(MasterWindow* Master);
		friend bool ConfigurationFileSystem::newSequence(MasterWindow* Master);
		friend bool ConfigurationFileSystem::openSequence(std::string sequenceName, MasterWindow* Master);
		// friend bool ConfigurationFileSystem::initialize(POINT& topLeftPosition, MasterWindow& Master);
		// script friends
		friend COLORREF Script::getSyntaxColor(std::string word, std::string editType, std::vector<variable> variables, std::unordered_map<std::string, COLORREF> rgbs, bool& colorLine, std::array<std::array<std::string, 16>, 4> ttlNames, std::array<std::string, 24> dacNames);
		friend bool Script::updateChildCombo(MasterWindow* Master);
		friend bool Script::changeView(std::string viewName, MasterWindow* Master, bool isFunction);
		friend bool Script::saveScript(MasterWindow* Master);
		friend bool Script::saveScriptAs(std::string location, MasterWindow* Master);
		friend bool Script::checkChildSave(MasterWindow* Master);
		friend bool Script::checkSave(MasterWindow* Master);
		friend bool Script::renameScript(MasterWindow* Master);
		friend bool Script::deleteScript(MasterWindow* Master);
		friend bool Script::newScript(MasterWindow* Master);
		friend bool Script::openParentScript(std::string parentScriptName, MasterWindow* Master);
		friend bool Script::considerCurrentLocation(MasterWindow* Master);
		friend bool Script::colorScriptSection(DWORD beginingOfChange, DWORD endOfChange, MasterWindow* Master);
};