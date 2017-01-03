#pragma once
#include "CameraSettingsControl.h"
#include "ColorBox.h"
#include "PictureStats.h"
#include "PictureManager.h"
#include "AlertSystem.h"
#include "DataAnalysisHandler.h"
#include "ExperimentTimer.h"

class MainWindow;
class ScriptingWindow;

// pass a structure like this to initializers.
struct cameraPositions
{
	POINT ksmPos;
	POINT amPos;
	POINT cssmPos;
};

class CameraWindow : public CDialog
{
	using CDialog::CDialog;
	DECLARE_DYNAMIC(CameraWindow)
	public:
		/// overrides
		CameraWindow::CameraWindow(MainWindow* mainWin, ScriptingWindow* scriptWin) : 
			CDialog(), 
			CameraSettings(&Andor), 
			Andor(mainWin->getComm())
		{
			// because of these lines the camera window does not need to "get friends".
			mainWindowFriend = mainWin;
			scriptingWindowFriend = scriptWin;
		};
		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		BOOL OnInitDialog() override;
		BOOL PreTranslateMessage(MSG* pMsg);
		void OnCancel() override;
		void OnSize(UINT nType, int cx, int cy);
		void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* scrollbar);
		void OnTimer(UINT_PTR id);
		void handlePictureEditChange( UINT id );
		/// Extra functions from this class.
		void redrawPictures();
		void changeBoxColor( colorBoxes<char> colors );
		std::vector<CToolTipCtrl*> getToolTips();
		bool getCameraStatus();
		void setTimerText( std::string timerText );
		void prepareCamera();
		void startCamera();
		std::string getStartMessage();
		/// passing commands on to members and their handling.
		void readImageParameters();
		void passCommonCommand(UINT id);
		void passTrigger();
		void passAlertPress();
		void passSetTemperaturePress();
		void setEmGain();
		void handlePictureSettings(UINT id);
		bool cameraIsRunning();
		LRESULT onCameraFinish( WPARAM wParam, LPARAM lParam );
		LRESULT onCameraProgress( WPARAM wParam, LPARAM lParam );

		void listViewDblClick(NMHDR* info, LRESULT* lResult);
		void listViewLClick( NMHDR* info, LRESULT* lResult );
		
		void OnRButtonUp(UINT stuff, CPoint loc);

		void abortCameraRun();

	private:
		DECLARE_MESSAGE_MAP();
		
		AndorCamera Andor;
		CameraSettingsControl CameraSettings; 
		ColorBox box;
		PictureStats stats;
		PictureManager pics;
		AlertSystem alerts;
		DataHandlingControl dataHandler;
		ExperimentTimer timer;

		MainWindow* mainWindowFriend;
		ScriptingWindow* scriptingWindowFriend;

		std::vector<CToolTipCtrl*> tooltips;
		
		POINT selectedPixel = { 0,0 };

		bool autoScalePictureData;
		bool realTimePic;
};
