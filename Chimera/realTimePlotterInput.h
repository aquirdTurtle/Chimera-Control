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
	std::vector<atomGrid> grids;
	//atomGrid atomGridInfo;
	// This gets set to false, e.g. when the experiment ends, but doesn't tell the plotter to immediately stop, the 
	// plotter can finish it's work if it's backed up on images.
	std::atomic<bool>* active;
	// this tells the plotter to immediately stop.
	std::atomic<bool>* aborting;
	std::vector<std::vector<std::vector<long>>>* imageQueue;
	std::vector<std::vector<std::vector<bool>>>* atomQueue;
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
	//Gnuplotter* plotter;
	// first level is for which plot. second level is which line in a plot. third level is which point in a line.
	std::vector<std::vector<std::shared_ptr<std::vector<dataPoint>>>> dataArrays;

	bool needsCounts;
};


