#include "stdafx.h"
#include "AnalysisThreadWorker.h"
#include "DataAnalysisControl.h"
#include <vector>

AnalysisThreadWorker::AnalysisThreadWorker (realTimePlotterInput* input_){
	input = input_;
}

AnalysisThreadWorker::~AnalysisThreadWorker () {
}

void AnalysisThreadWorker::handleNewPic (atomQueue atomPics) {
	/// Start loop waiting for plots
	// if no image, continue. 0th element is queue, 2nd element is grid num, always akot least 1 grid.
	/// get all the atom data
	bool thereIsAtLeastOneAtom = false;
	for (auto gridCount : range (input->grids.size ())) {
		unsigned groupNum = input->grids[gridCount].height * input->grids[gridCount].width;
		for (auto pixelInc : range (groupNum)) {
			// look at the most recent image.
			if (atomPics[gridCount].image[pixelInc]) {
				thereIsAtLeastOneAtom = true;
				atomPresentData[gridCount][pixelInc].push_back (1);
			}
			else {
				atomPresentData[gridCount][pixelInc].push_back (0);
			}
		}
	}
	if (thereIsAtLeastOneAtom) {
		noAtomsCounter = 0;
	}
	else {
		noAtomsCounter++;
	}
	if (noAtomsCounter >= input->alertThreshold && input->wantAtomAlerts) {
		//input->comm->sendNoAtomsAlert ();
	}
	/// check if have enough data to plot
	auto picNum = atomPics[0].picNum;
	if (picNum % allPlots[0].getPicNumber () != 0) {
		// In this case, not enough data to plot a point yet, but I've just analyzed a pic, so remove that pic.
		// currentThreadPictureNumber++;
		// wait for next picture.
		return;
	}
	unsigned variationNum = (picNum - 1) / (input->picsPerVariation);
	plotNumberCount++;
	for (auto plotI : range (allPlots.size ())) {
		/// Check Post-Selection Conditions
		unsigned whichGrid = input->plotInfo[plotI].whichGrid;
		unsigned groupNum = input->grids[whichGrid].height * input->grids[whichGrid].width;
		std::vector<std::vector<bool> > satisfiesPsc (allPlots[plotI].getDataSetNumber (),
			std::vector<bool> (groupNum, true));
		DataAnalysisControl::determineWhichPscsSatisfied (allPlots[plotI], groupNum, atomPresentData[whichGrid], satisfiesPsc);
		// split into one of two big subroutines. The handling here is encapsulated into functions mostly just for 
		// organization purposes.
		if (allPlots[plotI].getPlotType () == "Atoms") {
			auto res = DataAnalysisControl::handlePlotAtoms (
				allPlots[plotI], picNum, finalDataNew[plotI], dataContainers[plotI],
				variationNum, satisfiesPsc, plotNumberCount, atomPresentData[whichGrid], input->plottingFrequency,
				groupNum, input->picsPerVariation);
			if (res.size () != 0) {
				emit newPlotData (res, plotI);
			}
		}
		else if (allPlots[plotI].getPlotType () == "Pixel Counts") {
			// TODO: Reimplement this here.
		}
		else if (allPlots[plotI].getPlotType () == "Pixel Count Histograms") {
			// This is done in the different slot. Should review this chunck of code to make this more efficient and 
			// not do anything 
			//DataAnalysisControl::handlePlotHist (allPlots[plotI], countData[whichGrid], finalHistData[plotI],
			//	satisfiesPsc, histogramData[plotI], dataContainers[plotI], groupNum);
		}
	}
	/// clear data
	// all pixels being recorded, not pixels in a data set.
	for (auto gridCount : range (input->grids.size ())) {
		unsigned groupNum = input->grids[gridCount].height * input->grids[gridCount].width;
		for (auto pixelI : range (groupNum)) {
			//countData[gridCount][pixelI].clear ();
			atomPresentData[gridCount][pixelI].clear ();
		}
	}
}

void AnalysisThreadWorker::handleNewPix (PixListQueue pixlist) {
	/// Start loop waiting for plots
	// if no image, continue. 0th element is queue, 2nd element is grid num, always akot least 1 grid.
	/// get all the atom data
	if (!input->needsCounts) {
		return;
	}
	PixListQueue tempPixList( input->grids.size( ) );
	tempPixList = pixlist;
	/// for all pixels... gather count information
	for (auto gridCount : range (input->grids.size ())) {
		unsigned locIndex = 0;
		for (auto row : range (input->grids[gridCount].width)) {
			for (auto column : range (input->grids[gridCount].height)) {
				countData[gridCount][locIndex].push_back (tempPixList[gridCount].image[locIndex]);
				locIndex++;
			}
		}
	}
	int totalNumberOfPixels = 0;
	int numberOfLossDataPixels = 0;	
	/// figure out which pixels need any data
	for (auto plotInc : range(allPlots.size())){
		for (auto pixelInc : range(allPlots[plotInc].getPixelNumber())){
			unsigned whichGrid = input->plotInfo[plotInc].whichGrid;
			unsigned groupNum = input->grids[whichGrid].height * input->grids[whichGrid].width;
			for (auto groupInc : range(groupNum)){
				bool alreadyExists = false;
			}
		}
	}
	/// check if have enough data to plot
	auto picNum = tempPixList[0].picNum;
	if (picNum % allPlots[0].getPicNumber () != 0) {
		// In this case, not enough data to plot a point yet, but I've just analyzed a pic, so remove that pic.
		// wait for next picture.
		return;
	}
	unsigned variationNum = (picNum - 1) / (input->picsPerVariation);
	plotNumberCount++;
	for (auto plotI : range (allPlots.size ())) {
		/// Check Post-Selection Conditions
		unsigned whichGrid = input->plotInfo[plotI].whichGrid;
		unsigned groupNum = input->grids[whichGrid].height * input->grids[whichGrid].width;
		std::vector<std::vector<bool> > satisfiesPsc (allPlots[plotI].getDataSetNumber (),
			std::vector<bool> (groupNum, true));

		//DataAnalysisControl::determineWhichPscsSatisfied (allPlots[plotI], groupNum, atomPresentData[whichGrid], satisfiesPsc);
		// split into one of two big subroutines. The handling here is encapsulated into functions mostly just for 
		// organization purposes.
		if (allPlots[plotI].getPlotType () == "Pixel Count Histograms") {
			// This is done in the different slot. Should review this chunck of code to make this more efficient and 
			// not do anything 
			auto res = DataAnalysisControl::handlePlotHist (allPlots[plotI], countData[whichGrid], finalHistData[plotI],
												 satisfiesPsc, histogramData[plotI], dataContainers[plotI], groupNum);
			if (res.size () != 0) {
				emit newPlotData (res, plotI);
			}
		}
	}
	/// clear data
	// all pixels being recorded, not pixels in a data set.
	for (auto gridCount : range (input->grids.size ())) {
		unsigned groupNum = input->grids[gridCount].height * input->grids[gridCount].width;
		for (auto pixelI : range (groupNum)) {
			countData[gridCount][pixelI].clear ();
		}
	}
}

void AnalysisThreadWorker::init (){
	input->worker = this;
	// make vector of plot information classes.
	/// open files
	for (auto plotInc : range (input->plotInfo.size ())) {
		std::string tempFileName = PLOT_FILES_SAVE_LOCATION + "\\" + input->plotInfo[plotInc].name + "."
			+ PLOTTING_EXTENSION;
		allPlots.push_back (PlottingInfo::PlottingInfo (tempFileName));
		allPlots[plotInc].setGroups (std::vector<coordinate>());
	}
	if (allPlots.size () == 0) {
		// no plots to run so just quit.
		return;
	}
	/// check pictures per experiment
	for (auto plotInc : range (allPlots.size ())) {
		if (allPlots[0].getPicNumber () != allPlots[plotInc].getPicNumber ()) {
			//errBox ("ERROR: Number of pictures per experiment don't match between plots. The plotting thread will "
			//	"close.");
			return;
		}
	}
	int totalNumberOfPixels = 0;
	int numberOfLossDataPixels = 0;
	/// figure out which pixels need any data
	for (auto plotInc : range (allPlots.size ())) {
		for (auto pixelInc : range (allPlots[plotInc].getPixelNumber ())) {
			unsigned whichGrid = input->plotInfo[plotInc].whichGrid;
			unsigned groupNum = input->grids[whichGrid].height * input->grids[whichGrid].width;
			for (auto groupInc : range (groupNum)) {
				bool alreadyExists = false;
			}
		}
	}
	// 
	if (totalNumberOfPixels == 0) {
		// no locations selected for analysis; quit.
		// return 0; ??? why is this commented out?
	}
	/// Initialize Arrays for data. (using std::vector above)
	// thinking about making these experiment-picture sized and resetting after getting the needed data out of them.

	countData.resize(input->grids.size ());
	atomPresentData.resize(input->grids.size ());
	for (auto gridCount : range (input->grids.size ())) {
		unsigned groupNum = input->grids[gridCount].height * input->grids[gridCount].width;
		countData[gridCount] = std::vector<std::vector<long>> (groupNum);
		atomPresentData[gridCount] = std::vector<std::vector<int>> (groupNum);
	}
	finalHistData.resize (allPlots.size ());
	histogramData.resize (allPlots.size ());
	finalCountData.resize(allPlots.size ());
	dataContainers.resize (allPlots.size ());
	finalDataNew.resize(allPlots.size ());
	finalAvgs.resize (allPlots.size ()); 
	finalErrorBars.resize (allPlots.size ());
	finalXVals .resize(allPlots.size ());
	for (auto plotInc : range (allPlots.size ())) {
		unsigned datasetNumber = allPlots[plotInc].getDataSetNumber ();
		histogramData[plotInc].resize (datasetNumber);
		finalCountData[plotInc].resize (datasetNumber);
		finalDataNew[plotInc].resize (datasetNumber);
		finalAvgs[plotInc].resize (datasetNumber);
		finalErrorBars[plotInc].resize (datasetNumber);
		finalXVals[plotInc].resize (datasetNumber);
		for (auto dataSetInc : range (allPlots[plotInc].getDataSetNumber ())) {
			unsigned whichGrid = input->plotInfo[plotInc].whichGrid;
			unsigned groupNum = input->grids[whichGrid].height * input->grids[whichGrid].width;
			histogramData[plotInc][dataSetInc].resize (groupNum);
			finalCountData[plotInc][dataSetInc].resize (groupNum);
			finalAvgs[plotInc][dataSetInc].resize (groupNum);
			finalDataNew[plotInc][dataSetInc].resize (groupNum);
			finalErrorBars[plotInc][dataSetInc].resize (groupNum);
			finalXVals[plotInc][dataSetInc].resize (groupNum);
		}
	}
}

void AnalysisThreadWorker::setXpts (std::vector<double> newXpts) {
	for (auto plotInc : range(dataContainers.size())) {
		auto& plotInfo = allPlots[plotInc];
		dataContainers[plotInc].resize (input->grids[0].numAtoms());
		for (auto& dset : dataContainers[plotInc]) {
			dset.resize (newXpts.size ());
			for (auto dpNum : range(dset.size())) {
				dset[dpNum].x = newXpts[dpNum];
			}
		}
	}
	xvals = newXpts;
}
