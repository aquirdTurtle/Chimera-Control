#pragma once
#include "CameraSettingsControl.h"
#include "ColorBox.h"
#include "PictureStats.h"
#include "PictureManager.h"
#include "AlertSystem.h"
#include "PlottingControl.h"

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
		CameraWindow::CameraWindow() : CDialog(), CameraSettings(&Andor) {};
		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		BOOL OnInitDialog() override;
		void OnCancel() override;
		void OnSize(UINT nType, int cx, int cy);
		void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* scrollbar);

		void getFriends(MainWindow* mainWin, ScriptingWindow* scriptWin);
		void redrawPictures();
		void changeBoxColor(std::string color);

		void readImageParameters();
		void passCommonCommand(UINT id);
		void setEmGain();
		

	private:
		DECLARE_MESSAGE_MAP();
		AndorCamera Andor;
		CameraSettingsControl CameraSettings;
		ColorBox box;
		PictureStats stats;
		PictureManager pics;
		AlertSystem alerts;
		PlottingControl plotters;

		MainWindow* mainWindowFriend;
		ScriptingWindow* scriptingWindowFriend;

};
