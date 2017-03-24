#include "stdafx.h"
#include "ExperimentTimer.h"
#include "externals.h"
#include "constants.h"
#include "Commctrl.h"
#include "Uxtheme.h"
#include <vector>
#include "reorganizeControl.h"
#include "fonts.h"
#include "CameraWindow.h"


void ExperimentTimer::initialize( cameraPositions& inputLoc, CWnd* parent, bool isTriggerModeSensitive, int& id,
								  std::unordered_map<std::string, CFont*> fonts, std::vector<CToolTipCtrl*> toolTips )
{
	timeDisplay.seriesPos = { inputLoc.seriesPos.x, inputLoc.seriesPos.y, inputLoc.seriesPos.x + 168, inputLoc.seriesPos.y + 40 };
	timeDisplay.amPos = { inputLoc.amPos.x, inputLoc.amPos.y, inputLoc.amPos.x + 168, inputLoc.amPos.y + 40 };
	timeDisplay.videoPos = { inputLoc.videoPos.x, inputLoc.videoPos.y, inputLoc.videoPos.x + 168, inputLoc.videoPos.y + 40 };
	timeDisplay.ID = id++;
	timeDisplay.Create("", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, timeDisplay.seriesPos, parent, timeDisplay.ID);
	timeDisplay.fontType = "Normal";
	/// PROGRESS BARS
	// subseries progress bar
	variationProgress.seriesPos = { inputLoc.seriesPos.x + 168, inputLoc.seriesPos.y, inputLoc.seriesPos.x + 1168, inputLoc.seriesPos.y + 15 };
	variationProgress.amPos = { inputLoc.amPos.x + 168, inputLoc.amPos.y, inputLoc.amPos.x + 1168, inputLoc.amPos.y + 15 };
	variationProgress.videoPos = { inputLoc.videoPos.x + 168, inputLoc.videoPos.y, inputLoc.videoPos.x + 1168, 
		inputLoc.videoPos.y + 15 };
	variationProgress.ID = id++;
	variationProgress.Create(WS_CHILD | WS_VISIBLE, variationProgress.seriesPos, parent, variationProgress.ID);
	variationProgress.SetBkColor(RGB(100, 110, 100));
	variationProgress.SetBarColor(RGB(0, 200, 0));
	// series progress bar display
	overallProgress.seriesPos = { inputLoc.seriesPos.x + 168, inputLoc.seriesPos.y + 15, inputLoc.seriesPos.x + 1168, inputLoc.seriesPos.y + 40 };
	overallProgress.amPos = { inputLoc.amPos.x + 168, inputLoc.amPos.y + 15, inputLoc.amPos.x + 1168,
		inputLoc.amPos.y + 40 };
	overallProgress.videoPos = { inputLoc.videoPos.x + 168, inputLoc.videoPos.y + 15, inputLoc.videoPos.x + 1168,
		inputLoc.videoPos.y + 40 };
	overallProgress.ID = id++;
	overallProgress.Create(WS_CHILD | WS_VISIBLE, overallProgress.seriesPos, parent, overallProgress.ID);
	overallProgress.SetBkColor(RGB(100, 110, 100));
	overallProgress.SetBarColor(RGB(255, 255, 255));	
	inputLoc.seriesPos.y += 40;
	inputLoc.amPos.y += 40;
	inputLoc.videoPos.y += 40;
}

void ExperimentTimer::update(int currentRepNumber, int repsPerVariation, int numberOfVariations, int picsPerRep)
{
	int totalRepetitions = repsPerVariation * numberOfVariations;
	int minAverageNumber = 10;
	int variationPosition = (currentRepNumber % repsPerVariation) * 100.0 / repsPerVariation;
	int overallPosition = currentRepNumber / (double)totalRepetitions * 100;
	variationProgress.SetPos ( variationPosition );
	overallProgress.SetPos( overallPosition );
	if (currentRepNumber == 1)
	{
		this->firstTime = GetTickCount64();
		timeDisplay.fontType = "Normal";
		timeDisplay.SetWindowTextA( "Estimating Time..." );
		timeDisplay.RedrawWindow();
	}
	else if (currentRepNumber < totalRepetitions)
	{
		if (currentRepNumber == minAverageNumber)
		{
			timeDisplay.fontType = "Large";
			timeDisplay.RedrawWindow();
		}
		long long thisTime = GetTickCount64();
		if (currentRepNumber % picsPerRep == 0)
		{
			double averageTime = (thisTime - firstTime) / currentRepNumber;
			// in seconds...
			int timeLeft = (repsPerVariation * numberOfVariations - currentRepNumber) * averageTime / 1000;
			int hours = timeLeft / 3600;
			int minutes = (timeLeft % 3600) / 60;
			int seconds = (timeLeft % 3600) % 60;
			std::string timeString = "";
			timeString += std::to_string(hours) + ":";
			if (minutes < 10)
			{
				timeString += "0" + std::to_string(minutes);
			}
			else
			{
				timeString += std::to_string(minutes);
			}
			if (hours == 0 && minutes < 5)
			{
				if (seconds < 10)
				{
					timeString += ":0" + std::to_string(seconds);
				}
				else
				{
					timeString += ":" + std::to_string(seconds);
				}
			}
			timeDisplay.SetWindowTextA( timeString.c_str() );
		}
	}
	else
	{
		timeDisplay.SetWindowTextA( "FIN!" );
		timeDisplay.RedrawWindow();
	}
}

void ExperimentTimer::reorganizeControls( std::string cameraMode, std::string triggerMode, int width, int height,
										 std::unordered_map<std::string, CFont* > fonts )
{
	timeDisplay.rearrange( cameraMode, triggerMode, width, height, fonts );
	variationProgress.rearrange( cameraMode, triggerMode, width, height, fonts );
	overallProgress.rearrange( cameraMode, triggerMode, width, height, fonts );
}


void ExperimentTimer::setTimerDisplay(std::string newText)
{
	timeDisplay.SetWindowTextA( newText.c_str() );
}