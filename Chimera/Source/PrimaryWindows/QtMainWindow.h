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

#include "ConfigurationSystems/ProfileSystem.h"
#include "MiscellaneousExperimentOptions/DebugOptionsControl.h"
#include "MiscellaneousExperimentOptions/MainOptionsControl.h"
#include "ExperimentMonitoringAndStatus/StatusControl.h"
#include "ExperimentMonitoringAndStatus/StatusIndicator.h"
#include "Python/SmsTextingControl.h"
#include "Python/EmbeddedPythonHandler.h"
#include "ConfigurationSystems/MasterConfiguration.h"
#include "MiscellaneousExperimentOptions/Repetitions.h"
#include "DataLogging/DataLogger.h"
#include "ConfigurationSystems/NoteSystem.h"
#include "ConfigurationSystems/profileSettings.h"
#include "Plotting/PlotDialog.h"
#include "Plotting/ScopeViewer.h"
#include "GeneralUtilityFunctions/commonFunctions.h"
#include "CustomMessages.h"
#include "ExperimentThread/ExperimentThreadManager.h"
#include <string>
#include <vector>
#include <future>


namespace Ui {
    class QtMainWindow;
}


class QtMainWindow : public IChimeraWindowWidget
{
    Q_OBJECT

    public:
		explicit QtMainWindow (CDialog* splash, chronoTime* startTime);
        ~QtMainWindow ();

		void initializeWidgets ();

		void showHardwareStatus ();

		void fillExpDeviceList (DeviceList& list);
		// stuff directly called (or 1 simple step removed) by message map.
		//void onErrorMessage (std::string statusMessage);
		void onFatalErrorMessage (std::string statusMessage);

		void onDebugMessage (std::string msg);
		LRESULT onNoAtomsAlertMessage (WPARAM wp, LPARAM lp);
		LRESULT onNoMotAlertMessage (WPARAM wp, LPARAM lp);
		LRESULT onFinish (WPARAM wp, LPARAM lp);

		void onNormalFinishMessage ();
		void onGreyTempCalFin ();
		void onMachineOptRoundFin ();
		void handleThresholdAnalysis ();
		void onAutoCalFin ();
		void setStyleSheets ();
		//
		void loadCameraCalSettings (ExperimentThreadInput* input);
		void handlePause ();
		DeviceList getDevices ();
		CFont* getPlotFont ();
		// auxiliary functions used by the window.
		void setNotes (std::string notes);
		void fillMasterThreadInput (ExperimentThreadInput* input);
		void fillMotInput (ExperimentThreadInput* input);

		void startExperimentThread (ExperimentThreadInput* input);

		std::string getNotes ();
		fontMap getFonts ();
		profileSettings getProfileSettings ();
		debugInfo getDebuggingOptions ();
		mainOptions getMainOptions ();

		void checkProfileReady ();
		void checkProfileSave ();
		void updateConfigurationSavedStatus (bool status);

		void setDebuggingOptions (debugInfo options);
		void updateStatusText (std::string whichStatus, std::string text);
		void addTimebar (std::string whichStatus);
		void setShortStatus (std::string text);
		void changeShortStatusColor (std::string color);
		void changeBoxColor (std::string sysDelim, std::string color);
		void windowSaveConfig (ConfigStream& saveFile);
		void windowOpenConfig (ConfigStream& configFile);
		void abortMasterThread ();
		Communicator* getComm ();
		std::string getSystemStatusString ();
		bool masterIsRunning ();
		RunInfo getRunInfo ();
		void handleFinish ();
		UINT getRepNumber ();
		void logParams (DataLogger* logger, ExperimentThreadInput* input);
		bool experimentIsPaused ();
		void notifyConfigUpdate ();

		void OnTimer (UINT_PTR id);

		LRESULT autoServo (WPARAM w, LPARAM l);
		void runServos ();
		std::vector<servoInfo> getServoinfo ();
		void handleMasterConfigSave (std::stringstream& configStream);
		void handleMasterConfigOpen (ConfigStream& configStream);
		bool autoF5_AfterFinish = false;
		EmbeddedPythonHandler& getPython ();
		Communicator& getCommRef ();
		UINT getAutoCalNumber ();

	public Q_SLOTS:
		void handleColorboxUpdate (QString color, QString systemDelim);
		void handleExpNotification (QString txt);
		void onRepProgress (unsigned int repNum);
		void onErrorMessage (QString errMessage);

    private:
        Ui::QtMainWindow* ui;

		chronoTimes startupTimes;
		chronoTime* programStartTime;
		// members that have gui elements
		ProfileSystem profile;
		MasterConfiguration masterConfig;
		NoteSystem notes;
		DebugOptionsControl debugger;
		Repetitions repetitionControl;
		MainOptionsControl mainOptsCtrl;
		StatusControl mainStatus;
		StatusControl debugStatus;
		StatusControl errorStatus;
		SmsTextingControl texter;
		StatusIndicator shortStatus;
		ServoManager servos;
		// auxiliary members
		Communicator comm;
		fontMap mainFonts;
		ExperimentThreadManager expThreadManager;
		RunInfo systemRunningInfo;
		EmbeddedPythonHandler python;
		ScopeViewer masterRepumpScope, motScope;
		CFont* plotfont;
		//
		static BOOL CALLBACK monitorHandlingProc (_In_ HMONITOR hMonitor, _In_ HDC      hdcMonitor,
			_In_ LPRECT lprcMonitor, _In_ LPARAM dwData);
		std::vector<Gdiplus::Pen*> plotPens, brightPlotPens;
		std::vector<Gdiplus::SolidBrush*> plotBrushes, brightPlotBrushes;
		CDialog* appSplash;
		friend void commonFunctions::handleCommonMessage (int msgID, IChimeraWindowWidget* win);
		UINT autoCalNum = 0;

};

