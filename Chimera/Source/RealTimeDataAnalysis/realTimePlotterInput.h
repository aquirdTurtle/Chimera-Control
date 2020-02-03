// created by Mark O. Brown
#pragma once
#include "atomGrid.h"
#include "GeneralImaging/imageParameters.h"
#include "ExperimentThread/Communicator.h"
#include "Plotting/tinyPlotInfo.h"
#include <atomic>
#include <vector>
#include <mutex>


struct realTimePlotterInput
{
	realTimePlotterInput ( std::atomic<UINT>& pltTime ) : plotTime ( pltTime ) { }
	std::vector<Gdiplus::Pen*> plotPens;
	CFont* plotFont;
	std::vector<Gdiplus::SolidBrush*> plotBrushes;
	std::atomic<UINT>& plotTime;
	AndorCameraSettings cameraSettings;
	CWnd* plotParentWindow;

	std::vector<tinyPlotInfo> plotInfo;
	std::vector<coordinate> analysisLocations;
	std::vector<atomGrid> grids;
	//atomGrid atomGridInfo;
	// This gets set to false, e.g. when the experiment ends, but doesn't tell the plotter to immediately stop, the 
	// plotter can finish it's work if it's backed up on images.
	std::atomic<bool>* active;
	// this tells the plotter to immediately stop.
	std::atomic<bool>* aborting;
	multiGridImageQueue* imQueue;
	multiGridAtomQueue* atomQueue;
	imageParameters imageShape;

	UINT picsPerVariation;
	UINT picsPerRep;
	UINT variations;

	UINT alertThreshold;
	bool wantAtomAlerts;
	Communicator* comm;
	std::mutex* plotLock;

	UINT plottingFrequency;
	UINT numberOfRunsToAverage;
	std::vector<double> key;
	// first level is for which plot. second level is which line in a plot. third level is which point in a line.
	std::vector<std::vector<std::shared_ptr<std::vector<dataPoint>>>> dataArrays;

	bool needsCounts;
};


