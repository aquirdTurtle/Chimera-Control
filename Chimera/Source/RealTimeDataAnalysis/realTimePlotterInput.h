// created by Mark O. Brown
#pragma once
#include "atomGrid.h"
#include "GeneralImaging/imageParameters.h"
#include "Plotting/tinyPlotInfo.h"
#include <GeneralObjects/Queues.h>
#include <Plotting/dataPoint.h>
#include <atomic>
#include <vector>
#include <mutex>
#include <Andor/AndorRunSettings.h>

class IChimeraQtWindow;
class AnalysisThreadWorker;

struct realTimePlotterInput{
	realTimePlotterInput ( std::atomic<unsigned>& pltTime ) : plotTime ( pltTime ) { }
	//AnalysisThreadWorker* worker;
	std::atomic<unsigned>& plotTime;
	//AndorCameraSettings cameraSettings;
	IChimeraQtWindow* plotParentWindow;

	std::vector<tinyPlotInfo> plotInfo;
	std::vector<atomGrid> grids;
	// This gets set to false, e.g. when the experiment ends, but doesn't tell the plotter to immediately stop, the 
	// plotter can finish it's work if it's backed up on images.
	std::atomic<bool>* active;
	// this tells the plotter to immediately stop.
	std::atomic<bool>* aborting;
	imageParameters imageShape;

	unsigned picsPerVariation;
	unsigned picsPerRep;
	unsigned variations;

	unsigned alertThreshold;
	bool wantAtomAlerts;

	unsigned plottingFrequency;
	unsigned numberOfRunsToAverage;
	bool needsCounts;
};


