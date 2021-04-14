#pragma once

#include <QMainWindow>
#include <QTimer>
#include "NIAWG/NiawgSystem.h"
#include "ConfigurationSystems/ProfileIndicator.h"
#include "ConfigurationSystems/profileSettings.h"
#include "Agilent/Agilent.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include "IChimeraQtWindow.h"
 
// a convenient structure for containing one object for each script. For example, the address of each script.
template <typename type> struct scriptInfo{
	type niawg;
	type intensityAgilent;
	type master;
};

namespace Ui {
    class QtScriptWindow;
}

class QtScriptWindow : public IChimeraQtWindow{
    Q_OBJECT

    public:
        explicit QtScriptWindow (QWidget* parent=nullptr);
        ~QtScriptWindow ();
		void initializeWidgets ();

		void fillExpDeviceList (DeviceList& list);
		void passRerngModeComboChange ();
		void passExperimentRerngButton ();
		void checkScriptSaves ();

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

		void newIntensityScript ();
		void openIntensityScript (IChimeraQtWindow* parent);
		void openIntensityScript (std::string name);
		void saveIntensityScript ();
		void saveIntensityScriptAs (IChimeraQtWindow* parent);

		void newNiawgScript ();
		void openNiawgScript (IChimeraQtWindow* parent);
		void openNiawgScript (std::string name);
		void saveNiawgScript ();
		void saveNiawgScriptAs (IChimeraQtWindow* parent);

		void newMasterScript ();
		void openMasterScript (IChimeraQtWindow* parent);
		void openMasterScript (std::string name);
		void saveMasterScript ();
		void saveMasterScriptAs (IChimeraQtWindow* parent);
		void newMasterFunction ();
		void saveMasterFunction ();
		void deleteMasterFunction ();

		void updateConfigurationSavedStatus (bool status);

		void handleMasterFunctionChange ();
		void handleIntensityCombo ();

		profileSettings getProfile ();
		void setIntensityDefault ();
		void passNiawgIsOnPress ();
		void setNiawgRunningState (bool newRunningState);
		void handleControlNiawgCheck ();
		bool niawgIsRunning ();
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

