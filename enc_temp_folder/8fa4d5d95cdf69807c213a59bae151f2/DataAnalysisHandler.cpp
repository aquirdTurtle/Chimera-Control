
#include "stdafx.h"
#include <numeric>
#include <boost/tuple/tuple.hpp>

#include "DataAnalysisHandler.h"
#include "Control.h"
#include "CameraWindow.h"
#include "ProfileSystem.h"
#include "PlotDesignerDialog.h"


void DataAnalysisControl::initialize( cameraPositions& pos, int& id, CWnd* parent, cToolTips& tooltips, 
									  int isTriggerModeSensitive, rgbMap rgbs )
{
	header.seriesPos = { pos.seriesPos.x,  pos.seriesPos.y,  pos.seriesPos.x + 480, pos.seriesPos.y + 25 };
	header.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y + 25 };
	header.amPos = { pos.amPos.x,   pos.amPos.y,   pos.amPos.x + 480, pos.amPos.y + 25 };
	header.triggerModeSensitive = isTriggerModeSensitive;
	header.Create("DATA ANALYSIS", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER, header.seriesPos, parent, id++);
	header.fontType = HeadingFont;
	pos.seriesPos.y += 25;
	pos.videoPos.y += 25;
	pos.amPos.y += 25;

	/// Data analysis stuffs.
	//
	currentDataSetNumberText.seriesPos = { pos.seriesPos.x, pos.seriesPos.y,
		pos.seriesPos.x + 400, pos.seriesPos.y + 25 };
	currentDataSetNumberText.amPos = { pos.amPos.x, pos.amPos.y,
		pos.amPos.x + 400, pos.amPos.y + 25 };
	currentDataSetNumberText.videoPos = { -1,-1,-1,-1 };
	currentDataSetNumberText.triggerModeSensitive = isTriggerModeSensitive;
	currentDataSetNumberText.Create("Most Recent Data Set #:", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
									currentDataSetNumberText.seriesPos, parent, id++);
	//
	currentDataSetNumberEdit.seriesPos = { pos.seriesPos.x + 400, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y + 25 };
	currentDataSetNumberEdit.amPos = { pos.amPos.x + 400, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y + 25 };
	currentDataSetNumberEdit.videoPos = { -1,-1,-1,-1 };
	currentDataSetNumberEdit.triggerModeSensitive = isTriggerModeSensitive;
	currentDataSetNumberEdit.Create("?", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
									currentDataSetNumberEdit.seriesPos, parent, id++);
	pos.seriesPos.y += 25;
	pos.amPos.y += 25;

	// 
	setAnalysisLocationsButton.seriesPos = { pos.seriesPos.x, pos.seriesPos.y,
		pos.seriesPos.x + 480, pos.seriesPos.y + 25 };
	setAnalysisLocationsButton.amPos = { pos.amPos.x, pos.amPos.y,
		pos.amPos.x + 480, pos.amPos.y + 25 };
	setAnalysisLocationsButton.videoPos = { -1,-1,-1,-1 };
	setAnalysisLocationsButton.triggerModeSensitive = isTriggerModeSensitive;
	setAnalysisLocationsButton.Create("Set AutoAnalysis Points", WS_CHILD | WS_VISIBLE | BS_PUSHLIKE | BS_CHECKBOX,
									  setAnalysisLocationsButton.seriesPos, parent, IDC_SET_ANALYSIS_LOCATIONS );
	pos.seriesPos.y += 25;
	pos.amPos.y += 25;

	/// PLOTTING FREQUENCY CONTROLS
	// Set Plotting Frequency
	updateFrequencyLabel1.seriesPos = { pos.seriesPos.x,  pos.seriesPos.y,  pos.seriesPos.x + 150, pos.seriesPos.y + 25 };
	updateFrequencyLabel1.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 150, pos.videoPos.y + 25 };
	updateFrequencyLabel1.amPos = { pos.amPos.x,   pos.amPos.y,   pos.amPos.x + 150, pos.amPos.y + 25 };
	updateFrequencyLabel1.triggerModeSensitive = isTriggerModeSensitive;

	updateFrequencyLabel1.Create("Update plots every (", WS_CHILD | WS_VISIBLE | WS_BORDER,
								 updateFrequencyLabel1.seriesPos, parent, id++);
	// Plotting Frequency Edit
	updateFrequencyEdit.seriesPos = { pos.seriesPos.x + 150, pos.seriesPos.y,pos.seriesPos.x + 200, pos.seriesPos.y + 25 };
	updateFrequencyEdit.videoPos = { pos.videoPos.x + 150, pos.videoPos.y, pos.videoPos.x + 200, pos.videoPos.y + 25 };
	updateFrequencyEdit.amPos = { pos.amPos.x + 150, pos.amPos.y, pos.amPos.x + 200, pos.amPos.y + 25 };
	updateFrequencyEdit.triggerModeSensitive = isTriggerModeSensitive;
	updateFrequencyEdit.Create( WS_CHILD | WS_VISIBLE | WS_BORDER, updateFrequencyEdit.seriesPos, parent, id++);
	updateFrequency = 5;
	updateFrequencyEdit.SetWindowTextA("5");
	// end of that statement
	updateFrequencyLabel2.seriesPos = { pos.seriesPos.x + 200,  pos.seriesPos.y,  pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	updateFrequencyLabel2.videoPos = { pos.videoPos.x + 200, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y += 25 };
	updateFrequencyLabel2.amPos = { pos.amPos.x + 200,   pos.amPos.y,   pos.amPos.x + 480, pos.amPos.y += 25 };
	updateFrequencyLabel2.triggerModeSensitive = isTriggerModeSensitive;
	updateFrequencyLabel2.Create(") repetitions.", WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
								 updateFrequencyLabel2.seriesPos, parent, id++);
	/// the listview
	plotListview.seriesPos = { pos.seriesPos.x,   pos.seriesPos.y,  pos.seriesPos.x + 480,  pos.seriesPos.y += 250 };
	plotListview.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y += 250 };
	plotListview.amPos = { pos.amPos.x,     pos.amPos.y,   pos.amPos.x + 480,   pos.amPos.y += 250 };
	plotListview.triggerModeSensitive = isTriggerModeSensitive;
	plotListview.Create(WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_BORDER, plotListview.seriesPos, parent, 
						 IDC_PLOTTING_LISTVIEW );
	// initialize the listview
	LV_COLUMN listViewDefaultCollumn;
	// Zero Members
	memset(&listViewDefaultCollumn, 0, sizeof(listViewDefaultCollumn));
	listViewDefaultCollumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	listViewDefaultCollumn.pszText = "Name";
	// width between each coloum
	listViewDefaultCollumn.cx = 0x82;
	// Inserting Collumbs as much as we want
	plotListview.InsertColumn(0, &listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "Active?";
	plotListview.InsertColumn(1, &listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "View Details?";
	plotListview.InsertColumn(1, &listViewDefaultCollumn);
	listViewDefaultCollumn.pszText = "Edit?";
	plotListview.InsertColumn(1, &listViewDefaultCollumn);

	plotListview.SetBkColor( rgbs["Solarized Base02"]);
	plotListview.SetTextBkColor( rgbs["Solarized Base02"] );
	plotListview.SetTextColor( rgbs["Solarized Green"] );
	//
	reloadListView();
}


unsigned __stdcall DataAnalysisControl::plotterProcedure(void* voidInput)
{
	realTimePlotterInput* input = (realTimePlotterInput*)voidInput;
	// Register any windows messages for the main window
	//ePlottingIsSlowMessage = RegisterWindowMessage("ID_PLOTTING_IS_SLOW");
	//ePlottingCaughtUpMessage = RegisterWindowMessage("ID_PLOTTING_CAUGHT_UP");
	// make vector of plot information classes. 
	
	std::vector<PlottingInfo> allPlots;
	/// open files
	for (UINT plotInc = 0; plotInc < input->plotInfo.size(); plotInc++)
	{
		std::string tempFileName = PLOT_FILES_SAVE_LOCATION + "\\" + input->plotInfo[plotInc].name + PLOTTING_EXTENSION;
		allPlots.push_back(PlottingInfo::PlottingInfo(tempFileName));
		allPlots[plotInc].setGroups(input->analysisLocations);
	}

	// run some checks...
	if (allPlots.size() == 0)
	{
		// no plots to run so just quit.
		return 0;
	}
	/// check pictures per experiment
	for (UINT plotInc = 0; plotInc < allPlots.size(); plotInc++)
	{
		if (allPlots[0].getPicNumber() != allPlots[plotInc].getPicNumber())
		{
			errBox("ERROR: Number of pictures per experiment don't match between plots.");
			return 0;
		}
	}
	// create vector size of image being taken
	// first entry is row, second is collumn, third is data type (0 = pixel counts only, 1 = atom presense)
	std::vector<std::array<UINT, 3> > pixelDataType;
	int totalNumberOfPixels = 0;
	int numberOfLossDataPixels = 0;
	
	/// figure out which pixels need any data
	for (UINT plotInc = 0; plotInc < allPlots.size(); plotInc++)
	{
		for (UINT pixelInc = 0; pixelInc < allPlots[plotInc].getPixelNumber(); pixelInc++)
		{
			for (UINT groupInc = 0; groupInc < allPlots[plotInc].getPixelGroupNumber(); groupInc++)
			{
				UINT row, collumn;
				bool alreadyExists = false;
				allPlots[plotInc].getPixelLocation(pixelInc, groupInc, row, collumn);
				for (UINT savedPixelInc = 0; savedPixelInc < pixelDataType.size(); savedPixelInc++)
				{
					// figure out if it already exists
					if (allPlots[plotInc].getPlotType() == "Atoms")
					{
						std::array<UINT, 3> testArray = { { row, collumn, 1 } };
						if (pixelDataType[savedPixelInc] == testArray)
						{
							alreadyExists = true;
							allPlots[plotInc].setPixelIndex(pixelInc, groupInc, savedPixelInc);
						}
					}
					else
					{
						std::array<UINT, 3> testArray1 = { { row, collumn, 1 } }, testArray2 = { { row, collumn, 0 } };
						if (pixelDataType[savedPixelInc] == testArray1 || pixelDataType[savedPixelInc] == testArray2)
						{
							alreadyExists = true;
							allPlots[plotInc].setPixelIndex(pixelInc, groupInc, savedPixelInc);
						}
					}
				}
				// if doesn't already exist, add it.
				if (!alreadyExists)
				{
					if (allPlots[plotInc].getPlotType() == "Atoms")
					{
						// in this case I need atom data
						pixelDataType.push_back({ row, collumn, 1 });
						allPlots[plotInc].setPixelIndex(pixelInc, groupInc, totalNumberOfPixels);
						totalNumberOfPixels++;
					}
					else
					{
						// in this case I need atom data
						pixelDataType.push_back({ row, collumn, 0 });
						allPlots[plotInc].setPixelIndex(pixelInc, groupInc, totalNumberOfPixels);
						totalNumberOfPixels++;
						numberOfLossDataPixels++;
					}
				}
			}
		}
	}
	// 
	if (totalNumberOfPixels == 0)
	{
		// no locations selected for analysis; quit.
		return 0;
	}
	// set flag?

	/// Initialize Arrays for data.
	// thinking about making these experiment-picture sized and resetting after getting the needed data out of them.
	// pixelData[pixel Indicator][picture number indicator] = pixelCount;
	std::vector<std::vector<long>> countData;
	countData.resize(totalNumberOfPixels);
	// atomPresentData[pixelIndicator][picture number] = true if atom present, false if atom not present;
	std::vector<std::vector<int> > atomPresentData;
	atomPresentData.resize(totalNumberOfPixels);
	// finalData[plot][dataset][group][accumulationNumber];
	std::vector<std::vector<std::vector<std::vector<long> > > > finalData;
	// finalAvgs[plot][dataset][group][ExperimentNumber];
	std::vector<std::vector<std::vector<std::vector<double> > > > finalAvgs;
	// finalErrorBars[plot][dataset][group][ExperimentNumber];
	std::vector<std::vector<std::vector<std::vector<double> > > > finalErrorBars;
	// finalXVals[plot][dataset][group][ExperimentNumber];
	std::vector<std::vector<std::vector<std::vector<double> > > > finalXVals;
	// newData[plot][dataSet][group] = true if new data so change some vector sizes.
	std::vector<std::vector<std::vector<bool> > > newData;
	// size things up...
	finalData.resize(allPlots.size());
	finalAvgs.resize(allPlots.size());
	finalErrorBars.resize(allPlots.size());
	finalXVals.resize(allPlots.size());
	newData.resize(allPlots.size());
	// much sizing...
	for (UINT plotInc = 0; plotInc < allPlots.size(); plotInc++)
	{
		finalData[plotInc].resize(allPlots[plotInc].getDataSetNumber());
		finalAvgs[plotInc].resize(allPlots[plotInc].getDataSetNumber());
		finalErrorBars[plotInc].resize(allPlots[plotInc].getDataSetNumber());
		finalXVals[plotInc].resize(allPlots[plotInc].getDataSetNumber());
		newData[plotInc].resize(allPlots[plotInc].getDataSetNumber());
		for (UINT dataSetInc = 0; dataSetInc < allPlots[plotInc].getDataSetNumber(); dataSetInc++)
		{
			finalData[plotInc][dataSetInc].resize(allPlots[plotInc].getPixelGroupNumber());
			finalAvgs[plotInc][dataSetInc].resize(allPlots[plotInc].getPixelGroupNumber());
			finalErrorBars[plotInc][dataSetInc].resize(allPlots[plotInc].getPixelGroupNumber());
			finalXVals[plotInc][dataSetInc].resize(allPlots[plotInc].getPixelGroupNumber());
			newData[plotInc][dataSetInc].resize(allPlots[plotInc].getPixelGroupNumber());
			for (UINT groupInc = 0; groupInc < allPlots[plotInc].getPixelGroupNumber(); groupInc++)
			{
				newData[plotInc][dataSetInc][groupInc] = true;
			}
		}
	}

	UINT noAtomsCounter = 0;
	// this is used to keep track of the first time you start loosing atoms
	UINT atomCounterTotal = 0;
	UINT currentThreadRepNum = 1;
	UINT plotNumberCount = 0;
	// this effectively just keeps track of whether a "slow" message has been sent to the main window yet or not. Only want to send once.
	bool plotIsSlowStatus = false;

	/// /////////////////////////////////////////////
	/// Start loop waiting for plots
	/// /////////////////////////////////////////////
	while (*input->active || (input->atomQueue->size() > 0))
	{
		// if no image, continue.
		if (input->atomQueue->size() == 0 || input->atomQueue[0].size() == 0)
		{
			continue;
		}
		std::vector<long> tempImage;
		if ( input->needsCounts )
		{
			std::lock_guard<std::mutex> locker( *input->plotLock );
			if ( input->imageQueue->size( ) == 0 )
			{
				continue;
			}
			tempImage = input->imageQueue->front( );
			if ( tempImage.size( ) == 0 )
			{
				continue;
			}
		}
		if (input->atomQueue->size() > 2 && plotIsSlowStatus == false)
		{
			//PostMessage(eCameraWindowHandle, ePlottingIsSlowMessage, 0, 0);
			plotIsSlowStatus = true;
		}
		else if (input->atomQueue->size() == 1 && plotIsSlowStatus == true)
		{
			// the plotting has caught up, reset this.
			plotIsSlowStatus = false;
		}
		if (input->needsCounts)
		{
			/// for all pixels... gather count information
			std::lock_guard<std::mutex> locker( *input->plotLock );
			for (UINT pixelInc = 0; pixelInc < pixelDataType.size(); pixelInc++)
			{
				//int secondIndex = pixelDataType[pixelI][0] - 1 + input->imageShape.width * (pixelDataType[pixelI][1] - 1);
				int secondIndex = (input->imageShape.height - pixelDataType[pixelInc][0] - 1) * input->imageShape.width + pixelDataType[pixelInc][1] - 1;
				// first image, second index identifies the location.
				double data = tempImage[secondIndex];
				countData[pixelInc].push_back(data);
			}
		}

		/// get all the atom data
		bool isAtLeastOneAtom = false;
		// figure out what the threshold for current picture is.
		int experimentNumber = ((currentThreadRepNum - 1) % input->picsPerVariation) % input->picsPerRep;
		for (UINT pixelInc = 0; pixelInc < pixelDataType.size(); pixelInc++)
		{
			int secondIndex = (input->imageShape.height - pixelDataType[pixelInc][0]) * input->imageShape.width
				+ pixelDataType[pixelInc][1] - 1;
			bool val = input->atomQueue->at( 0 )[0];
			//errBox( val );
			if (input->atomQueue->at(0)[secondIndex]/*countData[pixelInc].back() > threshold*/)
			{
				// atom detected
				isAtLeastOneAtom = true;
				atomPresentData[pixelInc].push_back(1);
			}
			else
			{
				// no atom
				atomPresentData[pixelInc].push_back(0);
			}
		}
		if (!isAtLeastOneAtom)
		{
			noAtomsCounter++;
		}
		else
		{
			noAtomsCounter = 0;
		}

		// check if need to send an alert
		if (noAtomsCounter >= input->alertThreshold)
		{
			if (input->wantAlerts)
			{
				// comm message needed......
				// input->comm->send...
			}
		}

		/// check if have enough data to plot
		if (currentThreadRepNum % allPlots[0].getPicNumber() != 0)
		{
			// In this case, not enough data to plot a point yet, but I've just analyzed a pic, so remove that pic.
			std::lock_guard<std::mutex> locker(*input->plotLock);
			if ( input->needsCounts )
			{
				if ( input->imageQueue->size( ) > 0 )
				{
					// delete the first entry of the Queue which has just been handled.
					input->imageQueue->erase( input->imageQueue->begin( ) );

				}
			}
			if ( input->atomQueue->size( ) > 0 )
			{
				input->atomQueue->erase( input->atomQueue->begin( ) );
			}
			// increment the thread's accumulation Number.
			currentThreadRepNum++;
			continue;
		}
		// used to tell if time to plot given frequency.
		plotNumberCount++;
		// for every plot...
		for (UINT plotI = 0; plotI < allPlots.size(); plotI++)
		{
			/// DATA ANALYSIS
			/// Data Set Loop - need to kill.
			/// ///////////////////////////////
			/// Check Post-Selection Conditions
			// initialize this vector to all true. 
			// statisfiesPsc[dataSetI][groupI] = true or false
			std::vector<std::vector<bool> > statisfiesPsc;
			statisfiesPsc.resize(allPlots[plotI].getDataSetNumber());
			for (auto& dataSetStatus : statisfiesPsc)
			{
				dataSetStatus.resize(allPlots[plotI].getPixelGroupNumber());
				for (auto& groupStatus : dataSetStatus)
				{
					groupStatus = true;
				}
			}

			// check if actually true. there's got to be a better way to iterate through these guys...
			for (UINT dataSetI = 0; dataSetI < allPlots[plotI].getDataSetNumber(); dataSetI++)
			{
			  for (UINT groupI = 0; groupI < allPlots[plotI].getPixelGroupNumber(); groupI++)
			  {
				for (UINT conditionI = 0; conditionI < allPlots[plotI].getConditionNumber(); conditionI++)
				{
				  for (UINT pixelI = 0; pixelI < allPlots[plotI].getPixelNumber(); pixelI++)
				  {
					for (UINT picI = 0; picI < allPlots[plotI].getPicNumber(); picI++)
					{
					  // test if condition exists
					  int condition = allPlots[plotI].getPostSelectionCondition(dataSetI, conditionI, pixelI, picI);
					  if (condition != 0)
					  {
						if (condition == 1 && atomPresentData[allPlots[plotI].getPixelIndex(pixelI, groupI)][picI] != 1)
						{
							statisfiesPsc[dataSetI][groupI] = false;
						}
						else if (condition == -1 && atomPresentData[allPlots[plotI].getPixelIndex(pixelI, groupI)][picI] != 0)
						{
							statisfiesPsc[dataSetI][groupI] = false;
						}
					  }
					}
				  }
				}
			  }
			}

			if (allPlots[plotI].getPlotType() == "Atoms" || allPlots[plotI].getPlotType() == "Pixel Counts")
			{
				DataAnalysisControl::handlePlotAtomsOrCounts(input, allPlots[plotI], currentThreadRepNum, finalData[plotI],
															 finalAvgs[plotI], finalErrorBars[plotI], finalXVals[plotI],
															 newData[plotI], statisfiesPsc, plotI, countData, plotNumberCount,
															 atomPresentData);
			}
			else if (allPlots[plotI].getPlotType() == "Pixel Count Histograms")
			{
				DataAnalysisControl::handlePlotHist(input, allPlots[plotI], plotI, countData, finalData[plotI], 
													statisfiesPsc );
			}
		}
		// clear exp data
		// all pixels being recorded, not pixels in a data set.
		for (int pixelI = 0; pixelI < totalNumberOfPixels; pixelI++)
		{
			countData[pixelI].clear();
			atomPresentData[pixelI].clear();
		}
		// finally, remove the data from the queue.
		std::lock_guard<std::mutex> locker(*input->plotLock);
		// this should always be true if at this point...
		if (input->needsCounts)
		{
			// delete the first entry of the Queue which has just been handled.
			input->imageQueue->erase(input->imageQueue->begin());
		}
		input->atomQueue->erase(input->atomQueue->begin());
		// increment the thread's accumulation Number.
		currentThreadRepNum++;
	}
	return 0;
}


void DataAnalysisControl::handlePlotAtomsOrCounts( realTimePlotterInput* input, PlottingInfo plotInfo, UINT repNum,
												   std::vector<std::vector<std::vector<long> > >& finData,
												   std::vector<std::vector<std::vector<double> > >& finAvgs,
												   std::vector<std::vector<std::vector<double> > >& finErrs,
												   std::vector<std::vector<std::vector<double> > >& finX,
												   std::vector<std::vector<bool> >& needNewData, 
												   std::vector<std::vector<bool>>& pscSatisfied, bool plotNumber,
												   std::vector<std::vector<long>>& countData, int plotNumberCount,
												   std::vector<std::vector<int> > atomPresent )
{
	if (repNum % input->picsPerVariation == plotInfo.getPicNumber())
	{
		// first pic of new variation, so need to update x vals.
		for (UINT dataSetI = 0; dataSetI < plotInfo.getDataSetNumber(); dataSetI++)
		{
			// TODO: if x axis = average over experiments... else...
			finData[dataSetI].clear();
			finData[dataSetI].resize(plotInfo.getPixelGroupNumber());
			needNewData[dataSetI].resize(plotInfo.getPixelGroupNumber());
			std::fill(needNewData[dataSetI].begin(), needNewData[dataSetI].end(), true);
		}
	}
	/// Check Data Conditions
	if (plotInfo.getPlotType() == "Atoms")
	{
		for (UINT dataSetI = 0; dataSetI < plotInfo.getDataSetNumber(); dataSetI++)
		{
			for (UINT groupI = 0; groupI < plotInfo.getPixelGroupNumber(); groupI++)
			{
				if (pscSatisfied[dataSetI][groupI] == false)
				{
					// no new data.
					return;
				}
				bool dataVal = true;
				for (UINT pixelI = 0; pixelI < plotInfo.getPixelNumber(); pixelI++)
				{
					for (UINT picI = 0; picI < plotInfo.getPicNumber(); picI++)
					{
						// check if there is a condition at all
						int truthCondition = plotInfo.getResultCondition(dataSetI, pixelI, picI);
						if (truthCondition == 0)
						{
							continue;
						}
						int pixel = plotInfo.getPixelIndex(pixelI, groupI);
						if (truthCondition == 1 && atomPresent[pixel][picI] != 1)
						{
							dataVal = false;
						}
						else if (truthCondition == 0 && atomPresent[plotInfo.getPixelIndex(pixelI, groupI)][picI] != 0)
						{
							dataVal = false;
						}
					}
				}
				finData[dataSetI][groupI].push_back(dataVal);
				// then the size of the containers gets updated every time.
				if (plotInfo.getXAxis() == "Running Average")
				{
					needNewData[dataSetI][groupI] = true;
				}
			}
		}
	}
	else if (plotInfo.getPlotType() == "Pixel Counts")
	{
		for (UINT dataSetI = 0; dataSetI < plotInfo.getDataSetNumber(); dataSetI++)
		{
			for (UINT groupI = 0; groupI < plotInfo.getPixelGroupNumber(); groupI++)
			{
				if (pscSatisfied[dataSetI][groupI] == false)
				{
					continue;
				}
				UINT pixel, picture;
				// passing pixel and picture by reference.
				plotInfo.getDataCountsLocation(dataSetI, pixel, picture);
				// for a given group, figure out which picture
				finData[dataSetI][groupI].push_back(countData[plotInfo.getPixelIndex(pixel, groupI)][picture]);
			}
		}
	}

	/// Calculate averages and standard devations for Data sets AND groups...
	if (plotNumberCount % input->plottingFrequency != 0)
	{
		return;
	}
	for (UINT dataSetI = 0; dataSetI < plotInfo.getDataSetNumber(); dataSetI++)
	{
		for (UINT groupI = 0; groupI < plotInfo.getPixelGroupNumber(); groupI++)
		{
			// check if first picture of set
			if (repNum % input->plottingFrequency != 0)
			{
				continue;
			}
			if (needNewData[dataSetI][groupI] == true)
			{
				finAvgs[dataSetI][groupI].resize(finAvgs[dataSetI][groupI].size() + 1);
				// integer division here.
				if (plotInfo.getXAxis() == "Running Average")
				{
					finX[dataSetI][groupI].resize(finX[dataSetI][groupI].size() + 1);
					if (!(finData[dataSetI][groupI].size() >= input->numberOfRunsToAverage))
					{
						finX[dataSetI][groupI].back() = (std::accumulate( finX[dataSetI][groupI].begin(), 
																		  finX[dataSetI][groupI].end(), 0.0 )
														  + finData[dataSetI][groupI].size()) 
															/ finData[dataSetI][groupI].size();
					}
				}
				else
				{
					finErrs[dataSetI][groupI].resize(finErrs[dataSetI][groupI].size() + 1);
					finX[dataSetI][groupI].push_back(input->key[(repNum - 1) / input->picsPerVariation]);
				}
				// set the flag to not do this again before this array gets reset at beginning of the next accumulation stack.
				needNewData[dataSetI][groupI] = false;
			}
			// calculate new data points
			if (plotInfo.getXAxis() == "Running Average")
			{
				if (finData[dataSetI][groupI].size() >= input->numberOfRunsToAverage)
				{
					double sum = std::accumulate(finData[dataSetI][groupI].end() - input->numberOfRunsToAverage, finData[dataSetI][groupI].end(), 0.0);
					double mean = sum / input->numberOfRunsToAverage;
					finAvgs[dataSetI][groupI].back() = mean;
					finX[dataSetI][groupI].back() = (std::accumulate( finX[dataSetI][groupI].end() 
																	  - input->numberOfRunsToAverage + 1,
																	  finX[dataSetI][groupI].end(), 0.0 )
													  + finData[dataSetI][groupI].size()) / input->numberOfRunsToAverage;
					input->plotter->send("set xrange [" + str(finX[dataSetI][groupI][0] - 1) + ":"	
										 + str(finX[dataSetI][groupI].back() + 1) + "]");
				}
			}
			else
			{
				double sum = std::accumulate(finData[dataSetI][groupI].begin(), finData[dataSetI][groupI].end(), 0.0);
				double mean = sum / finData[dataSetI][groupI].size();
				double squaredSum = std::inner_product(finData[dataSetI][groupI].begin(), finData[dataSetI][groupI].end(),
													   finData[dataSetI][groupI].begin(), 0.0);
				double error = ((double)std::sqrt(squaredSum / finData[dataSetI][groupI].size() - mean * mean))
					/ std::sqrt(finData[dataSetI][groupI].size());
				finAvgs[dataSetI][groupI].back() = mean;
				finErrs[dataSetI][groupI].back() = error;
			}
			// 
		}
	}
	/// General Plotting Options
	input->plotter->send("set terminal wxt " + str(plotNumber) + " title \"" + plotInfo.getTitle() + "\" noraise background rgb 'black'");
	input->plotter->send("set format y \"%.1f\"");
	if (plotInfo.getPlotType() == "Atoms")
	{
		// set x range.
		double xRangeMin = *std::min_element(input->key.begin(), input->key.end());
		double xRangeMax = *std::max_element(input->key.begin(), input->key.end());
		double range = xRangeMax - xRangeMin;
		if ( range == 0 )
		{
			range++;
		}
		xRangeMin -= range / input->key.size();
		xRangeMax += range / input->key.size();
		input->plotter->send("set xrange [" + str(xRangeMin) + ":" + str(xRangeMax) + "]");
		input->plotter->send("set grid ytics lc rgb \"#bbbbbb\" lw 1 lt 0");
		input->plotter->send("set grid xtics lc rgb \"#bbbbbb\" lw 1 lt 0");
		input->plotter->send("set yrange [0:1]");
	}
	else
	{
		input->plotter->send("set autoscale y");
	}
	input->plotter->send("set title \"" + plotInfo.getTitle() + "\" tc rgb 'white'");
	input->plotter->send("set xlabel \"Key Value\" tc rgb 'white'");
	input->plotter->send("set ylabel \"" + plotInfo.getYLabel() + "\" tc rgb 'white'");
	input->plotter->send("set border lc rgb 'white'");
	input->plotter->send("set key tc rgb 'white'");
	/// FITTING
	for (UINT dataSetI = 0; dataSetI < plotInfo.getDataSetNumber(); dataSetI++)
	{
		if (plotInfo.whenToFit(dataSetI) == REAL_TIME_FIT 
			|| (plotInfo.whenToFit(dataSetI) == FIT_AT_END && repNum == input->picsPerVariation))
		{
			for (UINT groupInc = 0; groupInc < plotInfo.getPixelGroupNumber(); groupInc++)
			{
				std::string fitNum = str(plotInfo.getPixelGroupNumber() * dataSetI + groupInc);
				// in this case, fitting.
				switch (plotInfo.getFitOption(dataSetI))
				{
					// the to_string argument in each case is a unique number indicating the fit given the data set and group. I need
					// to keep track of each fit separately so that I can plot them all later. 
					case GAUSSIAN_FIT:
					{
						input->plotter->send("f" + fitNum + "(x) = A" + fitNum + " * exp(-(x - B" + fitNum + ")**2 / (2 * C" + fitNum + "))");
						input->plotter->send("A" + fitNum + " = 1");
						input->plotter->send("B" + fitNum + " = " + str(finX[dataSetI][groupInc].size() / 2.0));
						input->plotter->send("C" + fitNum + " = 1");
						input->plotter->send("fit f" + fitNum + "(x) '-' using 1:2 via A" + fitNum + ", B" + fitNum + ", C" + fitNum);
						break;
					}
					case LORENTZIAN_FIT:
					{
						input->plotter->send("f" + fitNum + "(x) = (A" + fitNum + " / (2 * 3.14159265359)) / ((x - B" + fitNum + ")**2 + (A" + fitNum + " / 2)**2)");
						input->plotter->send("A" + fitNum + " = 1");
						input->plotter->send("B" + fitNum + " = " + str(finX[dataSetI][groupInc].size() / 2.0));
						input->plotter->send("fit f" + fitNum + "(x) '-' using 1:2 via A" + fitNum + ", B" + fitNum);
						break;
					}
					case SINE_FIT:
					{
						input->plotter->send("f" + fitNum + "(x) = A" + fitNum + " * sin(B" + fitNum + " * x + C" + fitNum + ") * exp( - D" + fitNum + " * x)");
						input->plotter->send("A" + fitNum + " = 1");
						input->plotter->send("B" + fitNum + " = 1");
						input->plotter->send("C" + fitNum + " = 1");
						input->plotter->send("D" + fitNum + " = 1");
						input->plotter->send("fit f" + fitNum + "(x) '-' using 1:2 via A" + fitNum + ", B" + fitNum + ", C" + fitNum + ", D" + fitNum);
						break;
					}
					default:
					{
						errBox("Coding Error: Bad Fit option!");
					}
					input->plotter->sendData(finX[dataSetI][groupInc], finAvgs[dataSetI][groupInc]);
				}
			}
		}
	}
	/// SEND PLOT COMMANDS AND DATA
	/// send plot commands
	std::string gnuplotPlotCmd = "plot";
	if (plotInfo.getXAxis() == "Running Average")
	{
		for (UINT dataSetI = 0; dataSetI < plotInfo.getDataSetNumber(); dataSetI++)
		{
			for (UINT groupI = 0; groupI < plotInfo.getPixelGroupNumber(); groupI++)
			{
				gnuplotPlotCmd += " '-' using 1:2 " + GNUPLOT_COLORS[groupI] + " " + GNUPLOT_MARKERS[dataSetI] + " title \"G" + str(groupI + 1) + " " + plotInfo.getLegendText(dataSetI) + "\",";
				if (plotInfo.whenToFit(dataSetI) == REAL_TIME_FIT
					|| (plotInfo.whenToFit(dataSetI) == FIT_AT_END
					&& repNum == input->picsPerVariation))
				{
					std::string fitNum = str(plotInfo.getPixelGroupNumber() * dataSetI + groupI);
					std::string plotString = "fit" + str(plotInfo.getPixelGroupNumber() * dataSetI + groupI) + "= ";
					switch (plotInfo.getFitOption(dataSetI))
					{
						case GAUSSIAN_FIT:
						{
							plotString += "sprintf(\"%.3f * exp{/Symbol \\\\173}-(x - %.3f)^2 / (2 * %.3f){/Symbol \\\\175}\", A" + fitNum + ", B" + fitNum + ", C"
								+ fitNum + ")\n";
							break;
						}
						case LORENTZIAN_FIT:
						{

							plotString += "sprintf(\"(%.3f / (2 * Pi)) / ((x - %.3f)^2 + ( %.3f / 2)^2)\", A" + fitNum + ", B" + fitNum
								+ ", A" + fitNum + ")\n";
							break;
						}
						case SINE_FIT:
						{
							plotString += "sprintf(\"%.3f * sin{/Symbol \\\\173}%.3f * x + %.3f{/Symbol \\\\175} * exp{/Symbol \\\\173} - %.3f * x {/Symbol \\\\175}\", A" + fitNum + ", B" + fitNum
								+ ", C" + fitNum + ", D" + fitNum + ")\n";
							break;
						}
						default:
						{
							errBox("Coding Error: Bad Fit option!");
						}
					}
					input->plotter->send(plotString);
					gnuplotPlotCmd += " f" + fitNum + "(x) title fit" + str(groupI) + " " + GNUPLOT_COLORS[groupI]
						+ " " + GNUPLOT_LINETYPES[dataSetI] + ",";
				}
			}
		}
		for (UINT dataSetI = 0; dataSetI < plotInfo.getDataSetNumber(); dataSetI++)
		{
			for (UINT groupI = 0; groupI < plotInfo.getPixelGroupNumber(); groupI++)
			{
				if (finData[dataSetI][groupI].size() >= input->numberOfRunsToAverage)
				{
					gnuplotPlotCmd += " '-' using 1:2 " + GNUPLOT_COLORS[groupI] + " with lines title \"G"
						+ str(groupI + 1) + " " + plotInfo.getLegendText(dataSetI) + "\",";
				}
			}
		}
	}
	else
	{
		for (UINT dataSetI = 0; dataSetI < plotInfo.getDataSetNumber(); dataSetI++)
		{
			for (UINT groupI = 0; groupI < finAvgs[dataSetI].size(); groupI++)
			{
				gnuplotPlotCmd += " '-' using 1:2:3 with yerrorbars title \"G" + str(groupI + 1) + " "
					+ plotInfo.getLegendText(dataSetI) + "\" " + GNUPLOT_COLORS[groupI] + " "
					+ GNUPLOT_MARKERS[dataSetI] + " pointsize 0.5,";
				if (plotInfo.whenToFit(dataSetI) == REAL_TIME_FIT
					|| (plotInfo.whenToFit(dataSetI) == FIT_AT_END && repNum == input->picsPerVariation))
				{
					std::string fitNum = str(plotInfo.getPixelGroupNumber() * dataSetI + groupI);
					std::string plotString = "fit" + fitNum + "= ";
					switch (plotInfo.getFitOption(dataSetI))
					{
						case GAUSSIAN_FIT:
						{
							plotString += "sprintf(\"%.3f * exp{/Symbol \\\\173}-(x - %.3f)^2 / (2 * %.3f){/Symbol \\\\175}\", A" + fitNum + ", B" + fitNum + ", C" + fitNum + ")\n";
							break;
						}
						case LORENTZIAN_FIT:
						{
							plotString += "sprintf(\"(%.3f / (2 * Pi)) / ((x - %.3f)^2 + (%.3f / 2)^2)\", A" + fitNum + ", B" + fitNum + ", A" + fitNum + ")\n";
							break;
						}
						case SINE_FIT:
						{
							plotString += "sprintf(\"%.3f * sin{/Symbol \\\\173}%.3f * x + %.3f{/Symbol \\\\175} * exp{/Symbol \\\\173} - %.3f * x {/Symbol \\\\175}\", A" + fitNum + ", B" + fitNum + ", C" + fitNum + ", D" + fitNum + ")\n";
							break;
						}
						default:
						{
							errBox("Coding Error: Bad Fit option!");
						}
					}
					input->plotter->send(plotString);
					gnuplotPlotCmd += " f" + fitNum + "(x) title fit" + fitNum + " " + GNUPLOT_COLORS[groupI] + " " + GNUPLOT_LINETYPES[dataSetI] + ",";
				}
			}
		}
	}
	std::string error;
	input->plotter->send(gnuplotPlotCmd);
	/// SEND DATA
	if (plotInfo.getXAxis() == "Running Average")
	{
		for (UINT dataSetI = 0; dataSetI < plotInfo.getDataSetNumber(); dataSetI++)
		{
			for (UINT groupI = 0; groupI < plotInfo.getPixelGroupNumber(); groupI++)
			{
				input->plotter->sendData(finAvgs[dataSetI][groupI], finData[dataSetI][groupI]);
			}
			for (UINT groupI = 0; groupI < plotInfo.getPixelGroupNumber(); groupI++)
			{
				if (finData[dataSetI][groupI].size() >= input->numberOfRunsToAverage)
				{
					input->plotter->sendData(finX[dataSetI][groupI], finAvgs[dataSetI][groupI]);
				}
			}
		}
	}
	else
	{
		for (UINT dataSetI = 0; dataSetI < plotInfo.getDataSetNumber(); dataSetI++)
		{
			for (UINT groupI = 0; groupI < finAvgs[dataSetI].size(); groupI++)
			{
				input->plotter->sendData(finX[dataSetI][groupI], finAvgs[dataSetI][groupI], finErrs[dataSetI][groupI]);
			}
		}
	}
}



void DataAnalysisControl::handlePlotHist(realTimePlotterInput* input, PlottingInfo plotInfo, UINT plotNumber, 
										 std::vector<std::vector<long>> countData, 
										 std::vector<std::vector<std::vector<long>>>& finData,
										 std::vector<std::vector<bool>>pscSatisfied)
{
	/// options are fundamentally different for histograms.

	// load pixel counts into data array pixelData
	for (UINT dataSetI = 0; dataSetI < plotInfo.getDataSetNumber(); dataSetI++)
	{
		for (UINT groupI = 0; groupI < plotInfo.getPixelGroupNumber(); groupI++)
		{
			if (pscSatisfied[dataSetI][groupI] == false)
			{
				continue;
			}
			UINT pixel, pic;
			// passing by reference.
			plotInfo.getDataCountsLocation(dataSetI, pixel, pic);
			// for a given group, figure out which picture
			finData[dataSetI][groupI].push_back(countData[plotInfo.getPixelIndex(pixel, groupI)][pic]);
		}
	}
	//
	input->plotter->send("set terminal wxt " + str(plotNumber) + " title \"" + plotInfo.getTitle() + "\" noraise background rgb 'black'");
	input->plotter->send( "set title \"" + plotInfo.getTitle( ) + "\" tc rgb 'white'" );
	input->plotter->send( "set xlabel \"Key Value\" tc rgb 'white'" );
	input->plotter->send( "set ylabel \"" + plotInfo.getYLabel( ) + "\" tc rgb 'white'" );
	input->plotter->send( "set border lc rgb 'white'" );
	input->plotter->send( "set key tc rgb 'white'" );	input->plotter->send("set title \"" + plotInfo.getTitle() + "\"");
	input->plotter->send("set format y \"%.1f\"");
	input->plotter->send("set autoscale x");
	input->plotter->send("set yrange [0:*]");
	input->plotter->send("set xlabel \"Count #\"");
	input->plotter->send("set ylabel \"Occurrences\"");
	double spaceFactor = 0.8;
	double boxWidth = spaceFactor * 5 / (plotInfo.getPixelGroupNumber() * plotInfo.getDataSetNumber());
	input->plotter->send("set boxwidth " + str(boxWidth));
	input->plotter->send("set style fill solid 1");
	// leave 0.2 pixels worth of space in between the bins.
	std::string gnuCommand = "plot";
	int totalDataSetNum = plotInfo.getDataSetNumber();
	int totalGroupNum = plotInfo.getPixelGroupNumber();
	for (UINT dataSetI = 0; dataSetI < plotInfo.getDataSetNumber(); dataSetI++)
	{
		for (UINT groupI = 0; groupI < plotInfo.getPixelGroupNumber(); groupI++)
		{
			// long command that makes hist correctly.
			std::string singleHist = " '-' using (5 * floor(($1)/5) - " + str(boxWidth * (totalGroupNum * dataSetI + groupI)
																			  - spaceFactor * 0.5 + spaceFactor * 0.5 / (totalGroupNum * totalDataSetNum))
				+ ") : (1.0) smooth freq with boxes title \"G " + str(groupI + 1) + " "
				+ plotInfo.getLegendText(dataSetI) + "\" " + GNUPLOT_COLORS[groupI] + " " + GNUPLOT_MARKERS[dataSetI] + ",";
			gnuCommand += singleHist;
		}
	}
	input->plotter->send(gnuCommand);
	for (UINT dataSetI = 0; dataSetI < plotInfo.getDataSetNumber(); dataSetI++)
	{
		for (UINT groupI = 0; groupI < plotInfo.getPixelGroupNumber(); groupI++)
		{
			input->plotter->sendData(finData[dataSetI][groupI]);
		}
	}
}

///

void DataAnalysisControl::fillPlotThreadInput(realTimePlotterInput* input)
{
	std::vector<tinyPlotInfo> usedPlots;
	input->plotInfo.clear();

	for (auto plt : allPlots)
	{
		if (plt.isActive)
		{
			input->plotInfo.push_back(plt);
		}
	}

	input->analysisLocations = getAnalysisLocs();
	input->plottingFrequency = updateFrequency;

	// as I fill the input, also check this, which is necessary info for plotting.
	threadNeedsCounts = false;
	for (auto plt : input->plotInfo)
	{
		PlottingInfo info(PLOT_FILES_SAVE_LOCATION + "\\" + plt.name + PLOTTING_EXTENSION);
		if (info.getPlotType() != "Atoms")
		{
			threadNeedsCounts = true;
		}
	}
	
	input->needsCounts = threadNeedsCounts;
}


void DataAnalysisControl::rearrange(std::string cameraMode, std::string trigMode, int width, int height, fontMap fonts)
{
	updateFrequencyLabel1.rearrange(cameraMode, trigMode, width, height, fonts);
	updateFrequencyLabel2.rearrange(cameraMode, trigMode, width, height, fonts);
	updateFrequencyEdit.rearrange(cameraMode, trigMode, width, height, fonts);
	header.rearrange(cameraMode, trigMode, width, height, fonts);
	plotListview.rearrange(cameraMode, trigMode, width, height, fonts);
	currentDataSetNumberEdit.rearrange( cameraMode, trigMode, width, height, fonts );
	currentDataSetNumberText.rearrange( cameraMode, trigMode, width, height, fonts );
	setAnalysisLocationsButton.rearrange( cameraMode, trigMode, width, height, fonts );
}

std::vector<std::string> DataAnalysisControl::getActivePlotList()
{
	std::vector<std::string> list;
	for ( auto plot : allPlots )
	{
		if ( plot.isActive ) 
		{
			list.push_back( plot.name );
		}
	}
	return list;
}


bool DataAnalysisControl::getLocationSettingStatus()
{
	return currentlySettingAnalysisLocations;
}

void DataAnalysisControl::updateDataSetNumberEdit( int number )
{
	if ( number > 0 )
	{
		currentDataSetNumberEdit.SetWindowTextA( cstr( number ) );
	}
	else
	{
		currentDataSetNumberEdit.SetWindowTextA( "None" );
	}
}

void DataAnalysisControl::analyze( std::string date, long runNumber, long accumulations, 
								   EmbeddedPythonHandler* pyHandler, Communicator* comm )
{
	// Note: python is initialized in the constructor for the data handler object. 
	// Get information to send to the python script from inputParam
	pyHandler->runDataAnalysis( date, runNumber, accumulations, atomLocations );
}

bool DataAnalysisControl::buttonClicked()
{
	return setAnalysisLocationsButton.GetCheck();
}

// handles the pressing of the analysis points button.
// TODO: handle different cases where single locations or pairs of locations are being analyzed. 
void DataAnalysisControl::onButtonPushed()
{	
	if ( setAnalysisLocationsButton.GetCheck() )
	{
		// if pressed currently, then upress it.
		setAnalysisLocationsButton.SetCheck(0);
		setAnalysisLocationsButton.SetWindowTextA( "Set Analysis Points" );
		currentlySettingAnalysisLocations = false;
	}
	else
	{
		// else press it.
		atomLocations.clear();
		setAnalysisLocationsButton.SetCheck( 1 );
		setAnalysisLocationsButton.SetWindowTextA( "Right-Click Relevant Points and Reclick" );
		currentlySettingAnalysisLocations = true;
	}
}


void DataAnalysisControl::setAtomLocation( std::pair<UINT, UINT> location )
{
	bool exists = false;
	for (UINT locInc = 0; locInc < atomLocations.size(); locInc++ )
	{
		if ( location == atomLocations[locInc] )
		{
			exists = true;
		}
	}
	if ( !exists )
	{
		atomLocations.push_back( location );
	}
}


std::vector<std::pair<UINT, UINT>> DataAnalysisControl::getAnalysisLocs()
{
	return atomLocations;
}


void DataAnalysisControl::clearAtomLocations()
{
	atomLocations.clear();
}


void DataAnalysisControl::handleDoubleClick(fontMap* fonts, UINT currentPicsPerRepetition)
{
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	plotListview.ScreenToClient(&cursorPos);
	int subitemIndicator;
	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	int itemIndicator;
	plotListview.SubItemHitTest(&myItemInfo);
	itemIndicator = myItemInfo.iItem;
	subitemIndicator = myItemInfo.iSubItem;
	if (itemIndicator == -1)
	{
		// user didn't click in an item.
		return;
	}

	LVITEM listViewItem;
	memset(&listViewItem, 0, sizeof(listViewItem));
	listViewItem.mask = LVIF_TEXT;
	listViewItem.cchTextMax = 256;
	listViewItem.iItem = itemIndicator;
	if (itemIndicator == allPlots.size())
	{
		// open plot creator.
		PlotDesignerDialog dlg(fonts, currentPicsPerRepetition);
		dlg.DoModal();
		reloadListView();
		return;
	}
	/// Handle different subitem clicks
	switch (subitemIndicator)
	{
		case 0:
		{
			/// person name
			// prompt for a name
			std::string newName;
			//std::string newName = (const char*)DialogBoxParam(NULL, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, (LPARAM)"Please enter a name for the variable:");
			if (newName == "")
			{
				// probably canceled.
				break;
			}
			// rename the file 
			// ...
			// update the screen
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;
			listViewItem.pszText = (LPSTR)cstr(newName);
			plotListview.SetItem(&listViewItem);
			break;
		}
		case 1:
		{
			// edit existing plot file using the plot designer.
			try
			{
				PlotDesignerDialog dlg(fonts, PLOT_FILES_SAVE_LOCATION + "\\" + allPlots[itemIndicator].name 
								 + PLOTTING_EXTENSION);
				dlg.DoModal();
			}
			catch (Error& err)
			{
				errBox(err.what());
			}
			reloadListView();
			break;
		}
		case 2:
		{
			/// view plot settings.
			try
			{
				infoBox(PlottingInfo::getAllSettingsStringFromFile(PLOT_FILES_SAVE_LOCATION + "\\" + allPlots[itemIndicator].name
						+ PLOTTING_EXTENSION));
			}
			catch (Error& err)
			{
				errBox(err.what());
			}
			break;
		}
		case 3:
		{
			/// toggle active
			listViewItem.iItem = itemIndicator;
			listViewItem.iSubItem = subitemIndicator;

			if (allPlots[itemIndicator].isActive)
			{
				allPlots[itemIndicator].isActive = false;
				listViewItem.pszText = "NO";
			}
			else
			{
				allPlots[itemIndicator].isActive = true;
				listViewItem.pszText = "YES";
			}
			plotListview.SetItem(&listViewItem);			
			break;
		}
	}
}


void DataAnalysisControl::reloadListView()
{
	std::vector<std::string> names = ProfileSystem::searchForFiles(PLOT_FILES_SAVE_LOCATION, str("*") 
																   + PLOTTING_EXTENSION);
	plotListview.DeleteAllItems();
	allPlots.clear();
	for (auto item : range(names.size()))
	{
		LVITEM listViewItem;
		memset(&listViewItem, 0, sizeof(listViewItem));
		listViewItem.mask = LVIF_TEXT; 
		listViewItem.cchTextMax = 256; 
		listViewItem.iItem = item;
		//listViewItem.pszText = (LPSTR)cstr(names[item]);
		listViewItem.pszText = (LPSTR)names[item].c_str();
		plotListview.InsertItem(&listViewItem);
		listViewItem.iSubItem = 3;
		listViewItem.pszText = "NO";
		plotListview.SetItem(&listViewItem);
		tinyPlotInfo tempInfo;
		tempInfo.name = names[item];
		tempInfo.isActive = false;
		allPlots.push_back(tempInfo);
	}
	// Make First Blank row.
	LVITEM listViewDefaultItem;
	memset(&listViewDefaultItem, 0, sizeof(listViewDefaultItem));
	// Text Style
	listViewDefaultItem.mask = LVIF_TEXT;
	// Max size of test
	listViewDefaultItem.cchTextMax = 256;
	listViewDefaultItem.pszText = "___";
	// choose item  
	listViewDefaultItem.iItem = names.size();
	// Put in first coluom
	listViewDefaultItem.iSubItem = 0;
	plotListview.InsertItem(&listViewDefaultItem);
	// Add SubItems in a loop
	for (int itemInc = 1; itemInc <= 3; itemInc++)
	{
		listViewDefaultItem.iSubItem = itemInc;
		// Enter text to SubItems
		plotListview.SetItem(&listViewDefaultItem);
	}
}


void DataAnalysisControl::handleRClick()
{
	// delete...
	/// get the item and subitem
	POINT cursorPos;
	GetCursorPos(&cursorPos);
	plotListview.ScreenToClient(&cursorPos);
	int subitemIndicator = plotListview.HitTest(cursorPos);
	LVHITTESTINFO myItemInfo;
	memset(&myItemInfo, 0, sizeof(LVHITTESTINFO));
	myItemInfo.pt = cursorPos;
	int itemIndicator = plotListview.SubItemHitTest(&myItemInfo);
	if (itemIndicator == -1 || itemIndicator == allPlots.size())
	{
		// user didn't click in a deletable item.
		return;
	}
	int answer = promptBox("Delete Plot " + allPlots[itemIndicator].name + "?", MB_YESNO);
	if (answer == IDYES)
	{
		int result = DeleteFile(cstr(PLOT_FILES_SAVE_LOCATION + "\\" + allPlots[itemIndicator].name + PLOTTING_EXTENSION));
		if (!result)
		{
			errBox("Failed to delete script file! Error code: " + str(GetLastError()));
			return;
		}
		plotListview.DeleteItem(itemIndicator);
		allPlots.erase(allPlots.begin() + itemIndicator);
	}
}

