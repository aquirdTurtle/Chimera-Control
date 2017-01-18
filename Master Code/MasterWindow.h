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
		virtual BOOL OnInitDialog();
		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		void OnCancel();
		// the master needs to handle tooltip stuff.
		std::vector<CToolTipCtrl*> toolTips;
		BOOL MasterWindow::PreTranslateMessage(MSG* pMsg);
		/// Message Map Functions
		void handleTTLPush(UINT id);
		void handlTTLHoldPush();
		void ViewOrChangeTTLNames();
		void ViewOrChangeDACNames();
		void SaveMasterConfig();
		void Exit();
		void EditChange();
		void OnTimer(UINT TimerVal);
		void ListViewDblClick(NMHDR * pNotifyStruct, LRESULT * result);
		void ListViewRClick(NMHDR * pNotifyStruct, LRESULT * result);
		void ListViewClick(NMHDR * pNotifyStruct, LRESULT * result);
		void SaveMasterScript();
		void SaveMasterScriptAs();
		void NewMasterScript();
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
		void SetRepetitionNumber();
		void ClearError();
		void ClearGeneral();
		void DAC_EditChange(UINT id);
		void SetDacs();
		void LogSettings();
		void ConfigurationNotesChange();
		void CategoryNotesChange();
		void ExperimentNotesChange();
		void StartExperiment();
		void HandleFunctionChange();
		std::unordered_map<std::string, COLORREF> getRGBs();
		void HandleAbort();

		void loadMotSettings();

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
		TtlSystem ttlBoard{ id };
		RichEditControl errorStatus{ id };
		RichEditControl generalStatus{ id };
		VariableSystem variables{ id };
		DacSystem dacBoards{ id };
		Repetitions repetitionControl{ id };
		MasterConfiguration masterConfig{ MASTER_CONFIGURATION_FILE_ADDRESS };

		HINSTANCE programInstance;

		/// Friends (many friends)
		friend bool ExperimentManager::startExperimentThread(MasterWindow* master);

		friend bool ExperimentLogger::generateLog(MasterWindow* master);
		// configuration system friends
		friend void ConfigurationFileSystem::orientationChangeHandler(MasterWindow* Master);
		friend void ConfigurationFileSystem::newConfiguration(MasterWindow* Master);
		friend void ConfigurationFileSystem::openConfiguration(std::string configurationNameToOpen, MasterWindow* Master);
		friend void ConfigurationFileSystem::openExperiment(std::string experimentToOpen, MasterWindow* Master);
		friend void ConfigurationFileSystem::experimentChangeHandler(MasterWindow* Master);
		friend void ConfigurationFileSystem::saveConfigurationOnly(MasterWindow* Master);
		friend void ConfigurationFileSystem::saveConfigurationAs(MasterWindow* Master);
		friend void ConfigurationFileSystem::renameConfiguration(MasterWindow* Master);
		friend void ConfigurationFileSystem::saveCategoryOnly(MasterWindow* Master);
		friend void ConfigurationFileSystem::newCategory(MasterWindow* Master);
		friend void ConfigurationFileSystem::saveCategoryAs(MasterWindow* Master);
		friend void ConfigurationFileSystem::openCategory(std::string categoryToOpen, MasterWindow* Master);
		friend void ConfigurationFileSystem::categoryChangeHandler(MasterWindow* Master);
		friend void ConfigurationFileSystem::saveExperimentOnly(MasterWindow* Master);
		friend void ConfigurationFileSystem::saveExperimentAs(MasterWindow* Master);
		friend void ConfigurationFileSystem::newExperiment(MasterWindow* Master);
		friend void ConfigurationFileSystem::addToSequence(MasterWindow* Master);
		friend void ConfigurationFileSystem::loadNullSequence(MasterWindow* Master);
		friend void ConfigurationFileSystem::saveSequence(MasterWindow* Master);
		friend void ConfigurationFileSystem::saveSequenceAs(MasterWindow* Master);
		friend void ConfigurationFileSystem::renameSequence(MasterWindow* Master);
		friend void ConfigurationFileSystem::newSequence(MasterWindow* Master);
		friend void ConfigurationFileSystem::openSequence(std::string sequenceName, MasterWindow* Master);
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