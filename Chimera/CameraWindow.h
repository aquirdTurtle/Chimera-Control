#pragma once
#include "CameraSettingsControl.h"
#include "ColorBox.h"
#include "PictureStats.h"
#include "PictureManager.h"
#include "AlertSystem.h"
#include "DataAnalysisHandler.h"
#include "ExperimentTimer.h"
#include "DataLogger.h"
#include "gnuplotter.h"
#include "commonFunctions.h"

class MainWindow;
class ScriptingWindow;
class DeviceWindow;

struct cameraPositions
{
	POINT seriesPos;
	POINT amPos;
	POINT videoPos;
	POINT sPos;
};


struct plotThreadInput
{
	std::vector<tinyPlotInfo> plotInfo;
	std::vector<std::pair<int, int>> analysisLocations;
	std::atomic<bool>* active;
	std::vector<std::vector<long>>* imageQueue;
	std::vector<std::vector<bool>>* atomQueue;
	imageParameters imageShape;
	UINT picsPerVariation;
	UINT picsPerRep;
	UINT variations;

	UINT alertThreshold;
	bool wantAlerts;
	Communicator* comm;
	std::mutex* plotLock;

	UINT plottingFrequency;
	UINT numberOfRunsToAverage;
	std::vector<double>* key;
	Gnuplotter* plotter;

	bool needsCounts;
};


struct atomCruncherInput
{
	// what the thread watches...
	std::atomic<bool>* cruncherThreadActive;
	std::vector<std::vector<long>>* imageQueue;
	// options
	bool plotterActive;
	bool plotterNeedsImages;
	bool rearrangerActive;
	UINT picsPerRep;
	// locks
	std::mutex* imageLock;
	std::mutex* plotLock;
	std::mutex* rearrangerLock;
	// what the thread fills.
	std::vector<std::vector<long>>* plotterImageQueue;
	std::vector<std::vector<bool>>* plotterAtomQueue;
	std::vector<std::vector<bool>>* rearrangerAtomQueue;
	std::array<int, 4> thresholds;
};


class CameraWindow : public CDialog
{
	using CDialog::CDialog;
	
	DECLARE_DYNAMIC( CameraWindow )

	public:
		/// overrides
		CameraWindow();
		HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
		BOOL OnInitDialog() override;
		BOOL PreTranslateMessage( MSG* pMsg );
		void OnCancel() override;
		void OnSize( UINT nType, int cx, int cy );
		void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* scrollbar );
		void OnTimer( UINT_PTR id );
		void OnLButtonUp( UINT stuff, CPoint loc );
		void OnRButtonUp( UINT stuff, CPoint loc );
		/// directly called by the message map or 1 simple step removed.
		LRESULT onCameraFinish( WPARAM wParam, LPARAM lParam );
		LRESULT onCameraProgress( WPARAM wParam, LPARAM lParam );
		void handleDblClick( NMHDR* info, LRESULT* lResult );
		void listViewRClick( NMHDR* info, LRESULT* lResult );
		void handleSpecialGreaterThanMaxSelection();
		void handleSpecialLessThanMinSelection();
		void readImageParameters();
		void passCommonCommand( UINT id );
		void passTrigger();
		void passCameraMode();
		void passAlertPress();
		void passSetTemperaturePress();
		void passRepsPerVarPress();
		void passVariationNumberPress();
		void passAlwaysShowGrid();
		void passSetAnalysisLocations();
		void catchEnter();


		/// auxiliary functions.
		std::string getSystemStatusString();
		void getFriends(MainWindow* mainWin, ScriptingWindow* scriptWin, DeviceWindow* masterWin);
		void handleSaveConfig(std::ofstream& saveFile);
		void handleMasterConfigSave(std::stringstream& configStream);
		void handleMasterConfigOpen(std::stringstream& configStream, double version);
		void handlePictureEditChange(UINT id);
		void handleOpeningConfig(std::ifstream& configFile, double version);
		void redrawPictures( bool andGrid );
		void changeBoxColor( systemInfo<char> colors );
		cToolTips getToolTips();
		bool getCameraStatus();
		void setTimerText( std::string timerText );
		void prepareCamera();
		void startCamera();
		std::string getStartMessage();
		void setEmGain();
		void handlePictureSettings( UINT id );
		bool cameraIsRunning();
		void abortCameraRun();
		void handleAutoscaleSelection();
		void assertOff();

		static UINT __stdcall atomCruncherProcedure(void* input);

		friend void commonFunctions::handleCommonMessage( int msgID, CWnd* parent, MainWindow* mainWin, ScriptingWindow* scriptWin,
														  CameraWindow* camWin, DeviceWindow* masterWin );

	private:
		DECLARE_MESSAGE_MAP();

		AndorCamera Andor;
		CameraSettingsControl CameraSettings;
		ColorBox box;
		PictureStats stats;
		PictureManager pics;
		AlertSystem alerts;
		ExperimentTimer timer;		
		// these two could probably be combined in a sensible way.
		DataAnalysisControl analysisHandler;
		DataLogger dataHandler;

		MainWindow* mainWindowFriend;
		ScriptingWindow* scriptingWindowFriend;
		DeviceWindow* deviceWindowFriend;

		cToolTips tooltips;

		std::pair<int, int> selectedPixel = { 0,0 };
		CMenu menu;
		// some picture menu options
		bool autoScalePictureData;
		bool alwaysShowGrid;
		bool specialLessThanMin;
		bool specialGreaterThanMax;
		bool realTimePic;

		// plotting stuff;
		HANDLE plotThreadHandle;
		std::vector<std::vector<long> > imageQueue;
		std::mutex imageLock;
		// the following two queues and locks aren't directly used by the camera window, but the camera window
		// distributes them to the threads that do use them.
		std::vector<std::vector<bool>> plotterAtomQueue;
		// only used sometimes.
		std::vector<std::vector<long>> plotterPictureQueue;
		std::vector<std::vector<bool>> rearrangerAtomQueue;
		std::mutex plotLock;
		std::mutex rearrangerLock;
		HANDLE atomCruncherThreadHandle;
		std::atomic<bool> atomCrunchThreadActive;
		// 
		std::atomic<bool> plotThreadActive;
		std::vector<double> plotterKey;
		Gnuplotter plotter;
};

