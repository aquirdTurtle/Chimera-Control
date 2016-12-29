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
		/// Extra functions from this class.
		void redrawPictures();
		void changeBoxColor( colorBoxes<char> colors );
		std::vector<CToolTipCtrl*> getToolTips();
		bool getCameraStatus();
		void setTimerColor( std::string color );
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
		void onCameraFinish();

		void listViewDblClick(NMHDR* info, LRESULT* lResult);
		void listViewRClick(NMHDR* info, LRESULT* lResult);

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
};
