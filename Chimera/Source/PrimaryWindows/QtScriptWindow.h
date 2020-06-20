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

// a convenient structure for containing one object for each script. For example, the address of each script.
template <typename type> struct scriptInfo
{
	type niawg;
	type intensityAgilent;
	type master;
};


namespace Ui {
    class QtScriptWindow;
}

class QtScriptWindow : public IChimeraWindowWidget
{
    Q_OBJECT

    public:
        explicit QtScriptWindow (QWidget* parent=NULL);
        ~QtScriptWindow ();
		void initializeWidgets ();

		void fillExpDeviceList (DeviceList& list);
		void OnTimer (UINT_PTR eventID);
		void passRerngModeComboChange ();
		void passExperimentRerngButton ();
		void fillMotInput (ExperimentThreadInput* input);
		void checkScriptSaves ();
		void fillMasterThreadInput (ExperimentThreadInput* input);

		scriptInfo<std::string> getScriptNames ();
		scriptInfo<bool> getScriptSavedStatuses ();
		scriptInfo<std::string> getScriptAddresses ();

		profileSettings getProfileSettings ();
		std::string getSystemStatusString ();
		void updateDoAoNames ();
		void checkMasterSave ();
		
		void windowSaveConfig (ConfigStream& saveFile);
		void windowOpenConfig (ConfigStream& configFile);

		void updateScriptNamesOnScreen ();
		void updateProfile (std::string text);
		void considerScriptLocations ();
		void recolorScripts ();

		void newIntensityScript ();
		void openIntensityScript (IChimeraWindowWidget* parent);
		void openIntensityScript (std::string name);
		void saveIntensityScript ();
		void saveIntensityScriptAs (IChimeraWindowWidget* parent);
		void agilentEditChange ();

		void newNiawgScript ();
		void openNiawgScript (IChimeraWindowWidget* parent);
		void openNiawgScript (std::string name);
		void saveNiawgScript ();
		void saveNiawgScriptAs (IChimeraWindowWidget* parent);
		void niawgEditChange ();

		void newMasterScript ();
		void openMasterScript (IChimeraWindowWidget* parent);
		void openMasterScript (std::string name);
		void saveMasterScript ();
		void saveMasterScriptAs (IChimeraWindowWidget* parent);
		void newMasterFunction ();
		void saveMasterFunction ();
		void deleteMasterFunction ();

		void updateConfigurationSavedStatus (bool status);

		void handleIntensityButtons (UINT id);
		void handleNiawgScriptComboChange ();
		void handleAgilentScriptComboChange ();
		void handleMasterFunctionChange ();
		void handleIntensityCombo ();

		profileSettings getProfile ();
		void setIntensityDefault ();
		void passNiawgIsOnPress ();
		void setNiawgRunningState (bool newRunningState);
		void handleControlNiawgCheck ();
		void loadCameraCalSettings (ExperimentThreadInput* input);
		bool niawgIsRunning ();
		void stopRearranger ();
		void waitForRearranger ();
		void setNiawgDefaults ();
		void restartNiawgDefaults ();
		NiawgCore& getNiawg ();
		void stopNiawg ();
		void sendNiawgSoftwareTrig ();
		void streamNiawgWaveform ();
		std::string getNiawgErr ();
    private:
        Ui::QtScriptWindow* ui;
        NiawgSystem niawg;
        Script masterScript;
        ProfileIndicator profileDisplay;
        Agilent intensityAgilent;
	public Q_SLOTS:
		void updateVarNames ();
};

