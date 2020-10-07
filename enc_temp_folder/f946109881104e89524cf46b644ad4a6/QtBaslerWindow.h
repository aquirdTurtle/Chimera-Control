#pragma once

#include <QMainWindow>
#include <QTimer>
#include "NIAWG/NiawgSystem.h"
#include "ConfigurationSystems/ProfileIndicator.h"
#include "ConfigurationSystems/profileSettings.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include "IChimeraQtWindow.h"

#include <atomic>
#include "GeneralImaging/PictureManager.h"
#include "GeneralImaging/PictureControl.h"
#include "Control.h"
#include "IChimeraQtWindow.h"
#include "Basler/BaslerSettingsControl.h"

#include "GeneralImaging/PictureStats.h"
#include "GeneralObjects/commonTypes.h"
#include "Plotting/PlotCtrl.h"

class BaslerCameraCore;

namespace Ui {
    class QtBaslerWindow;
}

class QtBaslerWindow : public IChimeraQtWindow{
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
		void handlePrepareRequest (baslerSettings* settings);
		void pictureRangeEditChange (unsigned id);
		void handleSoftwareTrigger ();
		void windowOpenConfig (ConfigStream& configFile);
		void windowSaveConfig (ConfigStream& configFile);
		baslerSettings getCurrentSettings ();
		void fillExpDeviceList (DeviceList& list);

    private:
        Ui::QtBaslerWindow* ui;
		baslerSettings runSettings;
		// for the basler window, this is typically only one picture, but I include this here anyways.
		unsigned loadMotConsecutiveFailures = 0;
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
		coordinate selectedPixel = { 0,0 };
		HICON m_hIcon;
	public Q_SLOTS:
		void handleNewPics (Matrix<long> imageMatrix);
};

