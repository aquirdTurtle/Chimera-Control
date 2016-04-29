#include "stdafx.h"
#include "Windows.h"
#include "externals.h"
#include "constants.h"
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include "plotThread.h"

DWORD WINAPI plotThread() 
{
	std::vector<bool> mostRecentlyActive = { false , false, false, false, false, false, false };
	std::vector<int> allAtomSurvivalDataCount;
	// if only the indicator is true, the thread is up to speed. If the only the latter condition is true, the thread is still working after the image
	// acquisition has ended.
	while (ePlotThreadExitIndicator || (eImageVecQueue.size() >= 0)) 
	{
		if (eImageVecQueue.size() >= 0) 
		{
			if (eImageVecQueue.size() > 1) 
			{
				// TODO: post warning to the main thread that the plotting thread is running behind.
			}
			// If first run of an accumulation, reset the ePixelCounts array and the eAtomPresent Array.
			if (eCurrentThreadAccumulationNumber % ePicturesPerSubSeries == 1)
			{
				ePixelCounts.clear();
				ePixelCounts.resize(eAnalysisPixelLocations.size());
				eAtomPresent.clear();
				eAtomPresent.resize(eAnalysisPixelLocations.size());
			}
			// on the first accumulation only
			if (eCurrentThreadAccumulationNumber == 1)
			{
				allAtomSurvivalDataCount.resize(ePixelCounts.size());
				for (int pixelInc = 0; pixelInc < ePixelCounts.size(); pixelInc++)
				{
					allAtomSurvivalDataCount[pixelInc] = 1;
				}
			}

			// Load the relevant data into an array that sticks around until the next experiment. (gets reset above)
			for (unsigned int pixelInc = 0; pixelInc < ePixelCounts.size(); pixelInc++)
			{
				int secondIndex = eAnalysisPixelLocations[pixelInc].first - 1 + eImageWidth * (eAnalysisPixelLocations[pixelInc].second - 1);
				ePixelCounts[pixelInc].push_back(eImageVecQueue[0][secondIndex]);
			}
			// any of the following requires information about whether atoms were present or not.
			if (ePlotAtomLoading || ePlotAtomSurvival || ePlotTunneling || ePlotTwoParticleLoss || ePlotRunningAverageSurvival)
			{
				// Create a data point for each pixel for each picture.
				for (unsigned int pixelInc = 0; pixelInc < eAtomPresent.size(); pixelInc++) 
				{
					// how many data points
					// eAtomPresent[pixelInc].resize(eTotalNumberOfPicturesInSeries);
					// check the most recent data point.
					if (ePixelCounts[pixelInc].back() > eDetectionThreshold) 
					{
						eAtomPresent[pixelInc].push_back(true);
					}
					else 
					{
						eAtomPresent[pixelInc].push_back(false);
					}
				}
			}
			if ((eCurrentThreadAccumulationNumber % ePicturesPerSubSeries) % ePlottingFrequency != 0)
			{
				DWORD mutexMsg = WaitForSingleObject(ePlottingMutex, INFINITE);
				switch (mutexMsg)
				{
					case WAIT_OBJECT_0:
					{
						// mutex is available.
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
				continue;
			}
			/// Plot the Average number of counts in each picture
			if (ePlotAverageCounts) 
			{
				// If first picture of a stack...
				if (eCurrentThreadAccumulationNumber % ePicturesPerSubSeries == 1 || mostRecentlyActive[0] == false)
				{
					// mark this as having been active.
					mostRecentlyActive[0] = true;
					// no other clears needed since this option only takes 
					// one for each pair and each direction that the pair can tunnel in.
					eNewAveragePixelsDataFlag.resize(ePixelCounts.size());
					// fill with true.
					std::fill(eNewAveragePixelsDataFlag.begin(), eNewAveragePixelsDataFlag.end(), true);
				}

				// Compute Averages
				std::vector<double> sums, means, squaredSums, standardDeviations, xVals;;

				sums.resize(ePixelCounts.size());
				means.resize(ePixelCounts.size());
				squaredSums.resize(ePixelCounts.size());
				standardDeviations.resize(ePixelCounts.size());

				eAveragesAveragePixels.resize(ePixelCounts.size());
				eStandardDeviationsAveragePixels.resize(ePixelCounts.size());
				eXValsAveragePixels.resize(ePixelCounts.size());
				double spacingFactor = 0.2;
				double position;

				for (unsigned int pixelInc = 0; pixelInc < ePixelCounts.size(); pixelInc++) 
				{
					if (eNewAveragePixelsDataFlag[pixelInc] == true) 
					{
						// only update this size if there is data.
						eAveragesAveragePixels[pixelInc].resize(eAveragesAveragePixels[pixelInc].size() + 1);
						eStandardDeviationsAveragePixels[pixelInc].resize(eStandardDeviationsAveragePixels[pixelInc].size() + 1);
						// integer division here.
						position = (eCurrentThreadAccumulationNumber - 1) / ePicturesPerSubSeries + 1 - (spacingFactor * (pixelInc) / (ePixelCounts.size())
							- spacingFactor * 0.5 + spacingFactor * 0.5 / (ePixelCounts.size()));
						eXValsAveragePixels[pixelInc].push_back(position);
						// set the flag to not do this again before this array gets reset at beginning of the next accumulation stack.
						eNewAveragePixelsDataFlag[pixelInc] = false;
					}
					sums[pixelInc] = std::accumulate(ePixelCounts[pixelInc].begin(), ePixelCounts[pixelInc].end(), 0.0);
					means[pixelInc] = sums[pixelInc] / ePixelCounts[pixelInc].size();
					squaredSums[pixelInc] = std::inner_product(ePixelCounts[pixelInc].begin(), ePixelCounts[pixelInc].end(), ePixelCounts[pixelInc].begin(), 0.0);
					standardDeviations[pixelInc] = std::sqrt(squaredSums[pixelInc] / ePixelCounts[pixelInc].size() - means[pixelInc] * means[pixelInc]);
					eAveragesAveragePixels[pixelInc].back() = (means[pixelInc]);
					eStandardDeviationsAveragePixels[pixelInc].back() = (standardDeviations[pixelInc]);
				}
				ePlotter << "set terminal wxt 0 title \"Average Counts\" noraise\n";
				ePlotter << "set format y \"%.0f\"\n";
				ePlotter << "set yrange [95:105]\n";
				// set terminal wxt 0 title "Average Counts" noraise position
				// plot '-' using 1:2:3 with yerrorbars title "Pixel Number 0" lt rgb "black", '-' using 1:2:3 with yerrorbars title "Pixel Number 1" lt rgb "blue",

				ePlotter << "set title \"Average Pixel Counts Data\"\n";
				ePlotter << "set xlabel \"Experiment Number\"\n";
				ePlotter << "set ylabel \"Average Count\"\n";
				// set the command.
				std::string gnuCommand = "plot";
				for (unsigned int pixelInc = 0; pixelInc < ePixelCounts.size(); pixelInc++) 
				{
					gnuCommand += " '-' using 1:2:3 with yerrorbars title \"Pixel Number " + std::to_string(pixelInc)  + "\" " + GNU_PLOT_COLORS[pixelInc] + ",";
				}
				gnuCommand += "\n";
				ePlotter << gnuCommand;
				//
				std::vector<double> testVec1, testVec2, testVec3;
				testVec1.push_back(1);
				testVec2.push_back(2);
				testVec3.push_back(1);
				for (unsigned int pixelInc = 0; pixelInc < ePixelCounts.size(); pixelInc++) 
				{
					ePlotter.send1d(boost::make_tuple(testVec1, testVec2, testVec3));
					//ePlotter.send1d(boost::make_tuple(eXValsAveragePixels[pixelInc], eAveragesAveragePixels[pixelInc], eStandardDeviationsAveragePixels[pixelInc]));
				}
			}
			else if (mostRecentlyActive[0] == true)
			{
				// then it just got turned off.
				mostRecentlyActive[0] = false;
			}
			/// Plot a histogram of the count values. When atoms are present, this should look like two gaussians. 
			if (ePlotCountHistogram) 
			{
				//
				ePlotter << "set terminal wxt 1 title \"Histogram\" noraise\n";
				ePlotter << "set format y \"%.1f\"\n";
				ePlotter << "set autoscale x\n";
				ePlotter << "set yrange [0:*]\n";
				ePlotter << "set xlabel \"Count #\"\n";
				ePlotter << "set ylabel \"Occurrence Count\"\n";
				double spaceFactor = 0.8;
				double boxWidth = spaceFactor / eAnalysisPixelLocations.size();
				ePlotter << "set boxwidth " + std::to_string(boxWidth) + "\n";
				ePlotter << "set style fill solid 1\n";				std::string binWidth;
				// leave 0.2 pixels worth of space in between the bins.
				std::string gnuCommand = "plot";
				
				for (unsigned int pixelInc = 0; pixelInc < ePixelCounts.size(); pixelInc++) 
				{
					std::string singleHist = " '-' using ($1 - " + std::to_string(boxWidth * pixelInc - spaceFactor * 0.5 + spaceFactor * 0.5 / ePixelCounts.size())
						+ ") : (1.0) smooth freq with boxes title \"Pixel Number " + "\" " + GNU_PLOT_COLORS[pixelInc] + ",";
					gnuCommand += singleHist;
				}
				gnuCommand += "\n";
				ePlotter << gnuCommand;
				for (unsigned int pixelInc = 0; pixelInc < ePixelCounts.size(); pixelInc++) 
				{
					ePlotter.send1d(ePixelCounts[pixelInc]);
				}
			}
			else if (mostRecentlyActive[1] == true)
			{
				// then it just got turned off.
				mostRecentlyActive[1] = false;
			}
			/// Plot Whether an Atom was Present during a run or not.
			if (ePlotAtomLoading) 
			{
				
				if (eCurrentThreadAccumulationNumber % ePicturesPerExperiment == 0)
				{
					if (eCurrentThreadAccumulationNumber % ePicturesPerSubSeries == ePicturesPerExperiment || mostRecentlyActive[2] == false)
					{
						mostRecentlyActive[2] = true;
						// make sure array is correct size (slightly redundant, should only need to be done once).
						eNewAtomPrecenseDataFlag.resize(ePixelCounts.size());
						// fill with true.
						std::fill(eNewAtomPrecenseDataFlag.begin(), eNewAtomPrecenseDataFlag.end(), true);
					}
					// Compute Averages
					std::vector<double> sums, means, squaredSums, standardDeviations, xVals;;

					sums.resize(eAtomPresent.size());
					means.resize(eAtomPresent.size());
					squaredSums.resize(eAtomPresent.size());
					standardDeviations.resize(eAtomPresent.size());
					//		averagesVec.resize(eAtomPresent.size());
					//		standardDeviationsVec.resize(eAtomPresent.size());

					eAveragesAtomPrecense.resize(ePixelCounts.size());
					eStandardDeviationsAtomPrecense.resize(ePixelCounts.size());
					eXValsAtomPrecense.resize(ePixelCounts.size());
					double spacingFactor = 0.2;
					double position;

					for (unsigned int pixelInc = 0; pixelInc < eAtomPresent.size(); pixelInc++) 
					{
						if (eNewAtomPrecenseDataFlag[pixelInc] == true) 
						{
							// only update this size if there is data.
							eAveragesAtomPrecense[pixelInc].resize(eAveragesAtomPrecense[pixelInc].size() + 1);
							eStandardDeviationsAtomPrecense[pixelInc].resize(eStandardDeviationsAtomPrecense[pixelInc].size() + 1);
							position = (eCurrentThreadAccumulationNumber - 1) / ePicturesPerSubSeries + 1 - (spacingFactor * (pixelInc) / (ePixelCounts.size())
								- spacingFactor * 0.5 + spacingFactor * 0.5 / (ePixelCounts.size()));
							eXValsAtomPrecense[pixelInc].push_back(position);
							// set the flag to not do this again before this array gets reset at beginning of the next accumulation stack.
							eNewAtomPrecenseDataFlag[pixelInc] = false;
						}
						sums[pixelInc] = std::accumulate(eAtomPresent[pixelInc].begin(), eAtomPresent[pixelInc].end(), 0.0);
						means[pixelInc] = sums[pixelInc] / eAtomPresent[pixelInc].size();
						squaredSums[pixelInc] = std::inner_product(eAtomPresent[pixelInc].begin(), eAtomPresent[pixelInc].end(), eAtomPresent[pixelInc].begin(), 0.0);
						standardDeviations[pixelInc] = std::sqrt(squaredSums[pixelInc] / eAtomPresent[pixelInc].size() - means[pixelInc] * means[pixelInc]);

						eAveragesAtomPrecense[pixelInc].back() = means[pixelInc];
						eStandardDeviationsAtomPrecense[pixelInc].back() = standardDeviations[pixelInc];
					}
					ePlotter << "set terminal wxt 2 title \"Loading\" noraise\n";
					ePlotter << "set format y \"%f\"\n";
					ePlotter << "set yrange [-0.1:1.1]\n";
					ePlotter << "set title \"Atom Loading Data\"\n";
					ePlotter << "set xlabel \"Experiment #\"\n";
					ePlotter << "set ylabel \"Presence Success Rate\"\n";
					// set the command.
					std::string gnuCommand = "plot";
					for (unsigned int pixelInc = 0; pixelInc < ePixelCounts.size(); pixelInc++) 
					{
						gnuCommand += " '-' using 1:2:3 with yerrorbars title \"Pixel Number " + std::to_string(pixelInc) + "\" " + GNU_PLOT_COLORS[pixelInc] + ",";
					}
					gnuCommand += "\n";
					ePlotter << gnuCommand;
					//		ePlotter << "plot '-' using 1:2:3 with yerrorbars, '-' using 1:2:3 with yerrorbars,\n";
					for (unsigned int pixelInc = 0; pixelInc < ePixelCounts.size(); pixelInc++) 
					{
						ePlotter.send1d(boost::make_tuple(eXValsAtomPrecense[pixelInc], eAveragesAtomPrecense[pixelInc], eStandardDeviationsAtomPrecense[pixelInc]));
					}
				}
			}
			else if (mostRecentlyActive[2] == true)
			{
				// then it just got turned off.
				mostRecentlyActive[2] = false;
			}
			///
			if (ePlotAtomSurvival) 
			{
				// If first picture of an accumulation...
				if (eCurrentThreadAccumulationNumber % ePicturesPerSubSeries == 2 || mostRecentlyActive[3] == false)
				{
					mostRecentlyActive[3] = true;
					// reset the data variables
					eAtomSurvival.clear();
					eAtomSurvival.resize(eAnalysisPixelLocations.size());
					// one for each pair and each direction that the pair can tunnel in.
					eNewAtomSurvivalDataFlag.resize(eAnalysisPixelLocations.size());
					// fill with true.

					std::fill(eNewAtomSurvivalDataFlag.begin(), eNewAtomSurvivalDataFlag.end(), true);
				}

				// Every other run, calculate whether a particle survived.
				if (eAtomPresent.size() != 0) 
				{
					// if even run
					if (eAtomPresent[0].size() % 2 == 0) 
					{
						// for all pixels, test if the atom survived.
						eAtomSurvival.resize(eAnalysisPixelLocations.size());
						for (unsigned int pixelInc = 0; pixelInc < eAtomSurvival.size(); pixelInc++) 
						{
							int last = eAtomPresent[pixelInc].size() - 1;
							// if atom was there before... and is also there now,
							if (eAtomPresent[pixelInc][last - 1]) 
							{
								// and is also there now...
								if (eAtomPresent[pixelInc][last]) 
								{
									eAtomSurvival[pixelInc].push_back(true);
								}
								else 
								{
									eAtomSurvival[pixelInc].push_back(false);
								}
							}
							else 
							{
								// atom wasn't there to begin with. No data to be taken for this run.
							}
						}
						// Compute Averages
						std::vector<double> sums, means, squaredSums, standardDeviations;;
						//std::vector<std::vector<double> > averagesVec, standardDeviationsVec;

						sums.resize(eAtomSurvival.size());
						means.resize(eAtomSurvival.size());
						squaredSums.resize(eAtomSurvival.size());
						standardDeviations.resize(eAtomSurvival.size());
						eAveragesAtomSurvival.resize(eAtomSurvival.size());
						eStandardDeviationsAtomSurvival.resize(eAtomSurvival.size());

						double spacingFactor = 0.2;
						double position;
						eXValsAtomSurvival.resize(eAtomSurvival.size());

						for (unsigned int pixelInc = 0; pixelInc < eAtomSurvival.size(); pixelInc++) 
						{
							if (eNewAtomSurvivalDataFlag[pixelInc] == true) 
							{
								// only update this size if there is data.
								eAveragesAtomSurvival[pixelInc].resize(eAveragesAtomSurvival[pixelInc].size() + 1);
								eStandardDeviationsAtomSurvival[pixelInc].resize(eStandardDeviationsAtomSurvival[pixelInc].size() + 1);
								position = (eCurrentThreadAccumulationNumber - 1) / ePicturesPerSubSeries + 1 - (spacingFactor * (pixelInc) / (ePixelCounts.size())
									- spacingFactor * 0.5 + spacingFactor * 0.5 / (ePixelCounts.size()));
								eXValsAtomSurvival[pixelInc].push_back(position);
								// set the flag to not do this again before this array gets reset at beginning of the next accumulation stack.
								eNewAtomSurvivalDataFlag[pixelInc] = false;
							}
							sums[pixelInc] = std::accumulate(eAtomSurvival[pixelInc].begin(), eAtomSurvival[pixelInc].end(), 0.0);
							means[pixelInc] = sums[pixelInc] / eAtomSurvival[pixelInc].size();
							squaredSums[pixelInc] = std::inner_product(eAtomSurvival[pixelInc].begin(), eAtomSurvival[pixelInc].end(), eAtomSurvival[pixelInc].begin(), 0.0);
							standardDeviations[pixelInc] = std::sqrt(squaredSums[pixelInc] / eAtomSurvival[pixelInc].size() - means[pixelInc] * means[pixelInc]);

							eAveragesAtomSurvival[pixelInc].back() = means[pixelInc];
							eStandardDeviationsAtomSurvival[pixelInc].back() = standardDeviations[pixelInc];
						}
						
						ePlotter << "set terminal wxt 3 title \"Survival\" noraise\n";
						ePlotter << "set format y \"%.1f\"\n";
						ePlotter << "set yrange [-0.1:1.1]\n";
						ePlotter << "set title \"Atom Survival Data\"\n";
						ePlotter << "set xlabel \"Experiment #\"\n";
						// set the command.
						ePlotter << "set ylabel \"Survival Success Rate\"\n";						
						std::string gnuCommand = "plot";
						for (unsigned int pixelInc = 0; pixelInc < ePixelCounts.size(); pixelInc++) 
						{
							gnuCommand += " '-' using 1:2:3 with yerrorbars " + GNU_PLOT_COLORS[pixelInc] + " title \"Pixel Number " + std::to_string(pixelInc) + "\",";
						}
						gnuCommand += "\n";
						ePlotter << gnuCommand;
						for (unsigned int pixelInc = 0; pixelInc < ePixelCounts.size(); pixelInc++) {
							ePlotter.send1d(boost::make_tuple(eXValsAtomSurvival[pixelInc], eAveragesAtomSurvival[pixelInc], eStandardDeviationsAtomSurvival[pixelInc]));
						}
					}
				}
			}
			else if (mostRecentlyActive[3] == true)
			{
				// then it just got turned off.
				mostRecentlyActive[3] = false;
			}
			///
			if (ePlotTwoParticleLoss) 
			{
				// Every other run, calculate whether a particle survived.
				// if even run
				if (eAtomPresent[0].size() % 2 == 0) 
				{
					if (eCurrentThreadAccumulationNumber % ePicturesPerSubSeries == 2 || mostRecentlyActive[4] == false)
					{
						mostRecentlyActive[4] = true;
						eTwoParticleLoss.clear();
						eTwoParticleLoss.resize(eTunnelingPairs.size());
						// one for each pair and each direction that the pair can tunnel in.
						eNewTwoParticleLossDataFlag.resize(eTwoParticleLoss.size());
						// fill with true.
						std::fill(eNewTwoParticleLossDataFlag.begin(), eNewTwoParticleLossDataFlag.end(), true);
					}
					for (unsigned int pairInc = 0; pairInc < eTwoParticleLoss.size(); pairInc++) 
					{
						int last = eAtomPresent[0].size() - 1;
						// if started with two particles...
						if ((eAtomPresent[eTunnelingPairs[pairInc].first - 1][last - 1] == true) && (eAtomPresent[eTunnelingPairs[pairInc].second - 1][last - 1] == true)) 
						{
							// if didn't end with two particles
							if ((eAtomPresent[eTunnelingPairs[pairInc].first - 1][last] == false) || (eAtomPresent[eTunnelingPairs[pairInc].second - 1][last] == false)) 
							{
								eTwoParticleLoss[pairInc].push_back(true);
							}
							else 
							{
								eTwoParticleLoss[pairInc].push_back(false);
							}
						}
					}
					bool dataPresent = false;
					for (unsigned int dataInc = 0; dataInc < eTwoParticleLoss.size(); dataInc++) 
					{
						if (eTwoParticleLoss[dataInc].size() > 0) 
						{
							dataPresent = true;
						}
					}
					if (dataPresent) 
					{
						// Compute Averages
						std::vector<double> sums, means, squaredSums, standardDeviations;
						// size is number of pairs.
						sums.resize(eTwoParticleLoss.size());
						means.resize(eTwoParticleLoss.size());
						squaredSums.resize(eTwoParticleLoss.size());
						standardDeviations.resize(eTwoParticleLoss.size());

						// These four lines are redundant. It only needs to be done once for these.
						eAveragesTwoParticleLoss.resize(eTwoParticleLoss.size());
						eStandardDeviationsTwoParticleLoss.resize(eTwoParticleLoss.size());

						// redundant 2 lines.
						eXValsTwoParticleLoss.resize(eTwoParticleLoss.size());
						double spacingFactor = 0.2;
						double position;
						// for all pairs...
						for (unsigned int pairInc = 0; pairInc < eTwoParticleLoss.size(); pairInc++) 
						{
							// Current number of stacks processed: (eCurrentThreadAccumulationNumber - 1) / (eTotalNumberOfPicturesInSeries / ePictureSubSeriesNumber) + 1
							// these 4 lines are also somewhat redundant.
							// handle for both forward and backwards directions.
							if (eTwoParticleLoss[pairInc].size() > 0) 
							{
								if (eNewTwoParticleLossDataFlag[pairInc] == true) 
								{
									// only update this size if there is data.
									eAveragesTwoParticleLoss[pairInc].resize(eAveragesTwoParticleLoss[pairInc].size() + 1);
									eStandardDeviationsTwoParticleLoss[pairInc].resize(eStandardDeviationsTwoParticleLoss[pairInc].size() + 1);
									position = (eCurrentThreadAccumulationNumber - 1) / ePicturesPerSubSeries + 1 - (spacingFactor * (pairInc) / (eTwoParticleLoss.size())
										- spacingFactor * 0.5 + spacingFactor * 0.5 / (eTwoParticleLoss.size()));
									eXValsTwoParticleLoss[pairInc].push_back(position);
									// set the flag to not do this again before this array gets reset at beginning of the next accumulation stack.
									eNewTwoParticleLossDataFlag[pairInc] = false;
								}
								// forward direction:
								sums[pairInc] = std::accumulate(eTwoParticleLoss[pairInc].begin(), eTwoParticleLoss[pairInc].end(), 0.0);
								means[pairInc] = sums[pairInc] / eTwoParticleLoss[pairInc].size();
								squaredSums[pairInc] = std::inner_product(eTwoParticleLoss[pairInc].begin(), eTwoParticleLoss[pairInc].end(),
									eTwoParticleLoss[pairInc].begin(), 0.0);
								standardDeviations[pairInc] = std::sqrt(squaredSums[pairInc] / eTwoParticleLoss[pairInc].size()
									- means[pairInc] * means[pairInc]);
								eAveragesTwoParticleLoss[pairInc].back() = means[pairInc];
								eStandardDeviationsTwoParticleLoss[pairInc].back() = standardDeviations[pairInc];
							}
						}
						// set the terminal for this guy.
						ePlotter << "set terminal wxt 4 title \"Two Particle Loss\" noraise\n";
						ePlotter << "set format y \"%.1f\"\n";
						ePlotter << "set yrange [-0.01:1.01]\n";
						ePlotter << "set title \"Two Particle Loss\"\n";
						ePlotter << "set xlabel \"Experiment Number\"\n";
						ePlotter << "set ylabel \"Two Particle Dual-Survival Rate\"\n";
						// set the command.
						std::string gnuCommand = "plot";
						for (unsigned int pairInc = 0; pairInc < eTwoParticleLoss.size(); pairInc++) 
						{
							// 2 X because there are two data sets (forward and backwards) for each pair.
							if (eAveragesTwoParticleLoss[pairInc].size() > 0) 
							{
								gnuCommand += " '-' using 1:2:3 with yerrorbars title \"Pair " + std::to_string(pairInc) + "\" " + GNU_PLOT_COLORS[pairInc] + ",";
							}
						}
						gnuCommand += "\n";
						ePlotter << gnuCommand;
						for (unsigned int pairInc = 0; pairInc < eTwoParticleLoss.size(); pairInc++) 
						{
							if (eAveragesTwoParticleLoss[pairInc].size() > 0) 
							{
								ePlotter.send1d(boost::make_tuple(eXValsTwoParticleLoss[pairInc], eAveragesTwoParticleLoss[pairInc], eStandardDeviationsTwoParticleLoss[pairInc]));
							}
						}
					}
				}
			}
			else if (mostRecentlyActive[4] == true)
			{
				// then it just got turned off.
				mostRecentlyActive[4] = false;
			}
			///
			if (ePlotTunneling) 
			{
				// Every other run, calculate whether a particle survived.
				// if even run
				if (eAtomPresent[0].size() % 2 == 0) 
				{
					// if the first picture of a stack, erase the previous data of the stack.
					if (eCurrentThreadAccumulationNumber % ePicturesPerSubSeries == 2 || mostRecentlyActive[5] == false)
					{
						mostRecentlyActive[5] = true;
						eAtomTunneled.clear();
						eAtomTunneled.resize(eTunnelingPairs.size());
						// one for each pair and each direction that the pair can tunnel in.
						eNewTunnelDataFlag.resize(eAtomTunneled.size() * 2);
						// fill with true.

						std::fill(eNewTunnelDataFlag.begin(), eNewTunnelDataFlag.end(), true);
					}
					// for all pairs of pixels, test the two ways that the atom could have tunneled.
					// for all pairs...
					for (unsigned int pairInc = 0; pairInc < eAtomTunneled.size(); pairInc++) 
					{
						int last = eAtomPresent[0].size() - 1;
						// if there was an atom in the first of the pair of pixels in the first pic and NOT the second......
						if ((eAtomPresent[eTunnelingPairs[pairInc].first - 1][last - 1] == true) && (eAtomPresent[eTunnelingPairs[pairInc].second - 1][last - 1] == false)) 
						{
							// if there was an atom in the second of the pair in the second pic...
							if (eAtomPresent[eTunnelingPairs[pairInc].second - 1][last] == true) 
							{
								// then tunneled. 0 corresponds to the forward direction.
								eAtomTunneled[pairInc][0].push_back(true);
							}
							else 
							{
								// Didn't tunnel. 0 corresponds to the forward direction..
								eAtomTunneled[pairInc][0].push_back(false);
							}
						}
						// if there was an atom in the second of the pair of pixels in the first pic...
						else if ((eAtomPresent[eTunnelingPairs[pairInc].second - 1][last - 1] == true) && (eAtomPresent[eTunnelingPairs[pairInc].first - 1][last - 1] == false)) 
						{
							// if there was an atom in the first of the pair in the second pic...
							if (eAtomPresent[eTunnelingPairs[pairInc].first - 1][last] == true) 
							{
								// then tunneled. The 1 corresponds to the backwards direction.
								eAtomTunneled[pairInc][1].push_back(true);
							}
							else 
							{
								// Didn't tunnel. The 1 corresponds to the backwards direction.
								eAtomTunneled[pairInc][1].push_back(false);
							}
						}
					}
					// Check if there is data to plot
					bool dataPresent = false;
					for (unsigned int dataInc = 0; dataInc < eAtomTunneled.size(); dataInc++) 
					{
						if (eAtomTunneled[dataInc][0].size() > 0 || eAtomTunneled[dataInc][1].size() > 0) 
						{
							dataPresent = true;
						}
					}
					// if there is data to plot...
					if (dataPresent) 
					{
						// Compute Averages
						std::vector<double> sumsForward, meansForward, squaredSumsForward, standardDeviationsForward;
						std::vector<double> sumsBackward, meansBackward, squaredSumsBackward, standardDeviationsBackward;
						// size is number of pairs.
						sumsForward.resize(eAtomTunneled.size());
						meansForward.resize(eAtomTunneled.size());
						squaredSumsForward.resize(eAtomTunneled.size());
						standardDeviationsForward.resize(eAtomTunneled.size());

						sumsBackward.resize(eAtomTunneled.size());
						meansBackward.resize(eAtomTunneled.size());
						squaredSumsBackward.resize(eAtomTunneled.size());
						standardDeviationsBackward.resize(eAtomTunneled.size());
						// These four lines are redundant. It only needs to be done once for these.
						eAveragesVecTunnelForward.resize(eAtomTunneled.size());
						eStandardDeviationsVecTunnelForward.resize(eAtomTunneled.size());
						eAveragesVecTunnelBackward.resize(eAtomTunneled.size());
						eStandardDeviationsVecTunnelBackward.resize(eAtomTunneled.size());

						// redundant 2 lines.
						eXValsTunnelForward.resize(eAtomTunneled.size());
						eXValsTunnelBackward.resize(eAtomTunneled.size());
						double spacingFactor = 0.2;
						double position;
						// for all pairs...
						for (unsigned int pairInc = 0; pairInc < eAtomTunneled.size(); pairInc++) 
						{
							// Current number of stacks processed: (eCurrentThreadAccumulationNumber - 1) / (eTotalNumberOfPicturesInSeries / ePictureSubSeriesNumber) + 1
							// these 4 lines are also somewhat redundant.
							// handle for both forward and backwards directions.
							if (eAtomTunneled[pairInc][0].size() > 0) 
							{
								if (eNewTunnelDataFlag[2 * pairInc] == true) 
								{
									// only update this size if there is data.
									eAveragesVecTunnelForward[pairInc].resize(eAveragesVecTunnelForward[pairInc].size() + 1);
									eStandardDeviationsVecTunnelForward[pairInc].resize(eStandardDeviationsVecTunnelForward[pairInc].size() + 1);
									position = (eCurrentThreadAccumulationNumber - 1) / ePicturesPerSubSeries + 1 - (spacingFactor * (2 * pairInc) / (2 * eAtomTunneled.size())
										- spacingFactor * 0.5 + spacingFactor * 0.5 / (2 * eAtomTunneled.size()));
									eXValsTunnelForward[pairInc].push_back(position);
									// set the flag to not do this again before this array gets reset at beginning of the next accumulation stack.
									eNewTunnelDataFlag[2 * pairInc] = false;
								}
								// forward direction:
								sumsForward[pairInc] = std::accumulate(eAtomTunneled[pairInc][0].begin(), eAtomTunneled[pairInc][0].end(), 0.0);
								meansForward[pairInc] = sumsForward[pairInc] / eAtomTunneled[pairInc][0].size();
								squaredSumsForward[pairInc] = std::inner_product(eAtomTunneled[pairInc][0].begin(), eAtomTunneled[pairInc][0].end(),
									eAtomTunneled[pairInc][0].begin(), 0.0);
								standardDeviationsForward[pairInc] = std::sqrt(squaredSumsForward[pairInc] / eAtomTunneled[pairInc][0].size()
									- meansForward[pairInc] * meansForward[pairInc]);
								eAveragesVecTunnelForward[pairInc][eAveragesVecTunnelForward[pairInc].size() - 1] = meansForward[pairInc];
								eStandardDeviationsVecTunnelForward[pairInc][eStandardDeviationsVecTunnelForward[pairInc].size() - 1] = standardDeviationsForward[pairInc];
							}
							if (eAtomTunneled[pairInc][1].size() > 0) 
							{
								if (eNewTunnelDataFlag[2 * pairInc + 1] == true) 
								{
									// only update this size if there is data. Also, this is redundant.
									eAveragesVecTunnelBackward[pairInc].resize(eAveragesVecTunnelBackward[pairInc].size() + 1);
									eStandardDeviationsVecTunnelBackward[pairInc].resize(eStandardDeviationsVecTunnelBackward[pairInc].size() + 1);
									position = (eCurrentThreadAccumulationNumber - 1) / ePicturesPerSubSeries + 1 - (spacingFactor * (2 * pairInc + 1) / (2 * eAtomTunneled.size())
										- spacingFactor * 0.5 + spacingFactor * 0.5 / (2 * eAtomTunneled.size()));
									eXValsTunnelBackward[pairInc].push_back(position);
									// set the flag to not do this again before this array gets reset at beginning of the next accumulation stack.
									eNewTunnelDataFlag[2 * pairInc + 1] = false;

								}
								// backwards direction (all the [0] changed to [1] and suffixes changed.):
								sumsBackward[pairInc] = std::accumulate(eAtomTunneled[pairInc][1].begin(), eAtomTunneled[pairInc][1].end(), 0.0);
								meansBackward[pairInc] = sumsBackward[pairInc] / eAtomTunneled[pairInc][1].size();
								squaredSumsBackward[pairInc] = std::inner_product(eAtomTunneled[pairInc][1].begin(), eAtomTunneled[pairInc][1].end(), eAtomTunneled[pairInc][1].begin(), 0.0);
								standardDeviationsBackward[pairInc] = std::sqrt(squaredSumsBackward[pairInc] / eAtomTunneled[pairInc][1].size() - meansBackward[pairInc] * meansBackward[pairInc]);
								eAveragesVecTunnelBackward[pairInc][eAveragesVecTunnelBackward[pairInc].size() - 1] = meansBackward[pairInc];
								eStandardDeviationsVecTunnelBackward[pairInc][eStandardDeviationsVecTunnelBackward[pairInc].size() - 1] = standardDeviationsBackward[pairInc];
							}
						}
						// set the terminal for this guy.
						ePlotter << "set terminal wxt 5 title \"Tunneling\" noraise\n";
						ePlotter << "set format y \"%.1f\"\n";
						ePlotter << "set yrange [-0.01:1.01]\n";

						ePlotter << "set title \"Tunneling Data\"\n";
						ePlotter << "set xlabel \"Experiment Number\"\n";
						ePlotter << "set ylabel \"Tunneling Success Rate\"\n";
						// set the command.
						std::string gnuCommand = "plot";
						for (unsigned int pairInc = 0; pairInc < eAtomTunneled.size(); pairInc++) 
						{
							// 2 X because there are two data sets (forward and backwards) for each pair.
							if (eAveragesVecTunnelForward[pairInc].size() > 0) 
							{
								gnuCommand += " '-' using 1:2:3 with yerrorbars title \"Pair " + std::to_string(pairInc) + " Forward\" " + GNU_PLOT_COLORS[2 * pairInc] + ",";
							}
							if (eAveragesVecTunnelBackward[pairInc].size() > 0) {
								gnuCommand += " '-' using 1:2:3 with yerrorbars title \"Pair " + std::to_string(pairInc) + " Backward\" " + GNU_PLOT_COLORS[2 * pairInc + 1] + ",";
							}
						}
						gnuCommand += "\n";
						ePlotter << gnuCommand;
						for (unsigned int pairInc = 0; pairInc < eAtomTunneled.size(); pairInc++) 
						{
							if (eAveragesVecTunnelForward[pairInc].size() > 0) 
							{
								ePlotter.send1d(boost::make_tuple(eXValsTunnelForward[pairInc], eAveragesVecTunnelForward[pairInc], eStandardDeviationsVecTunnelForward[pairInc]));
							}
							if (eAveragesVecTunnelBackward[pairInc].size() > 0) 
							{
								ePlotter.send1d(boost::make_tuple(eXValsTunnelBackward[pairInc], eAveragesVecTunnelBackward[pairInc], eStandardDeviationsVecTunnelBackward[pairInc]));
							}
						}
					}
				}
			}
			else if (mostRecentlyActive[5] == true)
			{
				// then it just got turned off.
				mostRecentlyActive[5] = false;
			}
			/// PLOT RUNNING AVERAGE OF SURVIVALS
			if (ePlotRunningAverageSurvival)
			{
				/// Every other run, calculate whether a particle survived.
				if (eAtomPresent.size() != 0)
				{
					// if even run
					if (eAtomPresent[0].size() % 2 == 0)
					{
						// If first picture of an accumulation...
						if (mostRecentlyActive[6] == false)
						{
							mostRecentlyActive[6] = true;
							// reset the data variables
							//eAtomSurvival.clear();
							//eAtomSurvival.resize(eAnalysisPixelLocations.size());
							// one for each pair and each direction that the pair can tunnel in.
							eNewAtomSurvivalDataFlag.resize(eAnalysisPixelLocations.size());
							// fill with true.

							std::fill(eNewAtomSurvivalDataFlag.begin(), eNewAtomSurvivalDataFlag.end(), true);
						}
						// for all pixels, test if the atom survived.
						eAtomSurvival.resize(eAnalysisPixelLocations.size());
						eXValsAllAtomSurvival.resize(ePixelCounts.size());
						for (unsigned int pixelInc = 0; pixelInc < eAtomSurvival.size(); pixelInc++)
						{
							int last = eAtomPresent[pixelInc].size() - 1;
							// if atom was there before... and is also there now,
							if (eAtomPresent[pixelInc][last - 1])
							{
								// and is also there now...
								if (eAtomPresent[pixelInc][last])
								{
									eAtomSurvival[pixelInc].push_back(true);
								}
								else
								{
									eAtomSurvival[pixelInc].push_back(false);
								}
								eXValsAllAtomSurvival[pixelInc].push_back(eCurrentThreadAccumulationNumber / 2);
								eNewAtomSurvivalDataFlag[pixelInc] = true;
							}
							else
							{
								// atom wasn't there to begin with. No data to be taken for this run.
							}
						}
						
						/// Compute Averages
						std::vector<double> sums, means;

						sums.resize(eAtomSurvival.size());
						means.resize(eAtomSurvival.size());

						double position;


						for (unsigned int pixelInc = 0; pixelInc < eAtomSurvival.size(); pixelInc++)
						{
							if (eAtomSurvival[pixelInc].size() >= eNumberOfRunsToAverage)
							{
								eAveragesRunningAtomSurvival.resize(eAtomSurvival.size());
								eXValsRunningAtomSurvival.resize(eAtomSurvival.size());

								if (eNewAtomSurvivalDataFlag[pixelInc] == true)
								{
									// only update this size if there is data.
									eAveragesRunningAtomSurvival[pixelInc].resize(eAveragesRunningAtomSurvival[pixelInc].size() + 1);
									eXValsRunningAtomSurvival[pixelInc].resize(eXValsRunningAtomSurvival[pixelInc].size() + 1);
									// set the flag to not do this again before this array gets reset at beginning of the next accumulation stack.
									eNewAtomSurvivalDataFlag[pixelInc] = false;
								}
								sums[pixelInc] = std::accumulate(eAtomSurvival[pixelInc].end() - eNumberOfRunsToAverage, eAtomSurvival[pixelInc].end(), 0.0);
								means[pixelInc] = sums[pixelInc] / eNumberOfRunsToAverage;
								position = std::accumulate(eXValsAllAtomSurvival[pixelInc].end() - eNumberOfRunsToAverage, eXValsAllAtomSurvival[pixelInc].end(), 0.0) / eNumberOfRunsToAverage;
								eAveragesRunningAtomSurvival[pixelInc].back() = means[pixelInc];
								eXValsRunningAtomSurvival[pixelInc].back() = position;
							}
						}

						
						ePlotter << "set terminal wxt 6 title \"Running Survival\" noraise\n";
						ePlotter << "set format y \"%.1f\"\n";
						ePlotter << "set yrange [-0.1:1.1]\n";
						ePlotter << "set title \"Running Atom Survival Data\"\n";
						ePlotter << "set xlabel \"Trial #\"\n";
						ePlotter << "set ylabel \"Survival\"\n";

						std::string gnuCommand = "plot";
						for (unsigned int pixelInc = 0; pixelInc < ePixelCounts.size(); pixelInc++)
						{
							gnuCommand += " '-' using 1:2 " + GNU_PLOT_COLORS[pixelInc] + " title \"Pixel Number " + std::to_string(pixelInc) + "\",";
						}
						for (unsigned int pixelInc = 0; pixelInc < ePixelCounts.size(); pixelInc++)
						{
							if (eAtomSurvival[pixelInc].size() >= eNumberOfRunsToAverage)
							{
								gnuCommand += " '-' using 1:2 " + GNU_PLOT_COLORS[pixelInc] + " with lines title \"Pixel Number " + std::to_string(pixelInc)
									+ "\",";
							}
						}
						gnuCommand += "\n";
						ePlotter << gnuCommand;
						for (unsigned int pixelInc = 0; pixelInc < ePixelCounts.size(); pixelInc++) 
						{
							ePlotter.send1d(boost::make_tuple(eXValsAllAtomSurvival[pixelInc], eAtomSurvival[pixelInc]));
						}
						for (unsigned int pixelInc = 0; pixelInc < ePixelCounts.size(); pixelInc++)
						{
							if (eAtomSurvival[pixelInc].size() >= eNumberOfRunsToAverage)
							{
								ePlotter.send1d(boost::make_tuple(eXValsRunningAtomSurvival[pixelInc], eAveragesRunningAtomSurvival[pixelInc]));
							}
						}
						ePlotter << gnuCommand;

					}
				}
			}
			else if (mostRecentlyActive[6] == true)
			{
				// then it just got turned off.
				mostRecentlyActive[6] = false;
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
	return TRUE;
}