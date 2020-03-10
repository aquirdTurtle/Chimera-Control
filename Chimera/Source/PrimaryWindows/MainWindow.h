// created by Mark O. Brown
#pragma once
#include "ConfigurationSystems/ProfileSystem.h"
#include "MiscellaneousExperimentOptions/DebugOptionsControl.h"
#include "MiscellaneousExperimentOptions/MainOptionsControl.h"
#include "ExperimentMonitoringAndStatus/StatusControl.h"
#include "ExperimentMonitoringAndStatus/StatusIndicator.h"
#include "Python/SmsTextingControl.h"
#include "Python/EmbeddedPythonHandler.h"
#include "ConfigurationSystems/MasterConfiguration.h"
#include "MiscellaneousExperimentOptions/Repetitions.h"
#include "DataLogging/DataLogger.h"
#include "ConfigurationSystems/NoteSystem.h"
#include "ConfigurationSystems/profileSettings.h"
#include "Plotting/PlotDialog.h"
#include "Plotting/OscilloscopeViewer.h"
#include "GeneralUtilityFunctions/commonFunctions.h"
#include "CustomMessages.h"
#include "ExperimentThread/ExperimentThreadManager.h"

#include <string>
#include <vector>
#include "afxwin.h"

class ScriptingWindow;
class AndorWindow;
class AuxiliaryWindow;
class BaslerWindow;
class DeformableMirrorWindow;

struct scopeRefreshInput 
{
	ScopeViewer *masterRepumpScope, *motScope;
};

class MainWindow : public CDialog
{
	using CDialog::CDialog;
	DECLARE_DYNAMIC(MainWindow);
	public:
		void handlePlotPop (UINT id);
		void showHardwareStatus ( );
		void drawServoListview (NMHDR* pNMHDR, LRESULT* pResult);
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
		void onGreyTempCalFin ( );
		void onMachineOptRoundFin ( );
		void handleThresholdAnalysis ( );
		void onAutoCalFin ();
		//
		static unsigned int __stdcall scopeRefreshProcedure( void* voidInput );
		void loadCameraCalSettings( ExperimentThreadInput* input );
		void passCommonCommand( UINT id );
		void handlePause();
		void passDebugPress( UINT id );
		void passMainOptionsPress( UINT id );
		void handleDblClick( NMHDR * pNotifyStruct, LRESULT * result );
		void handleRClick( NMHDR * pNotifyStruct, LRESULT * result );
		void handleSequenceCombo();
		void passClear( UINT id );
		CFont* getPlotFont( );
		// auxiliary functions used by the window.
		void setNotes(std::string notes);
		void fillMasterThreadInput( ExperimentThreadInput* input );
		void fillMasterThreadSequence( ExperimentThreadInput* input );
		void fillMotInput( ExperimentThreadInput* input);

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
		void changeBoxColor(systemInfo<char> colors);
		void handleSaveConfig(std::ofstream& saveFile);
		void handleOpeningConfig(std::ifstream& configFile, Version ver );
		void abortMasterThread();
		Communicator* getComm();
		std::string getSystemStatusString();
		bool masterIsRunning();
		RunInfo getRunInfo();
		void handleFinish();
		UINT getRepNumber();
		void logParams( DataLogger* logger, ExperimentThreadInput* input );
		bool experimentIsPaused( );
		void notifyConfigUpdate( );
		void passConfigPress( );
		
		void OnTimer( UINT_PTR id );
		std::vector<Gdiplus::Pen*> getPlotPens( );
		std::vector<Gdiplus::Pen*> getBrightPlotPens( );
		std::vector<Gdiplus::SolidBrush*> getPlotBrushes( );
		std::vector<Gdiplus::SolidBrush*> getBrightPlotBrushes( );
		
		void checkAllMenus ( UINT menuItem, UINT itemState );
		void ServoRClick (NMHDR* pNotifyStruct, LRESULT* result);
		void ServoDblClick (NMHDR* pNotifyStruct, LRESULT* result);
		LRESULT autoServo (WPARAM w, LPARAM l);
		void runServos ();
		std::vector<servoInfo> getServoinfo ();
		void handleMasterConfigSave (std::stringstream& configStream);
		void handleMasterConfigOpen (std::stringstream& configStream, Version version);
		void handleServoUnitsComboChange ();
		bool autoF5_AfterFinish = false;
		EmbeddedPythonHandler& getPython ( );
		Communicator& getCommRef ( );
		UINT getAutoCalNumber ();

	private:
		DECLARE_MESSAGE_MAP();

		chronoTimes startupTimes;
		chronoTime* programStartTime;

		ScriptingWindow* TheScriptingWindow = NULL;
		AndorWindow* TheAndorWindow = NULL;
		AuxiliaryWindow* TheAuxiliaryWindow = NULL;
		BaslerWindow* TheBaslerWindow = NULL;
		DeformableMirrorWindow* TheDmWindow = NULL;
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
		ColorBox boxes;
		ServoManager servos;
		// auxiliary members
		Communicator comm;
		fontMap mainFonts;
		ExperimentThreadManager expThreadManager;
		CMenu menu;
		RunInfo systemRunningInfo;
		cToolTips tooltips;
		EmbeddedPythonHandler python;
		ScopeViewer masterRepumpScope, motScope;
		CFont* plotfont;
		//
		static BOOL CALLBACK monitorHandlingProc( _In_ HMONITOR hMonitor, _In_ HDC      hdcMonitor,
												  _In_ LPRECT lprcMonitor, _In_ LPARAM dwData );
		std::vector<Gdiplus::Pen*> plotPens, brightPlotPens;
		std::vector<Gdiplus::SolidBrush*> plotBrushes, brightPlotBrushes;
		CDialog* appSplash;
 		// friends (try to minimize these)
 		friend void commonFunctions::handleCommonMessage( int msgID, CWnd* parent, MainWindow* mainWin,
 														  ScriptingWindow* scriptWin, AndorWindow* camWin,
														  AuxiliaryWindow* masterWin, BaslerWindow* basWin, 
														  DeformableMirrorWindow* auxWin2);
		UINT autoCalNum = 0;
};


