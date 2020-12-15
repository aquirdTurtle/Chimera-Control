#pragma once
#include <string>
#include <GeneralObjects/coordinate.h>
#include <RealTimeDataAnalysis/atomGrid.h>
#include <AnalogOutput/calInfo.h>
// this class is just for conveniently grouping any python handling together. 
class NewPythonHandler {
	public:
		void runDataAnalysis (std::string date, long runNumber, long accumulations,
							  std::vector<coordinate> atomLocations);
		double runCarrierAnalysis (std::string date, long runNumber, atomGrid gridInfo, QWidget* parent);
		std::vector<double> runCalibrationFits (calSettings cal, QWidget* parent);
};