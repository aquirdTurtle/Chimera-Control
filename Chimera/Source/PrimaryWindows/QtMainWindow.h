#pragma once

#include <QMainWindow>
#include <QTimer>
#include "NIAWG/NiawgSystem.h"
#include "ConfigurationSystems/ProfileIndicator.h"
#include "ConfigurationSystems/profileSettings.h"
#include "Agilent/Agilent.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include "IChimeraQtWindow.h"
#include <AnalogInput/CalibrationManager.h>
#include "ConfigurationSystems/ConfigSystem.h"
#include "MiscellaneousExperimentOptions/DebugOptionsControl.h"
#include "MiscellaneousExperimentOptions/MainOptionsControl.h"
#include "ExperimentMonitoringAndStatus/StatusControl.h"
#include "ExperimentMonitoringAndStatus/StatusIndicator.h"
#include "Python/SmsTextingControl.h"
#include "ConfigurationSystems/MasterConfiguration.h"
#include "MiscellaneousExperimentOptions/Repetitions.h"
#include "DataLogging/DataLogger.h"
#include "ConfigurationSystems/NoteSystem.h"
#include "ConfigurationSystems/profileSettings.h"
#include "Plotting/ScopeViewer.h"
#include "GeneralUtilityFunctions/commonFunctions.h"
#include "CustomMessages.h"
#include <string>
#include <vector>
#include <future>

namespace Ui {
    class QtMainWindow;
} 

class QtMainWindow : public IChimeraQtWindow{
    Q_OBJECT
		 
    public:
		explicit QtMainWindow ();
		~QtMainWindow () {};
		bool expIsRunning ();
		void initializeWidgets ();
		void showHardwareStatus ();

		void fillExpDeviceList (DeviceList& list);

		LRESULT onNoAtomsAlertMessage (WPARAM wp, LPARAM lp);
		LRESULT onNoMotAlertMessage (WPARAM wp, LPARAM lp);

		void onMachineOptRoundFin ();
		void onAutoCalFin (QString msg, profileSettings finishedConfig);
		void setStyleSheets ();
		//
		void loadCameraCalSettings (ExperimentThreadInput* input);
		void handlePauseToggle ();
		DeviceList getDevices ();
		// auxiliary functions used by the window.
		void setNotes (std::string notes);
		void fillMasterThreadInput (ExperimentThreadInput* input);
		void fillMotInput (ExperimentThreadInput* input);

		void startExperimentThread (ExperimentThreadInput* input);

		std::string getNotes ();
		profileSettings getProfileSettings ();
		debugInfo getDebuggingOptions ();
		mainOptions getMainOptions ();

		void checkProfileSave ();
		void updateConfigurationSavedStatus (bool status);

		void setDebuggingOptions (debugInfo options);
		void addTimebar (std::string whichStatus);
		void setShortStatus (std::string text);
		void changeShortStatusColor (std::string color);
		void changeBoxColor (std::string sysDelim, std::string color);
		void windowSaveConfig (ConfigStream& saveFile);
		void windowOpenConfig (ConfigStream& configFile);
		void abortMasterThread ();
		std::string getSystemStatusString ();
		bool masterIsRunning ();
		RunInfo getRunInfo ();
		void handleFinishText ();
		unsigned getRepNumber ();
		void logParams (DataLogger* logger, ExperimentThreadInput* input);
		bool experimentIsPaused ();
		void notifyConfigUpdate ();

		void handleMasterConfigSave (std::stringstream& configStream);
		void handleMasterConfigOpen (ConfigStream& configStream);
		bool autoF5_AfterFinish = false;
		unsigned getAutoCalNumber ();

		QThread* getExpThread();
		ExpThreadWorker* getExpThreadWorker();
		void pauseExperiment ();
		std::vector<calResult> getCalInfo ();
	public Q_SLOTS:
		void handleColorboxUpdate (QString color, QString systemDelim);
		void handleNotification(statusMsg errMsg);
		void onRepProgress (unsigned int repNum);
		void onErrorMessage(statusMsg errMsg);
		void onNormalFinish (statusMsg, profileSettings finishedProfile );
		void onFatalError (statusMsg);

    private:
        Ui::QtMainWindow* ui;

		chronoTimes startupTimes;
		chronoTime* programStartTime;
		// members that have gui elements
		ConfigSystem profile;
		MasterConfiguration masterConfig;
		NoteSystem notes;
		DebugOptionsControl debugger;
		Repetitions repetitionControl;
		MainOptionsControl mainOptsCtrl;
		StatusControl mainStatus;
		StatusControl errorStatus;
		SmsTextingControl texter;
		StatusIndicator shortStatus;
		CalibrationManager calManager;

		ExpThreadWorker* expWorker=nullptr;
		QThread* expThread=nullptr;
		std::atomic<bool> experimentIsRunning = false;

		RunInfo systemRunningInfo;
		ScopeViewer masterRepumpScope, motScope, expScope;
		//
		friend void commonFunctions::handleCommonMessage (int msgID, IChimeraQtWindow* win);
		unsigned autoCalNum = 0;	
};

