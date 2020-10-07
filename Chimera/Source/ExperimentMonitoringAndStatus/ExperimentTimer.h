// created by Mark O. Brown
#pragma once
#include <vector>
#include "Control.h"
#include <qlabel>
#include <qprogressbar.h>
#include "PrimaryWindows/IChimeraQtWindow.h"

class ExperimentTimer
{
	public:
		void initialize( QPoint& inputLoc, IChimeraQtWindow* parent );
		void update( unsigned __int64 currentRepNumber, unsigned __int64 repsPerVariation, unsigned __int64 numberOfVariations, 
					 unsigned picsPerRep );
		void setTimerDisplay( std::string newText );
	private:
		QLabel* timeDisplay;
		QProgressBar* variationProgress;
		QProgressBar* overallProgress;
		long long firstTime;
		std::vector<double> recentDataPoints;
};