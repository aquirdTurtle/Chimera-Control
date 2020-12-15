#pragma once

#include "ConfigurationSystems/ProfileIndicator.h"
#include "ConfigurationSystems/profileSettings.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include "IChimeraQtWindow.h"

#include "LowLevel/constants.h"
#include "DigitalOutput/DoSystem.h"
#include "AnalogOutput/AoSystem.h"
#include "ParameterSystem/ParameterSystem.h"
#include "Scripts/Script.h"
#include "GeneralObjects/RunInfo.h"
#include "MiscellaneousExperimentOptions/Repetitions.h"
#include "ConfigurationSystems/MasterConfiguration.h"
#include "GeneralObjects/commonTypes.h"
#include "ExperimentMonitoringAndStatus/StatusControl.h"
#include "DirectDigitalSynthesis/DdsSystem.h"
#include "RealTimeDataAnalysis/MachineOptimizer.h"
#include "ExperimentThread/ExperimentThreadInput.h"

#include <QMainWindow>
#include <QTimer>

namespace Ui {
    class QtAuxiliaryWindow;
}

class QtAuxiliaryWindow : public IChimeraQtWindow{
    Q_OBJECT

    public:
        explicit QtAuxiliaryWindow (QWidget* parent=NULL);
        ~QtAuxiliaryWindow ();

		void initializeWidgets ();
		void handleNormalFin ();
		void updateOptimization (AllExperimentInput& input);
		void handleMasterConfigSave (std::stringstream& configStream);
		void handleMasterConfigOpen (ConfigStream& configStream);
		/// Message Map Functions
		void ViewOrChangeTTLNames ();
		void ViewOrChangeDACNames ();
		void passRoundToDac ();
		std::string getOtherSystemStatusMsg ();
		Matrix<std::string> getTtlNames ();
		DoSystem* getTtlSystem ();
		std::array<AoInfo, 24> getDacInfo ();
		std::string getVisaDeviceStatus ();

		void fillMasterThreadInput (ExperimentThreadInput* input);
		void SetDacs ();

		void handleAbort ();
		void zeroDacs ();

	 
		std::vector<parameterType> getAllParams ();

		void clearVariables ();

		unsigned getTotalVariationNumber ();
		void windowSaveConfig (ConfigStream& saveFile);
		void windowOpenConfig (ConfigStream& configFile);
		std::pair<unsigned, unsigned> getTtlBoardSize ();
		unsigned getNumberOfDacs ();
		void setVariablesActiveState (bool active);
		DoCore& getTtlCore ();
		AoSystem& getAoSys ();
		ParameterSystem& getGlobals ();
		std::vector<parameterType> getUsableConstants ();
		void fillExpDeviceList (DeviceList& list);
	protected:
		bool eventFilter (QObject* obj, QEvent* event);

    private:
        Ui::QtAuxiliaryWindow* ui;
		std::string title;
		/// control system classes
		DoSystem ttlBoard;
		AoSystem aoSys;
		MasterConfiguration masterConfig{ MASTER_CONFIGURATION_FILE_ADDRESS };
		MachineOptimizer optimizer;
		ParameterSystem configParamCtrl, globalParamCtrl;
		DdsSystem dds;

		std::vector<PlotCtrl*> aoPlots;
		std::vector<PlotCtrl*> ttlPlots;
		unsigned NUM_DAC_PLTS = 3;
		unsigned NUM_TTL_PLTS = 4;

	public Q_SLOTS:
		void handleDoAoPlotData (const std::vector<std::vector<plotDataVec>>& doData,
							     const std::vector<std::vector<plotDataVec>>& aoData);
		void updateExpActiveInfo (std::vector<parameterType> expParams);
};

