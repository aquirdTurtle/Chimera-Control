#pragma once

#include <QMainWindow>
#include <QTimer>
#include "NIAWG/NiawgSystem.h"
#include "ConfigurationSystems/ProfileIndicator.h"
#include "ConfigurationSystems/profileSettings.h"
#include "ExperimentThread/Communicator.h"
#include "Agilent/Agilent.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include "IChimeraWindowWidget.h"

#include "Andor/CameraSettingsControl.h"
#include "ExperimentMonitoringAndStatus/ColorBox.h"
#include "GeneralImaging/PictureStats.h"
#include "GeneralImaging/PictureManager.h"
#include "ExperimentMonitoringAndStatus/AlertSystem.h"
#include "RealTimeDataAnalysis/DataAnalysisControl.h"
#include "ExperimentMonitoringAndStatus/ExperimentTimer.h"
#include "DataLogging/DataLogger.h"
#include "GeneralUtilityFunctions/commonFunctions.h"
#include "RealTimeDataAnalysis/atomCruncherInput.h"
#include "Andor/cameraPositions.h"
#include "GeneralObjects/commonTypes.h"
#include "GeneralObjects/Queues.h"
#include "IChimeraWindow.h"
#include <bitset>
#include "PrimaryWindows/IChimeraWindowWidget.h"

namespace Ui {
    class QtAndorWindow;
}

class QtAndorWindow : public IChimeraWindowWidget
{
    Q_OBJECT

    public:
        explicit QtAndorWindow (QWidget* parent=NULL);
        ~QtAndorWindow ();

		void initializeWidgets ();

		void OnTimer (UINT_PTR id);
		/// directly called by the message map or 1 simple step removed.
		void wakeRearranger ();
		void handleDblClick (NMHDR* info, LRESULT* lResult);
		void listViewRClick (NMHDR* info, LRESULT* lResult);
		void handleSpecialGreaterThanMaxSelection ();
		void handleSpecialLessThanMinSelection ();
		void readImageParameters ();
		void passSetTemperaturePress ();
		void passAlwaysShowGrid ();

		void setDataType (std::string dataType);
		/// auxiliary functions.
		void calibrate ();
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

		void redrawPictures (bool andGrid);
		bool getCameraStatus ();
		void setTimerText (std::string timerText);
		void armCameraWindow (AndorRunSettings* settings);

		std::string getStartMessage ();
		void handlePictureSettings ();
		bool cameraIsRunning ();
		void abortCameraRun ();
		void handleAutoscaleSelection ();
		void assertOff ();
		void passPictureSettings (UINT id);
		void prepareAtomCruncher (AllExperimentInput& input);
		void preparePlotter (AllExperimentInput& input);
		static UINT __stdcall atomCruncherProcedure (void* input);
		void writeVolts (UINT currentVoltNumber, std::vector<float64> data);
		friend void commonFunctions::handleCommonMessage (int msgID, IChimeraWindowWidget* win);
		void startAtomCruncher (AllExperimentInput& input);
		void startPlotterThread (AllExperimentInput& input);
		bool wantsAutoPause ();
		std::atomic<bool>* getSkipNextAtomic ();
		void stopPlotter ();
		void stopSound ();
		void handleImageDimsEdit ();
		void loadCameraCalSettings (AllExperimentInput& input);
		bool wasJustCalibrated ();
		bool wantsAutoCal ();
		bool wantsNoMotAlert ();
		UINT getNoMotThreshold ();
		atomGrid getMainAtomGrid ();
		std::string getMostRecentDateString ();
		int getMostRecentFid ();
		int getPicsPerRep ();
		bool wantsThresholdAnalysis ();
		AndorCameraCore& getCamera ();
		std::atomic<bool>& getPlotThreadActiveRef ();
		std::atomic<HANDLE>& getPlotThreadHandleRef ();
		std::mutex& getActivePlotMutexRef ();
		void cleanUpAfterExp ();
		void handlePlotPop (UINT id);

		void fillExpDeviceList (DeviceList& list);

	private:
        Ui::QtAndorWindow* ui;

		bool justCalibrated = false;
		UINT numExcessCounts = 0;
		AndorCameraCore andor;
		AndorCameraSettingsControl andorSettingsCtrl;
		PictureManager pics;
		int plotIds = 17009;
		PictureStats stats;
		AlertSystem alerts;
		ExperimentTimer timer;

		DataAnalysisControl analysisHandler;
		DataLogger dataHandler;
		std::vector<PlotCtrl*> mainAnalysisPlots;
		coordinate selectedPixel = { 0,0 };
		CMenu menu;
		// some picture menu options
		bool autoScalePictureData;
		bool alwaysShowGrid;
		bool specialLessThanMin;
		bool specialGreaterThanMax;
		bool realTimePic;
		// plotting stuff;
		std::atomic<HANDLE> plotThreadHandle;
		imageQueue imQueue;
		std::mutex imageQueueLock;
		std::condition_variable rearrangerConditionVariable;
		// the following two queues and locks aren't directly used by the camera window, but the camera window
		// distributes them to the threads that do use them.

		multiGridAtomQueue plotterAtomQueue;
		multiGridImageQueue plotterPictureQueue;
		atomQueue rearrangerAtomQueue;
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
		std::vector<PlotDialog*> activeDlgPlots;
		UINT mostRecentPicNum = 0;
		UINT currentPictureNum = 0;
		Matrix<long> avgBackground;

	public Q_SLOTS:
		void onCameraProgress (int picNum);

		LRESULT onCameraFinish (WPARAM wParam, LPARAM lParam);
		LRESULT onCameraCalFinish (WPARAM wParam, LPARAM lParam);
		LRESULT onCameraCalProgress (WPARAM wParam, LPARAM lParam);
		LRESULT onBaslerFinish (WPARAM wParam, LPARAM lParam);
		void handlePrepareForAcq (void* lparam);

};

