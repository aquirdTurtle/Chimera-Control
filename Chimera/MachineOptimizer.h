#pragma once
#include "Control.h"
#include "MasterThreadInput.h"
#include "MyListCtrl.h"
#include "dataPoint.h"
#include <array>
#include <memory>

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
	double gain = 0.1;
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
	double limitSizeRange ( )
	{
		return upperLim - lowerLim;
	}
	std::vector<double> valueHist;
	std::vector<dataPoint> resultHist;
};


struct HillClimbingInfo
{
	UINT loops = 0;
	std::shared_ptr<optParamSettings> currParam;
	int scanDir = 0;
};


class MachineOptimizer
{
	public:
		void initialize ( POINT& pos, cToolTips& toolTips, CWnd* parent, int& id);
		void handleOpenConfig ( );
		void handleSaveConfig ( );
		void handleNewConfig ( );
		void reset ( );
		void verifyOptInput ( ExperimentInput input );
		void rearrange ( UINT width, UINT height, fontMap fonts );
		void updateParams( ExperimentInput input, dataPoint resultValue );
		void hillClimbingUpdate ( ExperimentInput input, dataPoint resultValue);
		void handleListViewClick ( );
		void deleteParam ( );
		std::vector<std::shared_ptr<optParamSettings>> getOptParams ( );
	private:
		// controls
		Control<CStatic> header;
		Control<CButton> optimizeButton;
		// optimization settings
		Control<CStatic> algorithmsHeader;
		Control<CStatic> optParamsHeader;
		Control<MyListCtrl> optParamsListview;
		std::array<Control<CButton>, 4> algorithmRadios;
		std::vector<std::vector<double>> paramHistory;
		std::vector<std::vector<dataPoint>> measuredResultHistory;
		bool isOptimizing = false;
		UINT optCount = 0;
		optimizationSettings currentSettings;
		std::vector<std::shared_ptr<optParamSettings>> optParams;
		HillClimbingInfo optStatus;
};