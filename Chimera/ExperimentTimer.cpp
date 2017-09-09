#include "stdafx.h"
#include "Commctrl.h"
#include "Uxtheme.h"
#include "ExperimentTimer.h"
#include <vector>
#include "CameraWindow.h"


void ExperimentTimer::initialize( cameraPositions& inputLoc, CWnd* parent, bool isTriggerModeSensitive, int& id,
								  cToolTips& toolTips )
{
	timeDisplay.sPos = { inputLoc.sPos.x, inputLoc.sPos.y, inputLoc.sPos.x + 168, inputLoc.sPos.y + 40 };
	timeDisplay.Create("", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, timeDisplay.sPos, parent, id++ );
	/// PROGRESS BARS
	// subseries progress bar
	variationProgress.sPos = { inputLoc.sPos.x + 168, inputLoc.sPos.y, inputLoc.sPos.x + 1168, inputLoc.sPos.y + 15 };
	variationProgress.Create(WS_CHILD | WS_VISIBLE | PBS_SMOOTH, variationProgress.sPos, parent, id++ );
	variationProgress.SetBkColor(RGB(100, 110, 100));
	variationProgress.SetBarColor(RGB(0, 200, 0));
	variationProgress.SetRange32( 0, 10000 );
	// series progress bar display
	overallProgress.sPos = { inputLoc.sPos.x + 168, inputLoc.sPos.y + 15, inputLoc.sPos.x + 1168, inputLoc.sPos.y + 40 };
	overallProgress.Create(WS_CHILD | WS_VISIBLE | PBS_SMOOTH, overallProgress.sPos, parent, id++ );
	overallProgress.SetBkColor(RGB(100, 110, 100));
	overallProgress.SetBarColor(RGB(255, 255, 255));
	overallProgress.SetRange32( 0, 10000 );
	//overallProgress.Set
	inputLoc.seriesPos.y += 40;
	inputLoc.amPos.y += 40;
	inputLoc.videoPos.y += 40;
}

void ExperimentTimer::update(ULONGLONG currentRepNumber, ULONGLONG repsPerVariation, ULONGLONG numberOfVariations, UINT picsPerRep)
{
	int totalRepetitions = repsPerVariation * numberOfVariations;
	int minAverageNumber = 10;
	int variationPosition = int((currentRepNumber % repsPerVariation) * 10000.0 / repsPerVariation);
	int overallPosition = int(currentRepNumber / (double)totalRepetitions * 10000.0);
	variationProgress.SetPos ( variationPosition );
	overallProgress.SetPos( overallPosition );
	if (currentRepNumber == 1)
	{
		firstTime = GetTickCount64();
		timeDisplay.fontType = NormalFont;
		timeDisplay.SetWindowTextA( "Estimating Time..." );
		timeDisplay.RedrawWindow();
	}
	else if (currentRepNumber < totalRepetitions)
	{
		if (currentRepNumber == minAverageNumber)
		{
			timeDisplay.fontType = LargeFont;
			timeDisplay.RedrawWindow();
		}
		long long thisTime = GetTickCount64();
		if (currentRepNumber % picsPerRep == 0)
		{
			double averageTime;
			if (currentRepNumber != 0)
			{
				averageTime = double((thisTime - firstTime) / currentRepNumber);
			}
			else
			{
				averageTime = 0;
			}
			// in seconds...
			int timeLeft = int((repsPerVariation * numberOfVariations - currentRepNumber) * averageTime / 1000);
			int hours = timeLeft / 3600;
			int minutes = (timeLeft % 3600) / 60;
			int seconds = (timeLeft % 3600) % 60;
			std::string timeString = "";
			timeString += str(hours) + ":";
			if (minutes < 10)
			{
				timeString += "0" + str(minutes);
			}
			else
			{
				timeString += str(minutes);
			}
			if (hours == 0 && minutes < 5)
			{
				if (seconds < 10)
				{
					timeString += ":0" + str(seconds);
				}
				else
				{
					timeString += ":" + str(seconds);
				}
			}
			timeDisplay.SetWindowTextA( cstr(timeString) );
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
	timeDisplay.SetWindowTextA( cstr(newText) );
}