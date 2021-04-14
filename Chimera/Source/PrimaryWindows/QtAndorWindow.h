#pragma once

#include <QMainWindow>
#include <QTimer>
#include "NIAWG/NiawgSystem.h"
#include "ConfigurationSystems/ProfileIndicator.h"
#include "ConfigurationSystems/profileSettings.h"
#include "Agilent/Agilent.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include "IChimeraQtWindow.h"

#include "Andor/AndorCameraSettingsControl.h"
#include "ExperimentMonitoringAndStatus/ColorBox.h"
#include "GeneralImaging/PictureStats.h"
#include "GeneralImaging/PictureManager.h"
#include "ExperimentMonitoringAndStatus/AlertSystem.h"
#include "RealTimeDataAnalysis/DataAnalysisControl.h"
#include "ExperimentMonitoringAndStatus/ExperimentTimer.h"
#include "DataLogging/DataLogger.h"
#include "GeneralUtilityFunctions/commonFunctions.h"
#include "RealTimeDataAnalysis/atomCruncherInput.h"
#include "GeneralObjects/commonTypes.h"
#include "GeneralObjects/Queues.h"
#include <Piezo/PiezoController.h>
#include <Python/NewPythonHandler.h>
#include <Plotting/QCustomPlotCtrl.h>
#include <bitset>

class AnalysisThreadWorker;
class CruncherThreadWorker;

namespace Ui {
    class QtAndorWindow;
}

class QtAndorWindow : public IChimeraQtWindow{
    Q_OBJECT

    public:
        explicit QtAndorWindow (QWidget* parent=nullptr);
        ~QtAndorWindow ();

		void initializeWidgets ();

		void handleBumpAnalysis (profileSettings finishedProfile);
		/// directly called by the message map or 1 simple step removed.
		void wakeRearranger ();
		void handleSpecialGreaterThanMaxSelection ();
		void handleSpecialLessThanMinSelection ();
		void readImageParameters ();
		void passSetTemperaturePress ();
		void passAlwaysShowGrid ();

		void setDataType (std::string dataType);
		/// auxiliary functions.
		dataPoint getMainAnalysisResult ();
		void checkCameraIdle ();
		void handleEmGainChange ();
		void fillMasterThreadInput (ExperimentThreadInput* input);
		DataLogger& getLogger ();
		std::string getSystemStatusString ();
		void windowSaveConfig (ConfigStream& saveFile);
		void windowOpenConfig (ConfigStream& configFile);

		void handleMasterConfigSave (std::stringstream& configStream);
		void handleMasterConfigOpen (ConfigStream& configStream);
		void handleNormalFinish (profileSettings finishedProfile);
		void copyDataFile(std::string specialName="");
		void redrawPictures (bool andGrid);
		bool getCameraStatus ();
		void setTimerText (std::string timerText);
		void armCameraWindow (AndorRunSettings* settings);
		int getDataCalNum ();
		std::string getStartMessage ();
		void handlePictureSettings ();
		bool cameraIsRunning ();
		void abortCameraRun ();
		void assertOff ();
		void assertDataFileClosed ();
		void prepareAtomCruncher (AllExperimentInput& input);
		void writeVolts (unsigned currentVoltNumber, std::vector<float64> data);
		friend void commonFunctions::handleCommonMessage (int msgID, IChimeraQtWindow* win);
		bool wantsAutoPause ();
		std::atomic<bool>* getSkipNextAtomic ();
		void stopPlotter ();
		void stopSound ();
		void handleImageDimsEdit ();
		void loadCameraCalSettings (AllExperimentInput& input);
		bool wasJustCalibrated ();
		bool wantsAutoCal ();
		bool wantsNoMotAlert ();
		unsigned getNoMotThreshold ();
		atomGrid getMainAtomGrid ();
		std::string getMostRecentDateString ();
		int getMostRecentFid ();
		int getPicsPerRep ();
		bool wantsThresholdAnalysis ();
		AndorCameraCore& getCamera ();
		void cleanUpAfterExp ();
		void handlePlotPop (unsigned id);

		void fillExpDeviceList (DeviceList& list);
		void handleSetAnalysisPress ();
		piezoChan<double> getAlignmentVals ();
		CruncherThreadWorker* atomCruncherWorker;
		AnalysisThreadWorker* analysisThreadWorker;
		void handleTransformationModeChange ();
		void manualArmCamera ();
		NewPythonHandler* getPython ();

	private:
        Ui::QtAndorWindow* ui;

		bool justCalibrated = false;
		unsigned numExcessCounts = 0;
		AndorCameraCore andor;
		AndorCameraSettingsControl andorSettingsCtrl;
		PictureManager pics;
		PictureStats stats;
		AlertSystem alerts;
		ExperimentTimer timer;

		DataAnalysisControl analysisHandler;
		NewPythonHandler pythonHandler;
		DataLogger dataHandler;
		std::vector<QCustomPlotCtrl*> mainAnalysisPlots;
		coordinate selectedPixel = { 0,0 };
		
		// some picture menu options
		bool autoScalePictureData=false;
		bool alwaysShowGrid=false;
		bool specialLessThanMin=false;
		bool specialGreaterThanMax=false;
		bool realTimePic=false;
		// plotting stuff;
		std::atomic<HANDLE> plotThreadHandle;
		std::condition_variable rearrangerConditionVariable;
		std::mutex plotLock;
		std::mutex rearrangerLock;

		dataPoint mostRecentAnalysisResult;
		// 
		HANDLE atomCruncherThreadHandle;
		std::atomic<bool> atomCrunchThreadActive;
		// 
		std::atomic<bool> plotThreadActive;
		std::atomic<bool> plotThreadAborting = false;
		std::atomic<bool> skipNext = false;
		std::vector<double> plotterKey;
		chronoTimes imageTimes, imageGrabTimes, mainThreadStartTimes, crunchSeesTimes, crunchFinTimes;
		std::mutex activePlotMutex;
		unsigned mostRecentPicNum = 0;
		unsigned currentPictureNum = 0;
		Matrix<long> avgBackground;
		PiezoController imagingPiezo;
	Q_SIGNALS:
		void newImage (NormalImage);

	public Q_SLOTS:
		void onCameraProgress (int picNum);
		LRESULT onCameraCalFinish (WPARAM wParam, LPARAM lParam);
		void handlePrepareForAcq (AndorRunSettings* lparam, analysisSettings aSettings);
		void completePlotterStart ();
		void completeCruncherStart ();

};

