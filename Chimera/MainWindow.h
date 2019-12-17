// created by Mark O. Brown
#pragma once
#include "NiawgController.h"
#include "ProfileSystem.h"
#include "DebugOptionsControl.h"
#include "MainOptionsControl.h"
#include "StatusControl.h"
#include "StatusIndicator.h"
#include "Communicator.h"
#include "SmsTextingControl.h"
#include "EmbeddedPythonHandler.h"
#include "MasterConfiguration.h"
#include "Repetitions.h"
#include "MasterManager.h"
#include "DataLogger.h"
#include "NoteSystem.h"
#include "profileSettings.h"
#include "rerngGuiControl.h"
#include "PlotDialog.h"
#include "OscilloscopeViewer.h"
#include "profileSettings.h"
#include "commonFunctions.h"
#include "DmCore.h"

#include <string>
#include <vector>
#include "afxwin.h"


class ScriptingWindow;
class AndorWindow;
class AuxiliaryWindow;
class BaslerWindow;
class AuxiliaryWindow2;

struct scopeRefreshInput
{
	ScopeViewer *masterRepumpScope, *motScope;
};


class MainWindow : public CDialog
{
	using CDialog::CDialog;
	DECLARE_DYNAMIC(MainWindow);
	public:
		void showHardwareStatus ( );
		BOOL handleAccelerators( HACCEL m_haccel, LPMSG lpMsg );
	    // overrides
		void OnRButtonUp( UINT stuff, CPoint clickLocation );
		void OnLButtonUp( UINT stuff, CPoint clickLocation );
		void OnPaint( );
		MainWindow(UINT id, CDialog* spash, chronoTime* initTime);
		BOOL OnInitDialog() override;
		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		BOOL PreTranslateMessage(MSG* pMsg); 
		void OnSize(UINT nType, int cx, int cy);
		void OnClose();
		void catchEnter( );
		void forceExit ( );
		void OnCancel() override;
		void setMenuCheck ( UINT menuItem, UINT itemState );

		// stuff directly called (or 1 simple step removed) by message map.
		LRESULT onRepProgress(WPARAM wParam, LPARAM lParam);
		LRESULT onStatusTextMessage(WPARAM wParam, LPARAM lParam);
		LRESULT onErrorMessage(WPARAM wParam, LPARAM lParam);
		LRESULT onFatalErrorMessage(WPARAM wParam, LPARAM lParam);

		LRESULT onDebugMessage(WPARAM wParam, LPARAM lParam);		
		LRESULT onNoAtomsAlertMessage( WPARAM wp, LPARAM lp);
		LRESULT onNoMotAlertMessage ( WPARAM wp, LPARAM lp );
		LRESULT onFinish ( WPARAM wp, LPARAM lp );

		void onNormalFinishMessage ( );
		void onMotNumCalFin ( );
		void onMotTempCalFin ( );
		void onPgcTempCalFin ( );
		void onGreyTempCalFin ( );
		void onMachineOptRoundFin ( );
		void handleThresholdAnalysis ( );
		//
		static unsigned int __stdcall scopeRefreshProcedure( void* voidInput );
		void loadCameraCalSettings( ExperimentThreadInput* input );
		void stopRearranger( );
		void waitForRearranger( );
		void passCommonCommand( UINT id );
		void handlePause();
		void passDebugPress( UINT id );
		void passMainOptionsPress( UINT id );
		void handleDblClick( NMHDR * pNotifyStruct, LRESULT * result );
		void handleRClick( NMHDR * pNotifyStruct, LRESULT * result );
		void handleSequenceCombo();
		void passClear( UINT id );
		void passNiawgIsOnPress( );
		CFont* getPlotFont( );
		// auxiliary functions used by the window.
		void setNotes(std::string notes);
		void setNiawgDefaults();
		void fillMasterThreadInput( ExperimentThreadInput* input );
		void fillMasterThreadSequence( ExperimentThreadInput* input );
		void fillMotInput( ExperimentThreadInput* input);
		void fillTempInput ( ExperimentThreadInput* input );
		void fillMotTempProfile ( ExperimentThreadInput* input );
		void fillRedPgcTempProfile ( ExperimentThreadInput* input );
		void fillGreyPgcTempProfile ( ExperimentThreadInput* input );

		HANDLE startExperimentThread( ExperimentThreadInput* input );
		
		std::string getNotes( );
		fontMap getFonts();
		profileSettings getProfileSettings();
		seqSettings getSeqSettings( );
		debugInfo getDebuggingOptions();
		mainOptions getMainOptions();

		void checkProfileReady();
		void checkProfileSave();
		void updateConfigurationSavedStatus(bool status);

		void setDebuggingOptions(debugInfo options);
		void updateStatusText(std::string whichStatus, std::string text);
		void addTimebar(std::string whichStatus);
		void setShortStatus(std::string text);
		void changeShortStatusColor(std::string color);
		void restartNiawgDefaults();
		void stopNiawg();
		void changeBoxColor(systemInfo<char> colors);
		void handleNewConfig( std::ofstream& newFile );
		void handleSaveConfig(std::ofstream& saveFile);
		void handleOpeningConfig(std::ifstream& configFile, Version ver );
		void abortMasterThread();
		Communicator* getComm();
		std::string getSystemStatusString();
		void fillMotSizeInput ( ExperimentThreadInput* input );
		bool niawgIsRunning();
		bool masterIsRunning();
		void setNiawgRunningState( bool newRunningState );
		RunInfo getRunInfo();
		void handleFinish();
		UINT getRepNumber();
		void logParams( DataLogger* logger, ExperimentThreadInput* input );
		bool experimentIsPaused( );
		void notifyConfigUpdate( );
		void passConfigPress( );
		void passExperimentRerngButton( );
		void OnTimer( UINT_PTR id );
		std::vector<Gdiplus::Pen*> getPlotPens( );
		std::vector<Gdiplus::Pen*> getBrightPlotPens( );
		std::vector<Gdiplus::SolidBrush*> getPlotBrushes( );
		std::vector<Gdiplus::SolidBrush*> getBrightPlotBrushes( );
		void passRerngModeComboChange ( );
		void checkAllMenus ( UINT menuItem, UINT itemState );
		const static UINT 
			StatusUpdateMessageID		= ( WM_APP + 1 ),
			ErrorUpdateMessageID		= ( WM_APP + 2 ),
			FatalErrorMessageID			= ( WM_APP + 3 ),
			NormalFinishMessageID		= ( WM_APP + 4 ),
			DebugUpdateMessageID		= ( WM_APP + 6 ),
			AndorFinishMessageID		= ( WM_APP + 7 ),
			AndorProgressMessageID		= ( WM_APP + 8 ),
			RepProgressMessageID		= ( WM_APP + 9 ),
			NoAtomsAlertMessageID		= ( WM_APP + 10 ),
			LogVoltsMessageID			= ( WM_APP + 11 ),
			AutoServoMessage			= ( WM_APP + 12 ),
			AndorCalProgMessageID		= ( WM_APP + 13 ),
			AndorCalFinMessageID		= ( WM_APP + 14 ),
			BaslerProgressMessageID		= ( WM_APP + 15 ),
			BaslerFinMessageID			= ( WM_APP + 18 ),
			GeneralFinMsgID				= ( WM_APP + 19 ),
			NoMotAlertMessageID			= ( WM_APP + 20 );
		bool autoF5_AfterFinish = false;
		EmbeddedPythonHandler& getPython ( );
		NiawgController& getNiawg ( );
		Communicator& getCommRef ( );
		
	private:
		DECLARE_MESSAGE_MAP();

		chronoTimes startupTimes;
		chronoTime* programStartTime;

		ScriptingWindow* TheScriptingWindow;
		AndorWindow* TheAndorWindow;
		AuxiliaryWindow* TheAuxiliaryWindow;
		BaslerWindow* TheBaslerWindow;
		AuxiliaryWindow2* TheAuxiliaryWindow2;
		// members that have gui elements
		ProfileSystem profile;
		MasterConfiguration masterConfig;
		NoteSystem notes;
		DebugOptionsControl debugger;
		Repetitions repetitionControl;
		MainOptionsControl mainOptsCtrl;
		StatusControl mainStatus;
		StatusControl debugStatus;
		StatusControl errorStatus;
		SmsTextingControl texter;
		StatusIndicator shortStatus;
		rerngGuiControl rearrangeControl;
		NiawgController niawg;
		ColorBox boxes;
		// auxiliary members
		Communicator comm;
		fontMap mainFonts;
		MasterThreadManager masterThreadManager;
		CMenu menu;
		RunInfo systemRunningInfo;
		cToolTips tooltips;
		EmbeddedPythonHandler python;
		ScopeViewer masterRepumpScope, motScope;
		CFont* plotfont;
		DmCore defMirror;
		//
		static BOOL CALLBACK monitorHandlingProc( _In_ HMONITOR hMonitor, _In_ HDC      hdcMonitor,
												  _In_ LPRECT lprcMonitor, _In_ LPARAM dwData );
		std::vector<Gdiplus::Pen*> plotPens, brightPlotPens;
		std::vector<Gdiplus::SolidBrush*> plotBrushes, brightPlotBrushes;
		// friends (try to minimize these)
		friend void commonFunctions::handleCommonMessage( int msgID, CWnd* parent, MainWindow* mainWin,
														  ScriptingWindow* scriptWin, AndorWindow* camWin,
														  AuxiliaryWindow* masterWin, BaslerWindow* basWin, AuxiliaryWindow2 *auxWin2 );
		CDialog* appSplash;
};

