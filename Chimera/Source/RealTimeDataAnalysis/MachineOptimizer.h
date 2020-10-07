// created by Mark O. Brown
#pragma once
#include "Control.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include "ExperimentThread/AllExperimentInput.h"
#include "Plotting/dataPoint.h"
#include <array>
#include <memory>
#include "DataLogging/DataLogger.h"
#include <qlabel.h>
#include "PrimaryWindows/IChimeraQtWindow.h"
#include <qlineedit.h>
#include <qtablewidget.h>

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
	unsigned index = 0;
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
	unsigned loops = 0;
	std::shared_ptr<optParamSettings> currParam;
	int scanDir = 0;
	optDataPoint bestSetting;
	std::vector<optDataPoint> optimizationHistory;
};


class MachineOptimizer
{
	public:
		void initialize ( QPoint& pos, IChimeraQtWindow* parent );
		void reset ( );
		void verifyOptInput ( AllExperimentInput& input );
		void hillClimbingUpdate ( AllExperimentInput& input, dataPoint resultValue, DataLogger& logger);
		void updateCurrentValueDisplays ( );
		void handleListViewClick ( );
		void deleteParam ( );
		void onFinOpt ( );
		void handleContextMenu (const QPoint& pos);
		void updateBestResult ( std::string val, std::string err );
		//void updateBestValueListview ( unsigned item, double bestVal );
		void MachineOptimizer::updateBestValueDisplays ( );
		std::vector<std::shared_ptr<optParamSettings>> getOptParams ( );
		unsigned getMaxRoundNum ( );
		bool isInMiddleOfOptimizing ( );
		void updateCurrRoundDisplay ( std::string roundtxt );
	private:
		// controls
		QLabel* header;
		QPushButton* optimizeButton;
		// optimization settings
		QLabel* algorithmsHeader;
		QLabel* optParamsHeader;
		QLabel* bestResultTxt;
		QLabel* bestResultVal;
		QLabel* bestResultErr;

		QLabel* maxRoundsTxt;
		QLineEdit* maxRoundsEdit;
		QLabel* currRoundTxt;
		QLabel* currRoundDisp;

		QTableWidget* optParamsListview;
		std::array<QPushButton*, 4> algorithmRadios;
		bool isOptimizing = false;
		unsigned optCount = 0;
		unsigned roundCount = 0;

		optimizationSettings currentSettings;
		std::vector<std::shared_ptr<optParamSettings>> optParams;
		HillClimbingInfo optStatus;
};

