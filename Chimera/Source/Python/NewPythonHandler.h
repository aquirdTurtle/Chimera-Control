#pragma once
#include <string>
#include <GeneralObjects/coordinate.h>
#include <Piezo/piezoChan.h>
#include <RealTimeDataAnalysis/atomGrid.h>

// this class is just for conveniently grouping any python handling together. 
class NewPythonHandler {
	public:
		void runDataAnalysis (std::string date, long runNumber, long accumulations,
							  std::vector<coordinate> atomLocations);
		double runCarrierAnalysis (std::string date, long runNumber, atomGrid gridInfo, QWidget* parent);
};