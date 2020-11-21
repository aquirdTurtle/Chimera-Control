#pragma once

#include <RealTimeDataAnalysis/atomGrid.h>
#include <Plotting/tinyPlotInfo.h>
#include <vector>
#include <string>

struct analysisSettings {
	bool autoThresholdAnalysisOption = false;
	bool displayGridOption=false;
	std::vector<atomGrid> grids;
	std::string bumpParam = "";
	bool autoBumpOption = false;
	std::vector<std::string> activePlotNames;
	std::vector<unsigned> whichGrids;
};

Q_DECLARE_METATYPE (analysisSettings)
