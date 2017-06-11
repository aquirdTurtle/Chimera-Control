#pragma once
#include <afxsock.h>
#include "stdafx.h"
#include "errBox.h"
#include "ConfigurationFileSystem.h"
#include "DebuggingOptionsControl.h"
#include "constants.h"
#include "commonFunctions.h"
#include "MainOptionsControl.h"
#include "StatusControl.h"
#include "StatusIndicator.h"
#include "Communicator.h"
#include "SMSTextingControl.h"
#include "NiawgController.h"
#include "ExperimentManager.h"
#include "commonTypes.h"

class ScriptingWindow;
class CameraWindow;

class MainWindow : public CDialog
{
	using CDialog::CDialog;
	DECLARE_DYNAMIC(MainWindow);
	public:
		MainWindow( UINT id ) : CDialog( id ), profile( PROFILES_PATH )
		{
			mainRGBs["Dark Grey"] = RGB( 15, 15, 15 );
			mainRGBs["Dark Grey Red"] = RGB( 20, 12, 12 );
			mainRGBs["Medium Grey"] = RGB( 30, 30, 30 );
			mainRGBs["Light Grey"] = RGB( 60, 60, 60 );
			mainRGBs["Green"] = RGB( 50, 200, 50 );
			mainRGBs["Red"] = RGB( 200, 50, 50 );
			mainRGBs["Blue"] = RGB( 50, 50, 200 );
			mainRGBs["Gold"] = RGB( 218, 165, 32 );
			mainRGBs["White"] = RGB( 255, 255, 255 );
			mainRGBs["Light Red"] = RGB( 255, 100, 100 );
			mainRGBs["Dark Red"] = RGB( 150, 0, 0 );
			mainRGBs["Light Blue"] = RGB( 100, 100, 255 );
			mainRGBs["Forest Green"] = RGB( 34, 139, 34 );
			mainRGBs["Dark Green"] = RGB( 0, 50, 0 );
			mainRGBs["Dull Red"] = RGB( 107, 35, 35 );
			mainRGBs["Dark Lavender"] = RGB( 100, 100, 205 );
			mainRGBs["Teal"] = RGB( 0, 255, 255 );
			mainRGBs["Tan"] = RGB( 210, 180, 140 );
			mainRGBs["Purple"] = RGB( 147, 112, 219 );
			mainRGBs["Orange"] = RGB( 255, 165, 0 );
			mainRGBs["Brown"] = RGB( 139, 69, 19 );
			mainRGBs["Black"] = RGB( 0, 0, 0 );
			mainRGBs["Dark Blue"] = RGB( 0, 0, 75 );
			// there are less brushes because these are only used for backgrounds.
			mainBrushes["Dark Red"] = new CBrush;
			mainBrushes["Dark Red"]->CreateSolidBrush( mainRGBs["Dark Red"] );
			mainBrushes["Gold"] = new CBrush;
			mainBrushes["Gold"]->CreateSolidBrush( mainRGBs["Gold"] );
			mainBrushes["Dark Grey"] = new CBrush;
			mainBrushes["Dark Grey"]->CreateSolidBrush( mainRGBs["Dark Grey"] );
			mainBrushes["Dark Grey Red"] = new CBrush;
			mainBrushes["Dark Grey Red"]->CreateSolidBrush( mainRGBs["Dark Grey Red"] );
			mainBrushes["Medium Grey"] = new CBrush;
			mainBrushes["Medium Grey"]->CreateSolidBrush( mainRGBs["Medium Grey"] );
			mainBrushes["Light Grey"] = new CBrush;
			mainBrushes["Light Grey"]->CreateSolidBrush( mainRGBs["Light Grey"] );
			mainBrushes["Green"] = new CBrush;
			mainBrushes["Green"]->CreateSolidBrush( mainRGBs["Green"] );
			mainBrushes["Red"] = new CBrush;
			mainBrushes["Red"]->CreateSolidBrush( mainRGBs["Red"] );
			mainBrushes["White"] = new CBrush;
			mainBrushes["White"]->CreateSolidBrush( mainRGBs["White"] );
			mainBrushes["Dull Red"] = new CBrush;
			mainBrushes["Dull Red"]->CreateSolidBrush( mainRGBs["Dull Red"] );
			mainBrushes["Dark Blue"] = new CBrush;
			mainBrushes["Dark Blue"]->CreateSolidBrush( mainRGBs["Dark Blue"] );
			mainBrushes["Dark Green"] = new CBrush;
			mainBrushes["Dark Green"]->CreateSolidBrush( mainRGBs["Dark Green"] );
			/// the following is all equivalent to:
			// mainFonts["Font name"] = new CFont;
			// mainFonts["Font name"].CreateFontA(...);
			(mainFonts["Smaller Font Max"] = new CFont)
				->CreateFontA(27, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
			(mainFonts["Normal Font Max"] = new CFont)
				->CreateFontA(34, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
			(mainFonts["Heading Font Max"] = new CFont)
				->CreateFontA(42, 0, 0, 0, FW_DONTCARE, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Old Sans Black"));
			(mainFonts["Code Font Max"] = new CFont)
				->CreateFontA(32, 0, 0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Consolas"));
			(mainFonts["Larger Font Max"] = new CFont)
				->CreateFontA(40, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
			(mainFonts["Smaller Font Large"] = new CFont)
				->CreateFontA(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
			(mainFonts["Normal Font Large"] = new CFont)
				->CreateFontA(20, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
			(mainFonts["Heading Font Large"] = new CFont)
				->CreateFontA(28, 0, 0, 0, FW_DONTCARE, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Old Sans Black"));
			(mainFonts["Code Font Large"] = new CFont)
				->CreateFontA(16, 0, 0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Consolas"));
			(mainFonts["Larger Font Large"] = new CFont)
				->CreateFontA(40, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
			(mainFonts["Smaller Font Med"] = new CFont)
				->CreateFontA(8, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
			(mainFonts["Normal Font Med"] = new CFont)
				->CreateFontA(12, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
			(mainFonts["Heading Font Med"] = new CFont)
				->CreateFontA(16, 0, 0, 0, FW_DONTCARE, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Old Sans Black"));
			(mainFonts["Code Font Med"] = new CFont)
				->CreateFontA(10, 0, 0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Consolas"));
			(mainFonts["Larger Font Med"] = new CFont)
				->CreateFontA(22, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
			(mainFonts["Smaller Font Small"] = new CFont)
				->CreateFontA(6, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
			(mainFonts["Normal Font Small"] = new CFont)
				->CreateFontA(8, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
			(mainFonts["Heading Font Small"] = new CFont)
				->CreateFontA(12, 0, 0, 0, FW_DONTCARE, TRUE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Old Sans Black"));
			(mainFonts["Code Font Small"] = new CFont)
				->CreateFontA(7, 0, 0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Consolas"));
			(mainFonts["Larger Font Small"] = new CFont)
				->CreateFontA(16, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							  CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Arial"));
		}

		BOOL OnInitDialog() override;
		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		BOOL PreTranslateMessage(MSG* pMsg); 
		void OnSize(UINT nType, int cx, int cy);
		void passCommonCommand(UINT id);
		void checkProfileReady();
		void checkProfileSave();
		void setOrientation(std::string orientation);
		void updateConfigurationSavedStatus(bool status);
		void clearVariables();
		void addVariable(std::string name, bool timelike, bool singleton, double value, int item);

		LRESULT onStatusTextMessage(WPARAM wParam, LPARAM lParam);
		LRESULT onErrorMessage(WPARAM wParam, LPARAM lParam);
		LRESULT onFatalErrorMessage(WPARAM wParam, LPARAM lParam);
		LRESULT onNormalFinishMessage(WPARAM wParam, LPARAM lParam);
		LRESULT onColoredEditMessage(WPARAM wParam, LPARAM lParam);
		LRESULT onDebugMessage(WPARAM wParam, LPARAM lParam);		

		void setNotes(std::string whichLevel, std::string notes);
		void setNiawgDefaults();

		std::vector<variable> getAllVariables();
		std::string getNotes(std::string whichLevel);
		brushMap getBrushes();
		rgbMap getRGB();
		fontMap getFonts();
		profileSettings getCurentProfileSettings();
		debugInfo getDebuggingOptions();
		mainOptions getMainOptions();
		void setDebuggingOptions(debugInfo options);
		void setMainOptions(mainOptions options);
		void updateStatusText(std::string whichStatus, std::string text);
		void addTimebar(std::string whichStatus);
		void setShortStatus(std::string text);
		void changeShortStatusColor(std::string color);
		void passDebugPress(UINT id);
		void passMainOptionsPress(UINT id);
		void listViewDblClick(NMHDR * pNotifyStruct, LRESULT * result);
		void handleRClick(NMHDR * pNotifyStruct, LRESULT * result);
		void handleExperimentCombo();
		void handleCategoryCombo();
		void handleConfigurationCombo();
		void handleSequenceCombo();
		void handleOrientationCombo();
		void OnClose();
		void OnCancel() override;
		void passClear(UINT id);
		void restartNiawgDefaults();
		void stopNiawg();
		void changeBoxColor(colorBoxes<char> colors);
		Communicator* getComm();
		CSocket* getSocket();

		bool niawgIsRunning();
		void setNiawgRunningState( bool newRunningState );

		void startExperiment( experimentThreadInput* input );

	private:
		DECLARE_MESSAGE_MAP();
		CBrush* test;
		ScriptingWindow* TheScriptingWindow;
		CameraWindow* TheCameraWindow;
		brushMap mainBrushes;
		rgbMap mainRGBs;
		fontMap mainFonts;
		ConfigurationFileSystem profile;
		NoteSystem notes;
		VariableSystem variables;
		DebuggingOptionsControl debugger;
		MainOptionsControl settings;
		StatusControl mainStatus;
		StatusControl debugStatus;
		StatusControl errorStatus;
		SMSTextingControl texter;
		StatusIndicator shortStatus;
		Communicator comm;
		NiawgController niawg;
		ColorBox boxes;
		CSocket masterSocket;
		ExperimentManager manager;
		std::vector<CToolTipCtrl*> tooltips;
		friend bool commonFunctions::handleCommonMessage(int msgID, CWnd* parent, MainWindow* mainWin, 
														ScriptingWindow* scriptWin, CameraWindow* camWin);
};

