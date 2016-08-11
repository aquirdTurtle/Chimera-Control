#include "stdafx.h"
#include "arbitraryPlottingThreadProcedure.h"
#include "Windows.h"
#include <string>
#include <vector>
#include "PlottingInfo.h"
#include <algorithm>
#include <numeric>
#include "externals.h"


unsigned __stdcall arbitraryPlottingThreadProcedure(LPVOID inputParam)
{
	imageParameters currentParameters = eImageParameters.getImageParameters();
	// Register any windows messages for the main window
	ePlottingIsSlowMessage = RegisterWindowMessage("ID_PLOTTING_IS_SLOW");
	ePlottingCaughtUpMessage = RegisterWindowMessage("ID_PLOTTING_CAUGHT_UP");
	
	// vector of filenames for the plotting info.
	std::vector<std::string>* inputPointer = (std::vector<std::string>*)inputParam;
	// make vector of plot information classes. 
	std::vector<PlottingInfo> allPlottingInfo;		
	allPlottingInfo.resize(inputPointer->size());
	/// open files
	for (int plotInc = 0; plotInc < allPlottingInfo.size(); plotInc++)
	{
		std::string tempFileName = PLOT_FILES_SAVE_LOCATION + (*inputPointer)[plotInc] + ".plot";
		allPlottingInfo[plotInc].loadPlottingInfoFromFile(tempFileName);
	}
	// now kill the input.
	delete inputPointer;
	// run some checks...
	if (allPlottingInfo.size() == 0)
	{
		// no plots, quit.
		return 0;
	}
	/// check pictures per experiment
	int picturesPerExperiment = allPlottingInfo[0].getPictureNumber();
	for (int plotInc = 0; plotInc < allPlottingInfo.size(); plotInc++)
	{
		if (picturesPerExperiment != allPlottingInfo[plotInc].getPictureNumber())
		{
			MessageBox(0, "ERROR: Number of pictures per experiment don't match between plots.", 0, 0);
			return 0;
		}
	}
	// create vector size of image being taken
	// first entry is row, second is collumn, third is data type (0 = pixel counts only, 1 = atom presense)
	std::vector<std::array<int, 3> > pixelDataType;
	int totalNumberOfPixels = 0;
	int numberOfLossDataPixels = 0;

	/// figure out which pixels need any data
	for (int plotInc = 0; plotInc < allPlottingInfo.size(); plotInc++)
	{
		for (int pixelInc = 0; pixelInc < allPlottingInfo[plotInc].getPixelNumber(); pixelInc++)
		{
			for (int groupInc = 0; groupInc < allPlottingInfo[plotInc].getPixelGroupNumber(); groupInc++)
			{
				int row, collumn;
				bool alreadyExists = false;
				allPlottingInfo[plotInc].getPixelLocation(pixelInc, groupInc, row, collumn);
				for (int savedPixelInc = 0; savedPixelInc < pixelDataType.size(); savedPixelInc++)
				{
					// figure out if it already exists
					if (allPlottingInfo[plotInc].getPlotType() == "Atoms")
					{
						std::array<int, 3> testArray = { {row, collumn, 1} };
						if (pixelDataType[savedPixelInc] == testArray)
						{
							alreadyExists = true;
							allPlottingInfo[plotInc].setPixelIndex(pixelInc, groupInc, savedPixelInc);
						}
					}
					else
					{
						std::array<int, 3> testArray1 = { { row, collumn, 1 } }, testArray2 = { { row, collumn, 0 } };
						if (pixelDataType[savedPixelInc] == testArray1 || pixelDataType[savedPixelInc] == testArray2)
						{
							alreadyExists = true;
							allPlottingInfo[plotInc].setPixelIndex(pixelInc, groupInc, savedPixelInc);
						}
					}
				}
				// if doesn't already exist, add it.
				if (!alreadyExists)
				{
					if (allPlottingInfo[plotInc].getPlotType() == "Atoms")
					{
						// in this case I need atom data
						pixelDataType.push_back({ row, collumn, 1 });
						allPlottingInfo[plotInc].setPixelIndex(pixelInc, groupInc, totalNumberOfPixels);
						totalNumberOfPixels++;
					}
					else
					{
						// in this case I need atom data
						pixelDataType.push_back({ row, collumn, 0 });
						allPlottingInfo[plotInc].setPixelIndex(pixelInc, groupInc, totalNumberOfPixels);
						totalNumberOfPixels++;
						numberOfLossDataPixels++;
					}
				}
			}
		}
	}
	// set flag?

	/// Initialize Arrays for data.
	// thinking about making these experiment-picture sized and resetting after getting the needed data out of them.
	// pixelData[pixel Indicator][picture number indicator] = pixelCount;
	std::vector<std::vector<int >> pixelData;
	pixelData.resize(totalNumberOfPixels);
	// atomPresentData[pixelIndicator][picture number] = true if atom present, false if atom not present;
	std::vector<std::vector<int> > atomPresentData;
	atomPresentData.resize(totalNumberOfPixels);
	// finalData[plot][dataset][group][accumulationNumber];
	std::vector<std::vector<std::vector<std::vector<int> > > > finalData;
	// finalAverages[plot][dataset][group][ExperimentNumber];
	std::vector<std::vector<std::vector<std::vector<double> > > > finalAverages;
	// finalErrorBars[plot][dataset][group][ExperimentNumber];
	std::vector<std::vector<std::vector<std::vector<double> > > > finalErrorBars;
	// finalXVals[plot][dataset][group][ExperimentNumber];
	std::vector<std::vector<std::vector<std::vector<double> > > > finalXVals;
	// newData[plot][dataSet][group] = true if new data so change some vector sizes.
	std::vector<std::vector<std::vector<bool> > > newData;
	// size things up...
	finalData.resize(allPlottingInfo.size());
	finalAverages.resize(allPlottingInfo.size());
	finalErrorBars.resize(allPlottingInfo.size());
	finalXVals.resize(allPlottingInfo.size());
	newData.resize(allPlottingInfo.size());
	// much sizing...
	for (int plotInc = 0; plotInc < allPlottingInfo.size(); plotInc++)
	{
		finalData[plotInc].resize(allPlottingInfo[plotInc].getDataSetNumber());
		finalAverages[plotInc].resize(allPlottingInfo[plotInc].getDataSetNumber());
		finalErrorBars[plotInc].resize(allPlottingInfo[plotInc].getDataSetNumber());
		finalXVals[plotInc].resize(allPlottingInfo[plotInc].getDataSetNumber());
		newData[plotInc].resize(allPlottingInfo[plotInc].getDataSetNumber());
		for (int dataSetInc = 0; dataSetInc < allPlottingInfo[plotInc].getDataSetNumber(); dataSetInc++)
		{
			finalData[plotInc][dataSetInc].resize(allPlottingInfo[plotInc].getPixelGroupNumber());
			finalAverages[plotInc][dataSetInc].resize(allPlottingInfo[plotInc].getPixelGroupNumber());
			finalErrorBars[plotInc][dataSetInc].resize(allPlottingInfo[plotInc].getPixelGroupNumber());
			finalXVals[plotInc][dataSetInc].resize(allPlottingInfo[plotInc].getPixelGroupNumber());
			newData[plotInc][dataSetInc].resize(allPlottingInfo[plotInc].getPixelGroupNumber());
			for (int groupInc = 0; groupInc < allPlottingInfo[plotInc].getPixelGroupNumber(); groupInc++)
			{
				newData[plotInc][dataSetInc][groupInc] = true;
			}
		}
	}
	
	int noAtomsCounter = 0;

	int plotNumberCount = 0;
	// this effectively just keeps track of whether a "slow" message has been sent to the main window yet or not. Only want to send once.
	bool plotIsSlowStatus = false;
	/// /////////////////////////////////////////////
	/// /////////////////////////////////////////////
	/// Start loop waiting for plots
	/// /////////////////////////////////////////////
	while (ePlotThreadExitIndicator || (eImageVecQueue.size() > 0))
	{
		// if no image, continue.
		if (eImageVecQueue.size() > 0)
		{
			if (eImageVecQueue.size() > 2 && plotIsSlowStatus == false)
			{
				PostMessage(eCameraWindowHandle, ePlottingIsSlowMessage, 0, 0);
				plotIsSlowStatus = true;
				// TODO: post warning to the main thread that the plotting thread is running behind.
			}
			else if (eImageVecQueue.size() == 1 && plotIsSlowStatus == true)
			{
				// the plotting has caught up, reset this.
				plotIsSlowStatus = false;
			}
			/// for all pixels... gather count information
			for (unsigned int pixelInc = 0; pixelInc < pixelDataType.size(); pixelInc++)
			{
				int secondIndex = pixelDataType[pixelInc][0] - 1 + currentParameters.width * (pixelDataType[pixelInc][1] - 1);
				// first image, second index identifies the location.
				try
				{
					pixelData.at(pixelInc);
					eImageVecQueue.at(0);
					eImageVecQueue[0].at(secondIndex);
					pixelData[pixelInc].push_back(eImageVecQueue[0][secondIndex]);
				}
				catch (std::out_of_range outOfRange)
				{
					// try again. 
					pixelInc--;
					continue;
				}
			}
		
			/// get all the atom data
			bool isAtLeastOneAtom = false;
			for (unsigned int pixelInc = 0; pixelInc < pixelDataType.size(); pixelInc++)
			{
				if (pixelData[pixelInc].back() > eDetectionThreshold)
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
			if (noAtomsCounter >= eAlerts.getAlertThreshold())
			{
				if (eAlerts.alertsAreToBeUsed())
				{
					eAlerts.alertMainThread();
				}
				noAtomsCounter = 0;
			}

			/// check if have enough data to plot
			if (eCurrentThreadAccumulationNumber % picturesPerExperiment != 0)
			{
				// finally, if so, remove the data from the queue.
				DWORD mutexMsg = WaitForSingleObject(ePlottingMutex, INFINITE);
				switch (mutexMsg)
				{
					case WAIT_OBJECT_0:
					{
						if (eImageVecQueue.size() > 0)
						{
							// delete the first entry of the Queue which has just been handled.
							eImageVecQueue.erase(eImageVecQueue.begin());
						}
						// increment the thread's accumulation Number.
						eCurrentThreadAccumulationNumber++;
						eCount2++;
						break;
					}
					case WAIT_ABANDONED:
					{
						// TODO:
						// handle error...
						break;
					}
					default:
					{
						// handle error...
						break;
					}
				}
				ReleaseMutex(ePlottingMutex);
				continue;
			}
			// used to tell if time to plot given frequency.
			plotNumberCount++;
			// for every plot...
			for (int plotInc = 0; plotInc < allPlottingInfo.size(); plotInc++)
			{
				/// DATA ANALYSIS
				/// Data Set Loop - need to kill.
				//for (int dataSetInc = 0; dataSetInc < allPlottingInfo[plotInc].getDataSetNumber(); dataSetInc++)
				//{
					/// ///////////////////////////////
					/// Check Post-Selection Conditions
					// initialize this vector to all true. 
					// satisfiesPostSelectionConditions[dataSetInc][groupInc] = true or false
					std::vector<std::vector<bool> > satisfiesPostSelectionConditions;
					satisfiesPostSelectionConditions.resize(allPlottingInfo[plotInc].getDataSetNumber());
					for (int dataSetInc = 0; dataSetInc < allPlottingInfo[plotInc].getDataSetNumber(); dataSetInc++)
					{
						satisfiesPostSelectionConditions[dataSetInc].resize(allPlottingInfo[plotInc].getPixelGroupNumber());
						for (int groupInc = 0; groupInc < satisfiesPostSelectionConditions[dataSetInc].size(); groupInc++)
						{
							satisfiesPostSelectionConditions[dataSetInc][groupInc] = true;
						}
					}
					// check if actually true.
					for (int dataSetInc = 0; dataSetInc < allPlottingInfo[plotInc].getDataSetNumber(); dataSetInc++)
					{
						for (int groupInc = 0; groupInc < allPlottingInfo[plotInc].getPixelGroupNumber(); groupInc++)
						{
							for (int conditionInc = 0; conditionInc < allPlottingInfo[plotInc].getConditionNumber(); conditionInc++)
							{
								for (int pixelInc = 0; pixelInc < allPlottingInfo[plotInc].getPixelNumber(); pixelInc++)
								{
									for (int pictureInc = 0; pictureInc < allPlottingInfo[plotInc].getPictureNumber(); pictureInc++)
									{
										// test if condition exists
										int condition = allPlottingInfo[plotInc].getPostSelectionCondition(dataSetInc, conditionInc, pixelInc, pictureInc);
										if (condition != 0)
										{
											if (condition == 1 && atomPresentData[allPlottingInfo[plotInc].getPixelIndex(pixelInc, groupInc)][pictureInc] != 1)
											{
												satisfiesPostSelectionConditions[dataSetInc][groupInc] = false;
											}
											else if (condition == -1 && atomPresentData[allPlottingInfo[plotInc].getPixelIndex(pixelInc, groupInc)][pictureInc] != 0)
											{
												satisfiesPostSelectionConditions[dataSetInc][groupInc] = false;
											}
										}
									}
								}
							}
						}
					}
					/// ??? (Title?)
					if (allPlottingInfo[plotInc].getPlotType() == "Atoms" || allPlottingInfo[plotInc].getPlotType() == "Pixel Counts")
					{
						if (eCurrentThreadAccumulationNumber % ePicturesPerVariation == picturesPerExperiment)
						{
							for (int dataSetInc = 0; dataSetInc < allPlottingInfo[plotInc].getDataSetNumber(); dataSetInc++)
							{
								// TODO: if x axis = average over experiments... else...
								finalData[plotInc][dataSetInc].clear();
								finalData[plotInc][dataSetInc].resize(allPlottingInfo[plotInc].getPixelGroupNumber());
								newData[plotInc][dataSetInc].resize(allPlottingInfo[plotInc].getPixelGroupNumber());
								std::fill(newData[plotInc][dataSetInc].begin(), newData[plotInc][dataSetInc].end(), true);
							}
						}
						/// Check Data Conditions
						if (allPlottingInfo[plotInc].getPlotType() == "Atoms")
						{
							for (int dataSetInc = 0; dataSetInc < allPlottingInfo[plotInc].getDataSetNumber(); dataSetInc++)
							{
								for (int groupInc = 0; groupInc < allPlottingInfo[plotInc].getPixelGroupNumber(); groupInc++)
								{
									if (satisfiesPostSelectionConditions[dataSetInc][groupInc] == false)
									{
										// no new data.
										continue;
									}
									bool dataVal = true;
									for (int pixelInc = 0; pixelInc < allPlottingInfo[plotInc].getPixelNumber(); pixelInc++)
									{
										for (int pictureInc = 0; pictureInc < allPlottingInfo[plotInc].getPictureNumber(); pictureInc++)
										{
											// check if there is a condition at all
											int truthCondition = allPlottingInfo[plotInc].getTruthCondition(dataSetInc, pixelInc, pictureInc);
											if (truthCondition != 0)
											{
												int pixel = allPlottingInfo[plotInc].getPixelIndex(pixelInc, groupInc);
												if (truthCondition == 1 && atomPresentData[pixel][pictureInc] != 1)
												{
													dataVal = false;
												}
												else if (truthCondition == 0 && atomPresentData[allPlottingInfo[plotInc].getPixelIndex(pixelInc, groupInc)][pictureInc] != 0)
												{
													dataVal = false;
												}
											}
										}
									}
									finalData[plotInc][dataSetInc][groupInc].push_back(dataVal);
									// then the size of the containers gets updated every time.
									if (allPlottingInfo[plotInc].getXAxis() == "Running Average")
									{
										newData[plotInc][dataSetInc][groupInc] = true;
									}
								}
							}
						}
						else if (allPlottingInfo[plotInc].getPlotType() == "Pixel Counts")
						{
							for (int dataSetInc = 0; dataSetInc < allPlottingInfo[plotInc].getDataSetNumber(); dataSetInc++)
							{
								for (int groupInc = 0; groupInc < allPlottingInfo[plotInc].getPixelGroupNumber(); groupInc++)
								{
									if (satisfiesPostSelectionConditions[dataSetInc][groupInc] == false)
									{
										continue;
									}
									int pixel, picture;
									// passing pixel and picture by reference.
									allPlottingInfo[plotInc].getDataCountsLocation(dataSetInc, pixel, picture);
									// for a given group, figure out which picture
									finalData[plotInc][dataSetInc][groupInc].push_back(pixelData[allPlottingInfo[plotInc].getPixelIndex(pixel, groupInc)][picture]);
								}
							}
						}
						std::vector<double> keyData = eExperimentData.getKey();
						/// Calculate averages and standard devations for Data sets AND groups...
						if (plotNumberCount % ePlottingFrequency == 0)
						{
							for (int dataSetInc = 0; dataSetInc < allPlottingInfo[plotInc].getDataSetNumber(); dataSetInc++)
							{
								for (int groupInc = 0; groupInc < allPlottingInfo[plotInc].getPixelGroupNumber(); groupInc++)
								{
									// check if first picture of set
									if (eCurrentThreadAccumulationNumber % ePicturesPerRepetition != 0)
									{
										continue;
									}
									// position for new data point.
									double position;
									if (newData[plotInc][dataSetInc][groupInc] == true)
									{
										finalAverages[plotInc][dataSetInc][groupInc].resize(finalAverages[plotInc][dataSetInc][groupInc].size() + 1);
										// integer division here.
										if (allPlottingInfo[plotInc].getXAxis() == "Running Average")
										{
											finalXVals[plotInc][dataSetInc][groupInc].resize(finalXVals[plotInc][dataSetInc][groupInc].size() + 1);
											if (!(finalData[plotInc][dataSetInc][groupInc].size() >= eNumberOfRunsToAverage))
											{
												position = (std::accumulate(finalXVals[plotInc][dataSetInc][groupInc].begin(), finalXVals[plotInc][dataSetInc][groupInc].end(), 0.0) + finalData[plotInc][dataSetInc][groupInc].size()) / finalData[plotInc][dataSetInc][groupInc].size();
												finalXVals[plotInc][dataSetInc][groupInc].back() = position;
											}
										}
										else
										{
											finalErrorBars[plotInc][dataSetInc][groupInc].resize(finalErrorBars[plotInc][dataSetInc][groupInc].size() + 1);
											position = (eCurrentThreadAccumulationNumber - 1) / ePicturesPerVariation + 1;
											
											finalXVals[plotInc][dataSetInc][groupInc].push_back(keyData[position - 1]);
										}
										// set the flag to not do this again before this array gets reset at beginning of the next accumulation stack.
										newData[plotInc][dataSetInc][groupInc] = false;
									}
									// calculate new data points
									if (allPlottingInfo[plotInc].getXAxis() == "Running Average")
									{
										if (finalData[plotInc][dataSetInc][groupInc].size() >= eNumberOfRunsToAverage)
										{
											double sum = std::accumulate(finalData[plotInc][dataSetInc][groupInc].end() - eNumberOfRunsToAverage, finalData[plotInc][dataSetInc][groupInc].end(), 0.0);
											double mean = sum / eNumberOfRunsToAverage;
											finalAverages[plotInc][dataSetInc][groupInc].back() = mean;
											position = (std::accumulate(finalXVals[plotInc][dataSetInc][groupInc].end() - eNumberOfRunsToAverage + 1, finalXVals[plotInc][dataSetInc][groupInc].end(), 0.0) + finalData[plotInc][dataSetInc][groupInc].size()) / eNumberOfRunsToAverage;
											finalXVals[plotInc][dataSetInc][groupInc].back() = position;
											std::string  plotString = "set xrange [" + std::to_string(finalXVals[plotInc][dataSetInc][groupInc][0] - 1) + ":" + std::to_string(finalXVals[plotInc][dataSetInc][groupInc].back() + 1) + "]\n";
											ePlotter << plotString;
										}
									}
									else
									{
										double sum = std::accumulate(finalData[plotInc][dataSetInc][groupInc].begin(), finalData[plotInc][dataSetInc][groupInc].end(), 0.0);
										double mean = sum / finalData[plotInc][dataSetInc][groupInc].size();
										double squaredSum = std::inner_product(finalData[plotInc][dataSetInc][groupInc].begin(), finalData[plotInc][dataSetInc][groupInc].end(),
											finalData[plotInc][dataSetInc][groupInc].begin(), 0.0);
										double error = ((double)std::sqrt(squaredSum / finalData[plotInc][dataSetInc][groupInc].size() - mean * mean))
											/ std::sqrt(finalData[plotInc][dataSetInc][groupInc].size());
										finalAverages[plotInc][dataSetInc][groupInc].back() = mean;
										
										finalErrorBars[plotInc][dataSetInc][groupInc].back() = error;
									}
									// 
								}
							}
							/// General Plotting Options
							std::string plotString;
							plotString = "set terminal wxt " + std::to_string(plotInc) + " title \"" + allPlottingInfo[plotInc].getTitle() + "\" noraise\n";
							ePlotter << plotString.c_str();
							plotString = "set format y \"%.1f\"\n";
							ePlotter << plotString.c_str();
							if (allPlottingInfo[plotInc].getPlotType() == "Atoms")
							{
								double xRangeMin = *std::min_element(keyData.begin(), keyData.end());
								double xRangeMax = *std::max_element(keyData.begin(), keyData.end());
								double range = xRangeMax - xRangeMin;
								xRangeMin -= range / keyData.size();
								xRangeMax += range / keyData.size();
								std::string  plotString = "set xrange [" + std::to_string(xRangeMin) + ":" + std::to_string(xRangeMax) + "]\n";
								ePlotter << plotString;
								plotString = "set yrange [-0.1:1.1]\n";																
							}
							else
							{
								plotString = "set autoscale y\n";
							}
							ePlotter << plotString.c_str();
							plotString = "set title \"" + allPlottingInfo[plotInc].getTitle() + "\"\n";
							ePlotter << plotString.c_str();
							plotString = "set xlabel \"Experiment #\"\n";
							ePlotter << plotString.c_str();
							plotString = "set ylabel \"" + allPlottingInfo[plotInc].getYLabel() + "\"\n";
							ePlotter << plotString.c_str();
							/// FITTING
							for (int dataSetInc = 0; dataSetInc < allPlottingInfo[plotInc].getDataSetNumber(); dataSetInc++)
							{
								if (allPlottingInfo[plotInc].getWhenToFit(dataSetInc) == REAL_TIME_FIT
									|| (allPlottingInfo[plotInc].getWhenToFit(dataSetInc) == FIT_AT_END
										&& eCurrentThreadAccumulationNumber == eTotalNumberOfPicturesInSeries))
								{
									for (int groupInc = 0; groupInc < allPlottingInfo[plotInc].getPixelGroupNumber(); groupInc++)
									{
										std::string fitNum = std::to_string(allPlottingInfo[plotInc].getPixelGroupNumber() * dataSetInc + groupInc);
										// in this case, fitting.
										switch (allPlottingInfo[plotInc].getFitOption(dataSetInc))
										{
											// the to_string argument in each case is a unique number indicating the fit given the data set and group. I need
											// to keep track of each fit separately so that I can plot them all later. 
											case GAUSSIAN_FIT:
											{
												plotString = "f" + fitNum + "(x) = A" + fitNum + " * exp(-(x - B" + fitNum + ")**2 / (2 * C" + fitNum + "))\n";
												ePlotter << plotString.c_str();
												plotString = "A" + fitNum + " = 1\n";
												ePlotter << plotString.c_str();
												plotString = "B" + fitNum + " = " + std::to_string(finalXVals[plotInc][dataSetInc][groupInc].size() / 2.0) + "\n";
												ePlotter << plotString.c_str();
												plotString = "C" + fitNum + " = 1\n";
												ePlotter << plotString.c_str();
												plotString = "fit f" + fitNum + "(x) '-' using 1:2 via A" + fitNum + ", B" + fitNum + ", C" + fitNum + "\n";
												ePlotter << plotString.c_str();
												break;
											}
											case LORENTZIAN_FIT:
											{
												plotString = "f" + fitNum + "(x) = (A" + fitNum + " / (2 * 3.14159265359)) / ((x - B" + fitNum + ")**2 + (A" + fitNum + " / 2)**2)\n";
												ePlotter << plotString.c_str();
												plotString = "A" + fitNum + " = 1\n";
												ePlotter << plotString.c_str();
												plotString = "B" + fitNum + " = " + std::to_string(finalXVals[plotInc][dataSetInc][groupInc].size() / 2.0) + "\n";
												ePlotter << plotString.c_str();
												plotString = "fit f" + fitNum + "(x) '-' using 1:2 via A" + fitNum + ", B" + fitNum + "\n";
												ePlotter << plotString.c_str();
												break;
											}
											case SINE_FIT:
											{
												plotString = "f" + fitNum + "(x) = A" + fitNum + " * sin(B" + fitNum + " * x + C" + fitNum + ") * exp( - D" + fitNum + " * x)\n";
												ePlotter << plotString.c_str();
												plotString = "A" + fitNum + " = 1\n";
												ePlotter << plotString.c_str();
												plotString = "B" + fitNum + " = 1\n";
												ePlotter << plotString.c_str();
												plotString = "C" + fitNum + " = 1\n";
												ePlotter << plotString.c_str();
												plotString = "D" + fitNum + " = 1\n";
												ePlotter << plotString.c_str();
												plotString = "fit f" + fitNum + "(x) '-' using 1:2 via A" + fitNum + ", B" + fitNum + ", C" + fitNum + ", D" + fitNum + "\n";
												ePlotter << plotString.c_str();
												break;
											}
											default:
											{
												MessageBox(0, "Coding Error: Bad Fit option!", 0, 0);
											}
										}
										ePlotter.send1d(boost::make_tuple(finalXVals[plotInc][dataSetInc][groupInc], finalAverages[plotInc][dataSetInc][groupInc]));
									}
								}
							}
							/// SEND PLOT COMMANDS AND DATA
							/// send plot commands
							std::string gnuplotPlotCommand = "plot";
							if (allPlottingInfo[plotInc].getXAxis() == "Running Average")
							{
								for (unsigned int dataSetInc = 0; dataSetInc < allPlottingInfo[plotInc].getDataSetNumber(); dataSetInc++)
								{
									for (unsigned int groupInc = 0; groupInc < allPlottingInfo[plotInc].getPixelGroupNumber(); groupInc++)
									{
										gnuplotPlotCommand += " '-' using 1:2 " + GNUPLOT_COLORS[dataSetInc] + " " + GNUPLOT_MARKERS[groupInc] + " title \"G" + std::to_string(groupInc + 1) + " " + allPlottingInfo[plotInc].getLegendText(dataSetInc) + "\",";
										if (allPlottingInfo[plotInc].getWhenToFit(dataSetInc) == REAL_TIME_FIT 
											|| (allPlottingInfo[plotInc].getWhenToFit(dataSetInc) == FIT_AT_END 
												&& eCurrentThreadAccumulationNumber == eTotalNumberOfPicturesInSeries))
										{
											std::string fitNum = std::to_string(allPlottingInfo[plotInc].getPixelGroupNumber() * dataSetInc + groupInc);
											plotString = "fit" + std::to_string(allPlottingInfo[plotInc].getPixelGroupNumber() * dataSetInc + groupInc) + "= ";
											switch (allPlottingInfo[plotInc].getFitOption(dataSetInc))
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
													MessageBox(0, "Coding Error: Bad Fit option!", 0, 0);
												}
											}
											ePlotter << plotString;
											gnuplotPlotCommand += " f" + fitNum + "(x) title fit" + std::to_string(groupInc) + " " + GNUPLOT_COLORS[dataSetInc] + " " + GNUPLOT_LINETYPES[groupInc] + ",";
										}
									}
								}
								for (unsigned int dataSetInc = 0; dataSetInc < allPlottingInfo[plotInc].getDataSetNumber(); dataSetInc++)
								{
									for (unsigned int groupInc = 0; groupInc < allPlottingInfo[plotInc].getPixelGroupNumber(); groupInc++)
									{
										if (finalData[plotInc][dataSetInc][groupInc].size() >= eNumberOfRunsToAverage)
										{
											gnuplotPlotCommand += " '-' using 1:2 " + GNUPLOT_COLORS[dataSetInc] + " with lines title \"G" 
												+ std::to_string(groupInc + 1) + " " + allPlottingInfo[plotInc].getLegendText(dataSetInc) + "\",";
										}
									}
								}
							}
							else
							{
								for (unsigned int dataSetInc = 0; dataSetInc < allPlottingInfo[plotInc].getDataSetNumber(); dataSetInc++)
								{
									for (unsigned int groupInc = 0; groupInc < finalAverages[plotInc][dataSetInc].size(); groupInc++)
									{
										gnuplotPlotCommand += " '-' using 1:2:3 with yerrorbars title \"G" + std::to_string(groupInc + 1) + " " 
											+ allPlottingInfo[plotInc].getLegendText(dataSetInc) + "\" " + GNUPLOT_COLORS[dataSetInc] + " " + GNUPLOT_MARKERS[groupInc] + ",";
										if (allPlottingInfo[plotInc].getWhenToFit(dataSetInc) == REAL_TIME_FIT
											|| (allPlottingInfo[plotInc].getWhenToFit(dataSetInc) == FIT_AT_END
												&& eCurrentThreadAccumulationNumber == eTotalNumberOfPicturesInSeries))
										{
											std::string fitNum = std::to_string(allPlottingInfo[plotInc].getPixelGroupNumber() * dataSetInc + groupInc);
											plotString = "fit" + fitNum + "= ";
											switch (allPlottingInfo[plotInc].getFitOption(dataSetInc))
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
													MessageBox(0, "Coding Error: Bad Fit option!", 0, 0);
												}
											}
											ePlotter << plotString;
											gnuplotPlotCommand += " f" + fitNum + "(x) title fit" + fitNum + " " + GNUPLOT_COLORS[dataSetInc] + " " + GNUPLOT_LINETYPES[groupInc] + ",";
										}
									}
								}
							}
							std::string error;
							gnuplotPlotCommand += "\n";
							ePlotter << gnuplotPlotCommand;
							/// SEND DATA
							if (allPlottingInfo[plotInc].getXAxis() == "Running Average")
							{
								for (unsigned int dataSetInc = 0; dataSetInc < allPlottingInfo[plotInc].getDataSetNumber(); dataSetInc++)
								{
									for (unsigned int groupInc = 0; groupInc < allPlottingInfo[plotInc].getPixelGroupNumber(); groupInc++)
									{
										ePlotter.send1d(boost::make_tuple(finalXVals[plotInc][dataSetInc][groupInc], finalData[plotInc][dataSetInc][groupInc]));
									}
									for (unsigned int groupInc = 0; groupInc < allPlottingInfo[plotInc].getPixelGroupNumber(); groupInc++)
									{
										if (finalData[plotInc][dataSetInc][groupInc].size() >= eNumberOfRunsToAverage)
										{
											ePlotter.send1d(boost::make_tuple(finalXVals[plotInc][dataSetInc][groupInc],
																			  finalAverages[plotInc][dataSetInc][groupInc]));
										}
									}
								}
							}
							else
							{
								for (unsigned int dataSetInc = 0; dataSetInc < allPlottingInfo[plotInc].getDataSetNumber(); dataSetInc++)
								{
									for (unsigned int groupInc = 0; groupInc < finalAverages[plotInc][dataSetInc].size(); groupInc++)
									{
										ePlotter.send1d(boost::make_tuple(finalXVals[plotInc][dataSetInc][groupInc],
																		  finalAverages[plotInc][dataSetInc][groupInc],
																		  finalErrorBars[plotInc][dataSetInc][groupInc]));
									}
								}
							}
						}
					}
					else if (allPlottingInfo[plotInc].getPlotType() == "Pixel Count Histograms")
					{
						/// options are fundamentally different for histograms.

						// load pixel counts into data array pixelData
						for (unsigned int dataSetInc = 0; dataSetInc < allPlottingInfo[plotInc].getDataSetNumber(); dataSetInc++)
						{
							for (int groupInc = 0; groupInc < allPlottingInfo[plotInc].getPixelGroupNumber(); groupInc++)
							{
								if (satisfiesPostSelectionConditions[dataSetInc][groupInc] == false)
								{
									continue;
								}
								int pixel, picture;
								// passing by reference.
								allPlottingInfo[plotInc].getDataCountsLocation(dataSetInc, pixel, picture);
								// for a given group, figure out which picture
								finalData[plotInc][dataSetInc][groupInc].push_back(pixelData[allPlottingInfo[plotInc].getPixelIndex(pixel, groupInc)][picture]);
							}
						}
						//
						ePlotter << ("set terminal wxt " + std::to_string(plotInc) + " title \"" + allPlottingInfo[plotInc].getTitle() + "\" noraise\n").c_str();
						ePlotter << ("set title \"" + allPlottingInfo[plotInc].getTitle() + "\"\n").c_str();
						ePlotter << "set format y \"%.1f\"\n";
						ePlotter << "set autoscale x\n";
						ePlotter << "set yrange [0:*]\n";
						ePlotter << "set xlabel \"Count #\"\n";
						ePlotter << "set ylabel \"Occurrences\"\n";
						double spaceFactor = 0.8;
						double boxWidth = spaceFactor * 5 / (allPlottingInfo[plotInc].getPixelGroupNumber() * allPlottingInfo[plotInc].getDataSetNumber());
						ePlotter << "set boxwidth " + std::to_string(boxWidth) + "\n";
						ePlotter << "set style fill solid 1\n";
						// leave 0.2 pixels worth of space in between the bins.
						std::string gnuCommand = "plot";
						int totalDataSetNum = allPlottingInfo[plotInc].getDataSetNumber();
						int totalGroupNum = allPlottingInfo[plotInc].getPixelGroupNumber();
						for (unsigned int dataSetInc = 0; dataSetInc < allPlottingInfo[plotInc].getDataSetNumber(); dataSetInc++)
						{
							for (unsigned int groupInc = 0; groupInc < allPlottingInfo[plotInc].getPixelGroupNumber(); groupInc++)
							{
								// long command that makes hist correctly.
								std::string singleHist = " '-' using (5 * floor(($1)/5) - " + std::to_string(boxWidth * (totalGroupNum * dataSetInc + groupInc)
									- spaceFactor * 0.5 + spaceFactor * 0.5 / (totalGroupNum * totalDataSetNum))
									+ ") : (1.0) smooth freq with boxes title \"G " + std::to_string(groupInc + 1) + " " 
									+ allPlottingInfo[plotInc].getLegendText(dataSetInc) + "\" " + GNUPLOT_COLORS[dataSetInc] + " " + GNUPLOT_MARKERS[groupInc] + ",";
								gnuCommand += singleHist;
							}
						}
						gnuCommand += "\n";
						ePlotter << gnuCommand;
						for (unsigned int dataSetInc = 0; dataSetInc < allPlottingInfo[plotInc].getDataSetNumber(); dataSetInc++)
						{
							for (unsigned int groupInc = 0; groupInc < allPlottingInfo[plotInc].getPixelGroupNumber(); groupInc++)
							{
								ePlotter.send1d(finalData[plotInc][dataSetInc][groupInc]);
							}
						}
					}
				//} end remove
			}
			// clear exp data
			// all pixels being recorded, not pixels in a data set.
			for (int pixelInc = 0; pixelInc < totalNumberOfPixels; pixelInc++)
			{
				pixelData[pixelInc].clear();
				atomPresentData[pixelInc].clear();
			}
			// finally, remove the data from the queue.
			DWORD mutexMsg = WaitForSingleObject(ePlottingMutex, INFINITE);
			switch (mutexMsg)
			{
				case WAIT_OBJECT_0:
				{
					if (eImageVecQueue.size() > 0)
					{
						// delete the first entry of the Queue which has just been handled.
						eImageVecQueue.erase(eImageVecQueue.begin());
					}
					// increment the thread's accumulation Number.
					eCurrentThreadAccumulationNumber++;
					eCount2++;
					break;
				}
				case WAIT_ABANDONED:
				{
					// handle error...
					break;
				}
				default:
				{
					// handle error...
					break;
				}
			}
			ReleaseMutex(ePlottingMutex);
		}
	}
	return 0;
}