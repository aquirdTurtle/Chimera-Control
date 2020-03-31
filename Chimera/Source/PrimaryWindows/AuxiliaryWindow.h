// created by Mark O. Brown
#pragma once
#include <unordered_map>
#include <string>

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
#include "ConfigurationSystems/Version.h"
#include "IChimeraWindow.h"

// short for which agilent. Putting the agilentNames in a struct is a trick that makes using the scope whichAg:: 
// required while allowing implicit int conversion, which is useful for these. 
struct whichAg
{
	enum agilentNames
	{
		TopBottom,
		Axial,
		Flashing,
		Microwave
	};
};


class BaslerWindow;
class DeformableMirrorWindow;

// The Device window houses most of the controls for seeting individual devices, other than the camera which gets its 
// own control. It also houses a couple auxiliary things like variables and the SMS texting control.
class AuxiliaryWindow : public IChimeraWindow
{
	DECLARE_DYNAMIC(AuxiliaryWindow);
	public:
		AuxiliaryWindow();
		BOOL handleAccelerators( HACCEL m_haccel, LPMSG lpMsg );
		void updateOptimization ( AllExperimentInput& input );
		void OnRButtonUp( UINT stuff, CPoint clickLocation );
		void OnLButtonUp( UINT stuff, CPoint clickLocation );
		BOOL OnInitDialog();
		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		void OnSize(UINT nType, int cx, int cy);
		void OnPaint( );
		void OnTimer( UINT_PTR id );
		std::vector<std::reference_wrapper<PiezoCore> > getPiezoControllers ();
		LRESULT onLogVoltsMessage( WPARAM wp, LPARAM lp );
		void handlePlotPop (UINT id);
		void uwDblClick (NMHDR* pNotifyStruct, LRESULT* result);
		void uwRClick (NMHDR* pNotifyStruct, LRESULT* result);
		void handleMasterConfigSave( std::stringstream& configStream );
		void handleMasterConfigOpen(ConfigStream& configStream);
		BOOL PreTranslateMessage(MSG* pMsg);
		/// Message Map Functions
		void programPiezo1 ( );
		void programPiezo2 ( );
		void handlePiezo1Ctrl ( );
		void handlePiezo2Ctrl ( );
		void handleTtlPush(UINT id);
		void handlTtlHoldPush();
		void ViewOrChangeTTLNames();
		void ViewOrChangeDACNames();
		void passRoundToDac();
		std::string getOtherSystemStatusMsg();
		Matrix<std::string> getTtlNames();
		DoSystem* getTtlSystem ();
		std::array<AoInfo, 24> getDacInfo ( );
		void GetAnalogInSnapshot( );
		std::string getVisaDeviceStatus( );
		std::string getMicrowaveSystemStatus( );

		void updateAgilent( whichAg::agilentNames name );
		void newAgilentScript( whichAg::agilentNames name );
		void openAgilentScript( whichAg::agilentNames name, CWnd* parent );
		void saveAgilentScript( whichAg::agilentNames name );
		void saveAgilentScriptAs( whichAg::agilentNames name, CWnd* parent );
		void handleAgilentEditChange( UINT id );
		void drawVariables(UINT id, NMHDR* pNMHDR, LRESULT* pResultf);
		void fillMasterThreadInput(ExperimentThreadInput* input);
		void DacEditChange(UINT id);
		void SetDacs();
		
		fontMap getFonts();

		void handleAbort();
		void zeroTtls();
		void zeroDacs();

		void handleAgilentOptions( UINT id );

		void handleTektronixButtons(UINT id);
		void invalidateSaved ( UINT id );

		std::vector<parameterType> getAllVariables();

		void GlobalVarDblClick(NMHDR * pNotifyStruct, LRESULT * result);
		void GlobalVarRClick(NMHDR * pNotifyStruct, LRESULT * result);
		void ConfigVarsColumnClick(NMHDR * pNotifyStruct, LRESULT * result);
		void clearVariables();
		void addVariable(std::string name, bool constant, double value);
		void ConfigVarsDblClick(NMHDR * pNotifyStruct, LRESULT * result);
		void ConfigVarsSingleClick ( NMHDR * pNotifyStruct, LRESULT * result );
		void ConfigVarsRClick(NMHDR * pNotifyStruct, LRESULT * result);
		void DdsRClick ( NMHDR * pNotifyStruct, LRESULT * result );
		void DdsDblClick ( NMHDR * pNotifyStruct, LRESULT * result );


		void OptParamDblClick ( NMHDR * pNotifyStruct, LRESULT * result );
		void OptParamRClick ( NMHDR * pNotifyStruct, LRESULT * result );

		UINT getTotalVariationNumber();
		void windowSaveConfig(ConfigStream& saveFile);
		void windowOpenConfig (ConfigStream& configFile);
		std::pair<UINT, UINT> getTtlBoardSize();
		UINT getNumberOfDacs();
		void setVariablesActiveState(bool active);
		void passTopBottomTekProgram();
		void passEoAxialTekProgram();
		Agilent& whichAgilent( UINT id );
		void handleAgilentCombo( UINT id );
		void autoOptimize ( );
		DoCore& getTtlCore ( );
		AoSystem& getAoSys ( );
		AiSystem& getAiSys ( );
		MicrowaveCore& getRsg ( );
		TekCore& getTopBottomTek ( );
		TekCore& getEoAxialTek( );
		ParameterSystem& getGlobals ( );
		DdsCore& getDds ( );
		void handleProgramUwSystemNow ();
		void programDds ( );
		std::vector<parameterType> getUsableConstants ();
	private:
		DECLARE_MESSAGE_MAP();		

		int plotIds = 17002;
		CMenu menu;
		std::string title;
		toolTipTextMap toolTipText;
		/// control system classes
		MicrowaveSystem RohdeSchwarzGenerator;
		std::array<Agilent, 4> agilents;

		std::vector<PlotCtrl*> aoPlots;
		std::vector<PlotCtrl*> ttlPlots;
		std::vector<std::vector<pPlotDataVec>> ttlData, dacData;
		UINT NUM_DAC_PLTS = 3;
		UINT NUM_TTL_PLTS = 4;
		
 		DoSystem ttlBoard;
 		AoSystem aoSys;
		AiSystem aiSys;
 		MasterConfiguration masterConfig{ MASTER_CONFIGURATION_FILE_ADDRESS };
		TektronixAfgControl topBottomTek, eoAxialTek;
		MachineOptimizer optimizer;
		ParameterSystem configParameters, globalParameters;
		DdsSystem dds;
		
		PiezoController piezo1, piezo2;
};
