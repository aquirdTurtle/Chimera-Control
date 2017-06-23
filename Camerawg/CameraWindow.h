#pragma once
#include "CameraSettingsControl.h"
#include "ColorBox.h"
#include "PictureStats.h"
#include "PictureManager.h"
#include "AlertSystem.h"
#include "DataAnalysisHandler.h"
#include "ExperimentTimer.h"
#include "DataLogger.h"


class MainWindow;
class ScriptingWindow;


struct cameraPositions
{
	POINT seriesPos;
	POINT amPos;
	POINT videoPos;
	POINT sPos;
};


class CameraWindow : public CDialog
{
	
	using CDialog::CDialog;
	
	DECLARE_DYNAMIC( CameraWindow )

	public:
		/// overrides
		CameraWindow::CameraWindow(MainWindow* mainWin, ScriptingWindow* scriptWin);
		HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
		BOOL OnInitDialog() override;
		BOOL PreTranslateMessage( MSG* pMsg );
		void OnCancel() override;
		void OnSize( UINT nType, int cx, int cy );
		void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* scrollbar );
		void OnTimer( UINT_PTR id );

		/// 
		void handlePictureEditChange(UINT id);
		void redrawPictures( bool andGrid );
		void changeBoxColor( colorBoxes<char> colors );
		std::vector<CToolTipCtrl*> getToolTips();
		bool getCameraStatus();
		void setTimerText( std::string timerText );
		void prepareCamera();
		void startCamera();
		std::string getStartMessage();
		void readImageParameters();
		void passCommonCommand( UINT id );
		void passTrigger();
		void passCameraMode();
		void passAlertPress();
		void passSetTemperaturePress();
		void passRepsPerVarPress();
		void passVariationNumberPress();
		void setEmGain();
		void handlePictureSettings( UINT id );
		bool cameraIsRunning();
		LRESULT onCameraFinish( WPARAM wParam, LPARAM lParam );
		LRESULT onCameraProgress( WPARAM wParam, LPARAM lParam );
		void listViewDblClick( NMHDR* info, LRESULT* lResult );
		void listViewLClick( NMHDR* info, LRESULT* lResult );
		
		void OnLButtonUp(UINT stuff, CPoint loc);
		void OnRButtonUp( UINT stuff, CPoint loc );
		void handleSpecialGreaterThanMaxSelection();
		void handleSpecialLessThanMinSelection();
		void abortCameraRun();
		friend bool commonFunctions::handleCommonMessage( int msgID, CWnd* parent, MainWindow* mainWin, ScriptingWindow* scriptWin, 
														  CameraWindow* camWin );
		void handleAutoscaleSelection();
		void assertOff();

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

		std::vector<CToolTipCtrl*> tooltips;

		std::pair<int, int> selectedPixel = { 0,0 };
		CMenu menu;
		// some picture menu options
		bool autoScalePictureData;
		bool specialLessThanMin;
		bool specialGreaterThanMax;
		bool realTimePic;

};
