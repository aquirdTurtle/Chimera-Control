#pragma once

#include <QMainWindow>
#include <QTimer>
#include "NIAWG/NiawgSystem.h"
#include "ConfigurationSystems/ProfileIndicator.h"
#include "ConfigurationSystems/profileSettings.h"
#include "ExperimentThread/Communicator.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include "IChimeraWindowWidget.h"

#include <atomic>
#include "GeneralImaging/PictureManager.h"
#include "GeneralImaging/PictureControl.h"
#include "Control.h"
#include "IChimeraWindow.h"
#include "Basler/BaslerSettingsControl.h"
#include "Basler/BaslerCamera.h"
#include "GeneralImaging/PictureStats.h"
#include "GeneralObjects/commonTypes.h"
#include "Plotting/PlotCtrl.h"

namespace Ui {
    class QtBaslerWindow;
}

class QtBaslerWindow : public IChimeraWindowWidget
{
    Q_OBJECT

    public:
        explicit QtBaslerWindow (QWidget* parent=NULL);
        ~QtBaslerWindow ();
		void initializeWidgets ();
		void handleBaslerAutoscaleSelection ();
		void startTemporaryAcquisition (baslerSettings motSizeSettings);
		void startDefaultAcquisition ();
		void prepareWinForAcq (baslerSettings* settings);
		void handleDisarmPress ();
		void initializeControls ();
		void passExposureMode ();
		void passCameraMode ();
		bool baslerCameraIsRunning ();
		bool baslerCameraIsContinuous ();
		LRESULT handleNewPics (WPARAM wParam, LPARAM lParam);
		LRESULT handlePrepareRequest (WPARAM wParam, LPARAM lParam);
		void pictureRangeEditChange (UINT id);
		void handleSoftwareTrigger ();
		void windowOpenConfig (ConfigStream& configFile);
		void windowSaveConfig (ConfigStream& configFile);
		baslerSettings getCurrentSettings ();
		void fillExpDeviceList (DeviceList& list);

    private:
        Ui::QtBaslerWindow* ui;

		// for the basler window, this is typically only one picture, but I include this here anyways.
		UINT loadMotConsecutiveFailures = 0;
		bool motLoaded = false;
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
		PlotCtrl* horGraph, * vertGraph;
		std::vector<Gdiplus::Pen*> plotPens, brightPlotPens;
		std::vector<Gdiplus::SolidBrush*> plotBrushes, brightPlotBrushes;
		CFont* plotfont;
		coordinate selectedPixel = { 0,0 };
		HICON m_hIcon;
};

