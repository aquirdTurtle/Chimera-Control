// created by Mark O. Brown
#pragma once
#include <atomic>
#include "GeneralImaging/PictureManager.h"
#include "GeneralImaging/PictureControl.h"
#include "Control.h"
#include "Basler/BaslerSettingsControl.h"
#include "Basler/BaslerCamera.h"
#include "GeneralImaging/PictureStats.h"
#include "GeneralObjects/commonTypes.h"
#include "Plotting/PlotCtrl.h"

class MainWindow;
class ScriptingWindow;
class AndorWindow;
class AuxiliaryWindow;
class BaslerWindow;


class BaslerWindow : public CDialogEx
{
 	public:
		BaslerWindow( );
		BOOL OnInitDialog();
		void handleEnter();
		void handleBaslerAutoscaleSelection ( );
		void OnPaint();
		void passCommonCommand ( UINT id );
		void startTemporaryAcquisition ( baslerSettings motSizeSettings );
		void startDefaultAcquisition ( );
		void setCameraForMotTempMeasurement ( );
		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		HCURSOR OnQueryDragIcon();
		void OnSize( UINT nType, int cx, int cy );
		void ArmCamera();
		void handleClose();
		void handleDisarmPress();
		void initializeControls();
		void passExposureMode();
		void passCameraMode();
		bool baslerCameraIsRunning ( );
		bool baslerCameraIsContinuous ( );
		void fillMotSizeInput ( baslerSettings& settings);
		LRESULT handleNewPics( WPARAM wParam, LPARAM lParam );
		void fillTemperatureMeasurementInput ( baslerSettings& settings );
		void pictureRangeEditChange( UINT id );
		void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* scrollbar );
		void handleSoftwareTrigger();
		void handleOpeningConfig ( std::ifstream& configFile, Version ver );
		void OnMouseMove(UINT flags, CPoint point );
		void OnRButtonUp( UINT stuff, CPoint clickLocation );
		void DoDataExchange( CDataExchange* pDX );
		void loadFriends ( MainWindow* mainWin_, ScriptingWindow* scriptWin_, AndorWindow* camWin_, AuxiliaryWindow* auxWin_ );
		void handleSavingConfig ( std::ofstream& configFile );
		void startCamera ( );
		baslerSettings getCurrentSettings ( );
		void setMenuCheck ( UINT menuItem, UINT itemState );
		BaslerCameraCore& getCore();
	private:
		// for the basler window, this is typically only one picture, but I include this here anyways.
		UINT loadMotConsecutiveFailures = 0;
		bool motLoaded=false;
		bool autoScaleBaslerPictureData = true;
		PictureManager picManager;
		BaslerSettingsControl settingsCtrl;
		BaslerCameraCore* basCamCore;
		PictureStats stats;
		baslerSettings tempAcqSettings;
		bool runningAutoAcq;
		//PictureSaver saver;
		unsigned int currentRepNumber;
		BaslerAutoExposure::mode runExposureMode;
		std::vector<std::vector<long>> images;
		unsigned int imageWidth;
		CMenu menu;
		bool isRunning;
		std::atomic<bool> triggerThreadFlag;
		PlotCtrl *horGraph, *vertGraph;
		std::vector<Gdiplus::Pen*> plotPens, brightPlotPens;
		std::vector<Gdiplus::SolidBrush*> plotBrushes, brightPlotBrushes;
		CFont* plotfont;
		MainWindow* mainWin;
		AndorWindow* camWin;
		AuxiliaryWindow* auxWin;
		ScriptingWindow* scriptWin;
		coordinate selectedPixel = { 0,0 };
	protected:
		HICON m_hIcon;
	DECLARE_MESSAGE_MAP()
};
