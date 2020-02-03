// created by Mark O. Brown
#pragma once
#include "Control.h"
#include "ExperimentThread/MasterThreadInput.h"
#include "CustomMfcControlWrappers/MyListCtrl.h"
#include "Plotting/dataPoint.h"
#include "CustomMfcControlWrappers/DoubleEdit.h"
#include <array>
#include <memory>
#include "DataLogging/DataLogger.h"

struct optimizationAlgorithm
{
	enum class which
	{
		HillClimbing,
		HillSplitting,
		NeuralNetwork,
		Genetic,
		RandomizedSearch
	};
	which fromStr ( std::string txt );
	std::string toStr ( which alg );
};


struct optimizationSettings
{
	optimizationAlgorithm::which alg = optimizationAlgorithm::which::HillClimbing;  
	double tolerance;
	//double gain = 0.1;
	std::string config;
};


struct optParamSettings
{
	std::string name;
	double currentValue;
	// search limits
	double lowerLim;
	double upperLim;
	// important: this is the index within the MachineOptimizer class member vector of params, not within the original 
	// parametersystem vector of variables.
	UINT index = 0;
	double increment = 0.1;
	double limitSizeRange ( )
	{
		return upperLim - lowerLim;
	}
	dataPoint bestResult;
	std::vector<dataPoint> resultHist;
};


struct optDataPoint
{
	std::vector<double> paramValues;
	double value;
	double yerr;
};


struct HillClimbingInfo
{
	UINT loops = 0;
	std::shared_ptr<optParamSettings> currParam;
	int scanDir = 0;
	optDataPoint bestSetting;
	std::vector<optDataPoint> optimizationHistory;
};


class MachineOptimizer
{
	public:
		void initialize ( POINT& pos, cToolTips& toolTips, CWnd* parent, int& id);
		void handleOpenConfig ( );
		void handleSaveConfig ( );
		void handleNewConfig ( );
		void reset ( );
		void verifyOptInput ( AllExperimentInput input );
		void rearrange ( UINT width, UINT height, fontMap fonts );
		void updateParams( AllExperimentInput input, dataPoint resultValue, DataLogger& logger );
		void hillClimbingUpdate ( AllExperimentInput input, dataPoint resultValue, DataLogger& logger);
		void updateCurrentValueDisplays ( );
		void handleListViewClick ( );
		void deleteParam ( );
		void onFinOpt ( );
		void updateBestResult ( std::string val, std::string err );
		//void updateBestValueListview ( UINT item, double bestVal );
		void MachineOptimizer::updateBestValueDisplays ( );
		std::vector<std::shared_ptr<optParamSettings>> getOptParams ( );
		UINT getMaxRoundNum ( );
		bool isInMiddleOfOptimizing ( );
		void updateCurrRoundDisplay ( std::string roundtxt );
	private:
		// controls
		Control<CStatic> header;
		Control<CButton> optimizeButton;
		// optimization settings
		Control<CStatic> algorithmsHeader;
		Control<CStatic> optParamsHeader;
		Control<CStatic> bestResultTxt;
		Control<CStatic> bestResultVal;
		Control<CStatic> bestResultErr;

		Control<CStatic> maxRoundsTxt;
		Control<CEdit> maxRoundsEdit;
		Control<CStatic> currRoundTxt;
		Control<CStatic> currRoundDisp;


		Control<MyListCtrl> optParamsListview;
		std::array<Control<CButton>, 4> algorithmRadios;
		bool isOptimizing = false;
		UINT optCount = 0;
		UINT roundCount = 0;

		optimizationSettings currentSettings;
		std::vector<std::shared_ptr<optParamSettings>> optParams;
		HillClimbingInfo optStatus;
};

