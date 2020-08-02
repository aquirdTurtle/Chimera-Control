#pragma once

#include "NIAWG/NiawgSystem.h"
#include "ConfigurationSystems/ProfileIndicator.h"
#include "ConfigurationSystems/profileSettings.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include "IChimeraQtWindow.h"

#include "LowLevel/constants.h"
#include "ExperimentThread/ExperimentThreadManager.h" 
#include "Piezo/PiezoController.h"
#include "DigitalOutput/DoSystem.h"
#include "AnalogOutput/AoSystem.h"
#include "ParameterSystem/ParameterSystem.h"
#include "Scripts/Script.h"
#include "GeneralObjects/RunInfo.h"
#include "MiscellaneousExperimentOptions/Repetitions.h"
#include "Microwave/MicrowaveSystem.h"
#include "GeneralFlumes/GpibFlume.h"
#include "ConfigurationSystems/MasterConfiguration.h"
#include "Agilent/Agilent.h"
#include "GeneralObjects/commonTypes.h"
#include "ExperimentMonitoringAndStatus/StatusControl.h"
#include "Tektronix/TektronixAfgControl.h"
#include "AnalogInput/AiSystem.h"
#include "AnalogInput/ServoManager.h"
#include "DirectDigitalSynthesis/DdsSystem.h"
#include "RealTimeDataAnalysis/MachineOptimizer.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include <Agilent/whichAg.h>

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
		std::vector<std::reference_wrapper<PiezoCore> > getPiezoControllers ();
		LRESULT onLogVoltsMessage (WPARAM wp, LPARAM lp);
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
		std::string getMicrowaveSystemStatus ();

		void updateAgilent (whichAgTy::agilentNames name);
		void newAgilentScript (whichAgTy::agilentNames name);
		void openAgilentScript (whichAgTy::agilentNames name, IChimeraQtWindow* parent);
		void saveAgilentScript (whichAgTy::agilentNames name);
		void saveAgilentScriptAs (whichAgTy::agilentNames name, IChimeraQtWindow* parent);
		void fillMasterThreadInput (ExperimentThreadInput* input);
		void SetDacs ();

		void handleAbort ();
		void zeroDacs ();

		void invalidateSaved (unsigned id);
		 
		std::vector<parameterType> getAllParams ();

		void clearVariables ();

		unsigned getTotalVariationNumber ();
		void windowSaveConfig (ConfigStream& saveFile);
		void windowOpenConfig (ConfigStream& configFile);
		std::pair<unsigned, unsigned> getTtlBoardSize ();
		unsigned getNumberOfDacs ();
		void setVariablesActiveState (bool active);
		Agilent& whichAgilent (unsigned id);
		DoCore& getTtlCore ();
		AoSystem& getAoSys ();
		AiSystem& getAiSys ();
		ParameterSystem& getGlobals ();
		std::vector<parameterType> getUsableConstants ();
		void fillExpDeviceList (DeviceList& list);

	protected:
		bool eventFilter (QObject* obj, QEvent* event);

    private:
        Ui::QtAuxiliaryWindow* ui;

		int plotIds = 17002;
		
		std::string title;
		/// control system classes
		MicrowaveSystem RohdeSchwarzGenerator;
		std::array<Agilent, 4> agilents;
		DoSystem ttlBoard;
		AoSystem aoSys;
		AiSystem aiSys;
		MasterConfiguration masterConfig{ MASTER_CONFIGURATION_FILE_ADDRESS };
		TektronixAfgControl topBottomTek, eoAxialTek;
		MachineOptimizer optimizer;
		ParameterSystem configParameters, globalParameters;
		DdsSystem dds;
		PiezoController piezo1, piezo2, piezo3;

		std::vector<PlotCtrl*> aoPlots;
		std::vector<PlotCtrl*> ttlPlots;
		std::vector<std::vector<pPlotDataVec>> ttlData, dacData;
		unsigned NUM_DAC_PLTS = 3;
		unsigned NUM_TTL_PLTS = 4;

	public Q_SLOTS:
		void handleDoAoPlotData (const std::vector<std::vector<plotDataVec>>& doData,
							     const std::vector<std::vector<plotDataVec>>& aoData);
};

