// created by Mark O. Brown
#pragma once
#include "Andor/cameraPositions.h"
#include <vector>
#include "Windows.h"
#include "Control.h"
#include <qWinWidget.h>
#include <qlabel>
#include <qprogressbar.h>
#include "PrimaryWindows/IChimeraWindowWidget.h"

class ExperimentTimer
{
	public:
		void initialize( POINT& inputLoc, IChimeraWindowWidget* parent );
		void update( ULONGLONG currentRepNumber, ULONGLONG repsPerVariation, ULONGLONG numberOfVariations, 
					 UINT picsPerRep );
		void rearrange( int width, int height, fontMap fonts );
		void setTimerDisplay( std::string newText );
	private:
		QLabel* timeDisplay;
		QProgressBar* variationProgress;
		QProgressBar* overallProgress;
		long long firstTime;
		std::vector<double> recentDataPoints;
};