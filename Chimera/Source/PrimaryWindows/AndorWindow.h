// created by Mark O. Brown
#pragma once

#include "Andor/CameraSettingsControl.h"
#include "ExperimentMonitoringAndStatus/ColorBox.h"
#include "GeneralImaging/PictureStats.h"
#include "GeneralImaging/PictureManager.h"
#include "ExperimentMonitoringAndStatus/AlertSystem.h"
#include "RealTimeDataAnalysis/DataAnalysisControl.h"
#include "ExperimentMonitoringAndStatus/ExperimentTimer.h"
#include "DataLogging/DataLogger.h"
#include "GeneralUtilityFunctions/commonFunctions.h"
#include "RealTimeDataAnalysis/atomCruncherInput.h"
#include "Andor/cameraPositions.h"
#include "GeneralObjects/commonTypes.h"
#include "GeneralObjects/Queues.h"
#include "Piezo/PiezoController.h"
#include <bitset>


class MainWindow;
class ScriptingWindow;
class AuxiliaryWindow;
class BaslerWindow;
class DeformableMirrorWindow;


class AndorWindow : public CDialog
{
	using CDialog::CDialog;
	
	DECLARE_DYNAMIC( AndorWindow )

	public:
		/// overrides
 		AndorWindow();
		void OnPaint ();
 		HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
		BOOL OnInitDialog() override;
		void OnMouseMove( UINT thing, CPoint point );
		BOOL PreTranslateMessage( MSG* pMsg );
		void OnCancel() override;
		void OnSize( UINT nType, int cx, int cy );
		void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* scrollbar );
		void OnTimer( UINT_PTR id );
		void OnLButtonUp( UINT stuff, CPoint loc );
		void OnRButtonUp( UINT stuff, CPoint loc );
		/// directly called by the message map or 1 simple step removed.
		void wakeRearranger( );
		LRESULT onCameraFinish( WPARAM wParam, LPARAM lParam );
		LRESULT onCameraCalFinish( WPARAM wParam, LPARAM lParam );
		LRESULT onCameraProgress( WPARAM wParam, LPARAM lParam );
		LRESULT onCameraCalProgress( WPARAM wParam, LPARAM lParam );
		LRESULT onBaslerFinish( WPARAM wParam, LPARAM lParam );
		LRESULT handlePrepareForAcq (WPARAM wparam, LPARAM lparam);
		void handleDblClick( NMHDR* info, LRESULT* lResult );
		void listViewRClick( NMHDR* info, LRESULT* lResult );
		void handleSpecialGreaterThanMaxSelection();
		void handleSpecialLessThanMinSelection();
		void readImageParameters();
		void passCommonCommand( UINT id );
		void passTrigger();
		void passCameraMode();
		void passSetTemperaturePress();
		void passAlwaysShowGrid();
		void passManualSetAnalysisLocations();
		void passSetGridCorner( );
		void catchEnter();
		void setDataType( std::string dataType );
		/// auxiliary functions.
		void calibrate ( );
		dataPoint getMainAnalysisResult ( );
		void checkCameraIdle( );
		void handleEmGainChange();
		void fillMasterThreadInput( ExperimentThreadInput* input );
		DataLogger& getLogger();
		std::string getSystemStatusString();
		void loadFriends(MainWindow* mainWin, ScriptingWindow* scriptWin, AuxiliaryWindow* auxWin, 
						  BaslerWindow* basWin, DeformableMirrorWindow* dmWindow);
		void handleNewConfig( std::ofstream& newFile );
		void handleSaveConfig(std::ofstream& saveFile);
		void handleMasterConfigSave(std::stringstream& configStream);
		void handleMasterConfigOpen(std::stringstream& configStream, Version version);
		void handlePictureEditChange(UINT id);
		void handleOpeningConfig(std::ifstream& configFile, Version ver );
		void redrawPictures( bool andGrid );
		void changeBoxColor( systemInfo<char> colors );
		cToolTips getToolTips();
		bool getCameraStatus();
		void setTimerText( std::string timerText );
		void armCameraWindow(AndorRunSettings* settings);
		void handlePiezoProg ();
		std::string getStartMessage();
		void handlePictureSettings( UINT id );
		bool cameraIsRunning();
		void abortCameraRun();
		void handleAutoscaleSelection();
		void assertOff();
		void passPictureSettings( UINT id );
		void prepareAtomCruncher( AllExperimentInput& input );
		void preparePlotter( AllExperimentInput& input );
		static UINT __stdcall atomCruncherProcedure(void* input);
		void writeVolts( UINT currentVoltNumber, std::vector<float64> data );
		friend void commonFunctions::handleCommonMessage( int msgID, CWnd* parent, MainWindow* mainWin, 
														  ScriptingWindow* scriptWin, AndorWindow* camWin, 
														  AuxiliaryWindow* masterWin, BaslerWindow* basWin, DeformableMirrorWindow* dmWin );
		void passAtomGridCombo( );
		void passDelGrid( );
		void startAtomCruncher(AllExperimentInput& input);
		void startPlotterThread( AllExperimentInput& input );
		bool wantsAutoPause( );
		std::atomic<bool>* getSkipNextAtomic();
		void handlePlotTimerEdit ( );
		void stopPlotter( );
		void stopSound( );
		void handleImageDimsEdit(UINT id );
		void loadCameraCalSettings( AllExperimentInput& input );
		bool wasJustCalibrated( );
		bool wantsAutoCal( );
		bool wantsNoMotAlert ( );
		UINT getNoMotThreshold ( );
		void setMenuCheck ( UINT menuItem, UINT itemState );
		atomGrid getMainAtomGrid( );
		std::string getMostRecentDateString ( );
		int getMostRecentFid ( );
		int getPicsPerRep ( );
		bool wantsThresholdAnalysis ( );
		AndorCameraCore& getCamera ( );
		std::atomic<bool>& getPlotThreadActiveRef ( );
		std::atomic<HANDLE>& getPlotThreadHandleRef ( );
		std::mutex& getActivePlotMutexRef ( );
		void cleanUpAfterExp ( );
		void handlePlotPop (UINT id);
		int getDataCalNum ();
	private:
		bool justCalibrated = false;
		DECLARE_MESSAGE_MAP();
		UINT numExcessCounts = 0;
		AndorCameraCore andor;
		AndorCameraSettingsControl andorSettingsCtrl;
		PictureManager pics;
		int plotIds = 17009;
		ColorBox box;
		PictureStats stats;
		AlertSystem alerts;
		ExperimentTimer timer; 
		PiezoController cameraPiezo;
		
		DataAnalysisControl analysisHandler;
		DataLogger dataHandler;
		std::vector<PlotCtrl*> mainAnalysisPlots;
		MainWindow* mainWin;
		ScriptingWindow* scriptWin;
		AuxiliaryWindow* auxWin;
		BaslerWindow* basWin;
		DeformableMirrorWindow* dmWin;

		cToolTips tooltips;
		coordinate selectedPixel = { 0,0 };
		CMenu menu;
		// some picture menu options
		bool autoScalePictureData;
		bool alwaysShowGrid;
		bool specialLessThanMin;
		bool specialGreaterThanMax;
		bool realTimePic;
		// plotting stuff;
		std::atomic<HANDLE> plotThreadHandle;
		imageQueue imQueue;
		std::mutex imageQueueLock;
		std::condition_variable rearrangerConditionVariable;
		// the following two queues and locks aren't directly used by the camera window, but the camera window
		// distributes them to the threads that do use them.

		multiGridAtomQueue plotterAtomQueue;
		multiGridImageQueue plotterPictureQueue;
		atomQueue rearrangerAtomQueue;
		std::mutex plotLock;
		std::mutex rearrangerLock;

		dataPoint mostRecentAnalysisResult;
		// 
		HANDLE atomCruncherThreadHandle;
		std::atomic<bool> atomCrunchThreadActive;
		// 
		std::atomic<bool> plotThreadActive;
		std::atomic<bool> plotThreadAborting = false;
		std::atomic<bool> skipNext=false;
		std::vector<double> plotterKey;
		chronoTimes imageTimes, imageGrabTimes, mainThreadStartTimes, crunchSeesTimes, crunchFinTimes;		
		std::mutex activePlotMutex;
		std::vector<PlotDialog*> activeDlgPlots;
		UINT mostRecentPicNum = 0;
		UINT currentPictureNum = 0;
		Matrix<long> avgBackground;
};



