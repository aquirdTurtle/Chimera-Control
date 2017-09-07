#pragma once
#include "atomGrid.h"
#include "imageParameters.h"
#include "Communicator.h"
#include "gnuplotter.h"
#include "tinyPlotInfo.h"
#include <atomic>
#include <vector>
#include <mutex>

struct realTimePlotterInput
{
	std::vector<tinyPlotInfo> plotInfo;
	std::vector<coordinate> analysisLocations;
	atomGrid atomGridInfo;
	std::atomic<bool>* active;
	std::vector<std::vector<long>>* imageQueue;
	std::vector<std::vector<bool>>* atomQueue;
	imageParameters imageShape;
	UINT picsPerVariation;
	UINT picsPerRep;
	UINT variations;

	UINT alertThreshold;
	bool wantAlerts;
	Communicator* comm;
	std::mutex* plotLock;

	UINT plottingFrequency;
	UINT numberOfRunsToAverage;
	std::vector<double> key;
	Gnuplotter* plotter;

	bool needsCounts;
};


