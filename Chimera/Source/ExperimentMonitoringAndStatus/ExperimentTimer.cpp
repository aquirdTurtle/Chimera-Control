// created by Mark O. Brown
#include "stdafx.h"
#include "Commctrl.h"
#include "Uxtheme.h"
#include "ExperimentTimer.h"
#include "PrimaryWindows/QtAndorWindow.h"
#include <qprogressbar.h>


void ExperimentTimer::initialize( QPoint& pos, IChimeraQtWindow* parent ){
	auto& px = pos.rx (), & py = pos.ry ();
	timeDisplay = new QLabel ("", parent);
	timeDisplay->setGeometry (px, py, 168, 40);
	/// PROGRESS BARS
	// subseries progress bar
	long timerWidth = 550 * 2;
	variationProgress = new QProgressBar (parent);
	variationProgress->setGeometry (px + 168, py, timerWidth - 168,15);
	variationProgress->setRange (0, 10000);
	// series progress bar display
	overallProgress = new QProgressBar (parent);
	overallProgress->setGeometry (px + 168, py+15, timerWidth - 168, 25);
	overallProgress->setRange (0, 10000);
}

void ExperimentTimer::update(unsigned __int64 currentRepNumber, unsigned __int64 repsPerVariation, unsigned __int64 numberOfVariations, unsigned picsPerRep)
{
	int totalRepetitions = repsPerVariation * numberOfVariations;
	int minAverageNumber = 10;
	int variationPosition = int((currentRepNumber % repsPerVariation) * 10000.0 / repsPerVariation);
	int overallPosition = int( currentRepNumber / (double)totalRepetitions * 10000.0 );
	variationProgress->setValue(variationPosition);
	overallProgress->setValue( overallPosition );
	if (currentRepNumber == 1)
	{
		firstTime = GetTickCount64();
		timeDisplay->setText( "Estimating Time..." );
	}
	else if (currentRepNumber < totalRepetitions)
	{
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
			timeDisplay->setText( cstr(timeString) );
		}
	}
	else
	{
		timeDisplay->setText( "FIN!" );
	}
}


void ExperimentTimer::setTimerDisplay(std::string newText)
{
	timeDisplay->setText( cstr(newText) );
}