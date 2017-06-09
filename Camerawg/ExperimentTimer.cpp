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
								  fontMap fonts, std::vector<CToolTipCtrl*> toolTips )
{
	timeDisplay.sPos = { inputLoc.sPos.x, inputLoc.sPos.y, inputLoc.sPos.x + 168, inputLoc.sPos.y + 40 };
	timeDisplay.ID = id++;
	timeDisplay.Create("", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, timeDisplay.sPos, parent, timeDisplay.ID);
	timeDisplay.fontType = Normal;
	/// PROGRESS BARS
	// subseries progress bar
	variationProgress.sPos = { inputLoc.sPos.x + 168, inputLoc.sPos.y, inputLoc.sPos.x + 1168, inputLoc.sPos.y + 15 };
	variationProgress.ID = id++;
	variationProgress.Create(WS_CHILD | WS_VISIBLE, variationProgress.sPos, parent, variationProgress.ID);
	variationProgress.SetBkColor(RGB(100, 110, 100));
	variationProgress.SetBarColor(RGB(0, 200, 0));
	// series progress bar display
	overallProgress.sPos = { inputLoc.sPos.x + 168, inputLoc.sPos.y + 15, inputLoc.sPos.x + 1168, inputLoc.sPos.y + 40 };
	overallProgress.ID = id++;
	overallProgress.Create(WS_CHILD | WS_VISIBLE, overallProgress.sPos, parent, overallProgress.ID);
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
		firstTime = GetTickCount64();
		timeDisplay.fontType = Normal;
		timeDisplay.SetWindowTextA( "Estimating Time..." );
		timeDisplay.RedrawWindow();
	}
	else if (currentRepNumber < totalRepetitions)
	{
		if (currentRepNumber == minAverageNumber)
		{
			timeDisplay.fontType = Large;
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

void ExperimentTimer::rearrange( std::string cameraMode, std::string triggerMode, int width, int height,
										 fontMap fonts )
{
	timeDisplay.rearrange( cameraMode, triggerMode, width, height, fonts );
	variationProgress.rearrange( cameraMode, triggerMode, width, height, fonts );
	overallProgress.rearrange( cameraMode, triggerMode, width, height, fonts );
}


void ExperimentTimer::setTimerDisplay(std::string newText)
{
	timeDisplay.SetWindowTextA( newText.c_str() );
}