#pragma once

#include <QMainWindow>
#include <QTimer>
#include "NIAWG/NiawgSystem.h"
#include "ConfigurationSystems/ProfileIndicator.h"
#include "ConfigurationSystems/profileSettings.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include "IChimeraQtWindow.h"

#include "GeneralImaging/PictureManager.h"
#include "GeneralImaging/PictureControl.h"
#include "IChimeraQtWindow.h"
#include "Basler/BaslerSettingsControl.h"

#include "GeneralImaging/PictureStats.h"
#include "GeneralObjects/commonTypes.h"
#include "Plotting/PlotCtrl.h"
#include <atomic>

class BaslerCameraCore;

namespace Ui {
    class QtBaslerWindow;
}

class QtBaslerWindow : public IChimeraQtWindow{
    Q_OBJECT

    public:
        explicit QtBaslerWindow (QWidget* parent=nullptr);
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
		bool isRunning;
		PlotCtrl* horGraph, * vertGraph;
		coordinate selectedPixel = { 0,0 };
	public Q_SLOTS:
		void handleNewPics (Matrix<long> imageMatrix);
};

