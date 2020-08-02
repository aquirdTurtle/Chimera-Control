// created by Mark O. Brown
#pragma once
#include "Andor/cameraPositions.h"
#include <vector>
#include "Control.h"
#include <qlabel>
#include <qprogressbar.h>
#include "PrimaryWindows/IChimeraQtWindow.h"

class ExperimentTimer
{
	public:
		void initialize( POINT& inputLoc, IChimeraQtWindow* parent );
		void update( ULONGLONG currentRepNumber, ULONGLONG repsPerVariation, ULONGLONG numberOfVariations, 
					 unsigned picsPerRep );
		void setTimerDisplay( std::string newText );
	private:
		QLabel* timeDisplay;
		QProgressBar* variationProgress;
		QProgressBar* overallProgress;
		long long firstTime;
		std::vector<double> recentDataPoints;
};