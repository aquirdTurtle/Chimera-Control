#pragma once

#include "TtlSystem.h"
#include "DacSystem.h"
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
#include "Debugger.h"
#include "Agilent.h"
#include "commonTypes.h"
#include "StatusControl.h"

class MasterWindow : public CDialog
{
//	using CDialog::CDialog;
	DECLARE_DYNAMIC(MasterWindow);
	public:
		//static MasterWindow* InitializeWindowInfo(HINSTANCE hInstance);
		MasterWindow(UINT IDD) : CDialog(IDD), tektronics1(TEKTRONICS_AFG_1_ADDRESS), 
			tektronics2(TEKTRONICS_AFG_2_ADDRESS)
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
			masterRGBs["Light Green"] = RGB( 163, 190, 140 );
			masterRGBs["Slate Grey"] = RGB(101, 115, 126);
			masterRGBs["Pale Pink"] = RGB(180, 142, 173);
			masterRGBs["Musky Red"] = RGB(191, 97, 106);
			masterRGBs["Solarized Red"] = RGB(220,  50,  47);
			masterRGBs["Solarized Violet"] = RGB(108, 113, 196);
			masterRGBs["Solarized Cyan"] = RGB(42, 161, 152);
			masterRGBs["Solarized Green"] = RGB(133, 153,   0);
			masterRGBs["Solarized Blue"] = RGB(38, 139, 210);
			masterRGBs["Solarized Magenta"] = RGB(211,  54, 130);
			masterRGBs["Solarized Orange"] = RGB(203,  75,  22);
			masterRGBs["Solarized Yellow"] = RGB(181, 137, 0);
			masterRGBs["Slate Green"] = RGB(23, 84, 81);
			// there are less brushes because these are only used for backgrounds.
			masterBrushes["Dark Grey"] = CreateSolidBrush(masterRGBs["Dark Grey"]);
			masterBrushes["Medium Grey"] = CreateSolidBrush(masterRGBs["Medium Grey"]);
			masterBrushes["Light Grey"] = CreateSolidBrush(masterRGBs["Light Grey"]);
			masterBrushes["Green"] = CreateSolidBrush(masterRGBs["Green"]);
			masterBrushes["White"] = CreateSolidBrush(masterRGBs["White"]);
			masterBrushes["Red"] = CreateSolidBrush(masterRGBs["Red"]);
			
			/*
			(masterFonts["Normal Font"] = new CFont)
				->CreateFontA(20, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
			(masterFonts["Small Font"] = new CFont)
				->CreateFontA(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
			(masterFonts["Code Font"] = new CFont)
				->CreateFontA(16, 0, 0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Consolas"));
			(masterFonts["Small Code Font"] = new CFont)
				->CreateFontA(12, 0, 0, 0, 400, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Consolas"));
			(masterFonts["Heading Font"] = new CFont)
				->CreateFontA(18, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Old Sans Black"));
			(masterFonts["Large Heading Font"] = new CFont)
				->CreateFontA(28, 0, 0, 0, FW_DONTCARE, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Old Sans Black"));
			*/
			(masterFonts["Smaller Font Max"] = new CFont)
				->CreateFontA(25, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
			(masterFonts["Normal Font Max"] = new CFont)
				->CreateFontA(30, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
			(masterFonts["Heading Font Max"] = new CFont)
				->CreateFontA(38, 0, 0, 0, FW_DONTCARE, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Old Sans Black"));
			(masterFonts["Code Font Max"] = new CFont)
				->CreateFontA(28, 0, 0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Consolas"));
			(masterFonts["Larger Font Max"] = new CFont)
				->CreateFontA(36, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
			(masterFonts["Smaller Font Large"] = new CFont)
				->CreateFontA(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
			(masterFonts["Normal Font Large"] = new CFont)
				->CreateFontA(20, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
			(masterFonts["Heading Font Large"] = new CFont)
				->CreateFontA(28, 0, 0, 0, FW_DONTCARE, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Old Sans Black"));
			(masterFonts["Code Font Large"] = new CFont)
				->CreateFontA(16, 0, 0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Consolas"));
			(masterFonts["Larger Font Large"] = new CFont)
				->CreateFontA(40, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
			(masterFonts["Smaller Font Med"] = new CFont)
				->CreateFontA(8, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
			(masterFonts["Normal Font Med"] = new CFont)
				->CreateFontA(12, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
			(masterFonts["Heading Font Med"] = new CFont)
				->CreateFontA(16, 0, 0, 0, FW_DONTCARE, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Old Sans Black"));
			(masterFonts["Code Font Med"] = new CFont)
				->CreateFontA(10, 0, 0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Consolas"));
			(masterFonts["Larger Font Med"] = new CFont)
				->CreateFontA(22, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
			(masterFonts["Smaller Font Small"] = new CFont)
				->CreateFontA(6, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
			(masterFonts["Normal Font Small"] = new CFont)
				->CreateFontA(8, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
			(masterFonts["Heading Font Small"] = new CFont)
				->CreateFontA(12, 0, 0, 0, FW_DONTCARE, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Old Sans Black"));
			(masterFonts["Code Font Small"] = new CFont)
				->CreateFontA(7, 0, 0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Consolas"));
			(masterFonts["Larger Font Small"] = new CFont)
				->CreateFontA(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, 
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
		}
		~MasterWindow();
		virtual BOOL OnInitDialog();
		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		void OnCancel();
		void OnSize(UINT nType, int cx, int cy);
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
		void passRoundToDac();

		void ConfigVarsDblClick(NMHDR * pNotifyStruct, LRESULT * result);
		void ConfigVarsRClick(NMHDR * pNotifyStruct, LRESULT * result);
		void ConfigVarsColumnClick(NMHDR * pNotifyStruct, LRESULT * result);

		void GlobalVarDblClick( NMHDR * pNotifyStruct, LRESULT * result );
		void GlobalVarRClick( NMHDR * pNotifyStruct, LRESULT * result );
		void GlobalVarClick( NMHDR * pNotifyStruct, LRESULT * result );
		void drawVariables(UINT id, NMHDR* pNMHDR, LRESULT* pResultf);
		void handleEnter();
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
		//void SetRepetitionNumber();
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
		
		rgbMap getRGBs();
		brushMap getBrushes();
		fontMap getFonts();

		void HandlePause();
		void HandleAbort();
		void zeroTtls();
		void zeroDacs();
		void handleOptionsPress( UINT id );
		void handleAgilentOptions( UINT id );
		void handleTopBottomAgilentCombo();
		void handleAxialUWaveAgilentCombo();
		void handleFlashingAgilentCombo();

		void loadMotSettings();

		void onStatusTextMessage();
		void onErrorTextMessage();
		void handleTektronicsButtons(UINT id);

	private:
		DECLARE_MESSAGE_MAP();
		CMenu menu;
		//static LRESULT CALLBACK MasterWindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		std::string title;

		HWND masterWindowHandle;
		brushMap masterBrushes;
		rgbMap masterRGBs;
		toolTipTextMap toolTipText;
		fontMap masterFonts;
		/// control system classes
		ExperimentLogger logger;
		ExperimentManager manager;
		RunInfo systemRunningInfo;
		SocketWrapper niawgSocket;
		RhodeSchwarz RhodeSchwarzGenerator;
		Gpib gpib;
		KeyHandler masterKey;
		Debugger debugControl;
		Agilent topBottomAgilent, uWaveAxialAgilent, flashingAgilent;

		Script masterScript{ FUNCTIONS_FOLDER_LOCATION, "Master" };
		ConfigurationFileSystem profile{ PROFILES_PATH };
		NoteSystem notes;
 		TtlSystem ttlBoard;
		StatusControl errorStatus, generalStatus;
 		VariableSystem configVariables;
		VariableSystem globalVariables;
 		DacSystem dacBoards;
 		Repetitions repetitionControl;
 		MasterConfiguration masterConfig{ MASTER_CONFIGURATION_FILE_ADDRESS };
		TektronicsControl tektronics1, tektronics2;

		HINSTANCE programInstance;

		//const static UINT eStatusTextMessageID = RegisterWindowMessage( "ID_THREAD_STATUS_MESSAGE" );
		//const static UINT eErrorTextMessageID = RegisterWindowMessage( "ID_THREAD_ERROR_MESSAGE" );

		/// Friends (many friends)
		friend void ExperimentManager::startExperimentThread(MasterWindow* master);
		friend void ExperimentManager::loadMotSettings(MasterWindow* master);

		friend void ExperimentLogger::generateLog(MasterWindow* master);

		// configuration system friends
		friend void ConfigurationFileSystem::orientationChangeHandler(MasterWindow* Master);
		friend void ConfigurationFileSystem::newConfiguration(MasterWindow* Master);
		friend void ConfigurationFileSystem::openConfiguration(std::string configurationNameToOpen, MasterWindow* Master);
		friend void ConfigurationFileSystem::openExperiment(std::string experimentToOpen, MasterWindow* Master);
		friend void ConfigurationFileSystem::experimentChangeHandler(MasterWindow* Master);
		friend void ConfigurationFileSystem::saveConfiguration(MasterWindow* Master);
		friend void ConfigurationFileSystem::saveConfigurationAs(MasterWindow* Master);
		friend void ConfigurationFileSystem::renameConfiguration(MasterWindow* Master);
		friend void ConfigurationFileSystem::saveCategory(MasterWindow* Master);
		friend void ConfigurationFileSystem::newCategory(MasterWindow* Master);
		friend void ConfigurationFileSystem::saveCategoryAs(MasterWindow* Master);
		friend void ConfigurationFileSystem::openCategory(std::string categoryToOpen, MasterWindow* Master);
		friend void ConfigurationFileSystem::categoryChangeHandler(MasterWindow* Master);
		friend void ConfigurationFileSystem::saveExperiment(MasterWindow* Master);
		friend void ConfigurationFileSystem::saveExperimentAs(MasterWindow* Master);
		friend void ConfigurationFileSystem::newExperiment(MasterWindow* Master);
		friend void ConfigurationFileSystem::addToSequence(MasterWindow* Master);
		friend void ConfigurationFileSystem::loadNullSequence(MasterWindow* Master);
		friend void ConfigurationFileSystem::saveSequence(MasterWindow* Master);
		friend void ConfigurationFileSystem::saveSequenceAs(MasterWindow* Master);
		friend void ConfigurationFileSystem::renameSequence(MasterWindow* Master);
		friend void ConfigurationFileSystem::newSequence(MasterWindow* Master);
		friend void ConfigurationFileSystem::openSequence(std::string sequenceName, MasterWindow* Master);
		friend void ConfigurationFileSystem::configurationChangeHandler(MasterWindow* Master);
		// friend bool ConfigurationFileSystem::initialize(POINT& topLeftPosition, MasterWindow& Master);
		// script friends
		friend COLORREF Script::getSyntaxColor(std::string word, std::string editType, std::vector<variable> variables, 
											   rgbMap rgbs, bool& colorLine, 
											   std::array<std::array<std::string, 16>, 4> ttlNames, 
											   std::array<std::string, 24> dacNames);
		friend void Script::changeView(std::string viewName, MasterWindow* Master, bool isFunction);
		friend void Script::saveScript(MasterWindow* Master);
		friend void Script::saveScriptAs(std::string location, MasterWindow* Master);
		friend void Script::checkSave(MasterWindow* Master);
		friend void Script::renameScript(MasterWindow* Master);
		friend void Script::deleteScript(MasterWindow* Master);
		friend void Script::newScript(MasterWindow* Master);
		friend void Script::openParentScript(std::string parentScriptName, MasterWindow* Master);
		friend void Script::considerCurrentLocation(MasterWindow* Master);
		friend void Script::colorScriptSection(DWORD beginingOfChange, DWORD endOfChange, MasterWindow* Master);

};