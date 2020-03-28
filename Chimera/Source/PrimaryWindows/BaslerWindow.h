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
#include "IChimeraWindow.h"

class MainWindow;
class ScriptingWindow;
class AndorWindow;
class AuxiliaryWindow;
class BaslerWindow;
class DeformableMirrorWindow;

class BaslerWindow : public IChimeraWindow
{
 	public:
		BaslerWindow( );
		BOOL OnInitDialog();
		void handleBaslerAutoscaleSelection ( );
		void OnPaint();
		void startTemporaryAcquisition ( baslerSettings motSizeSettings );
		void startDefaultAcquisition ( );
		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		void OnSize( UINT nType, int cx, int cy );
		void prepareWinForAcq(baslerSettings* settings);
		void handleClose();
		void handleDisarmPress();
		void initializeControls();
		void passExposureMode();
		void passCameraMode();
		bool baslerCameraIsRunning ( );
		bool baslerCameraIsContinuous ( );
		LRESULT handleNewPics( WPARAM wParam, LPARAM lParam );
		LRESULT handlePrepareRequest(WPARAM wParam, LPARAM lParam);
		void pictureRangeEditChange( UINT id );
		void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* scrollbar );
		void handleSoftwareTrigger();
		void windowOpenConfig ( ConfigStream& configFile, Version ver );
		void OnMouseMove(UINT flags, CPoint point );
		void OnRButtonUp( UINT stuff, CPoint clickLocation );
		void windowSaveConfig ( ConfigStream& configFile );
		baslerSettings getCurrentSettings ( );
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
		unsigned int currentRepNumber;
		BaslerAutoExposure::mode runExposureMode;
		std::vector<std::vector<long>> images;
		bool isRunning;
		PlotCtrl *horGraph, *vertGraph;
		std::vector<Gdiplus::Pen*> plotPens, brightPlotPens;
		std::vector<Gdiplus::SolidBrush*> plotBrushes, brightPlotBrushes;
		CFont* plotfont;
		coordinate selectedPixel = { 0,0 };
		HICON m_hIcon;
		DECLARE_MESSAGE_MAP()
};
