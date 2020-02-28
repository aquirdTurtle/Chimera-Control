// created by Mark O. Brown
#pragma once
#include "Andor/cameraPositions.h"
#include <vector>
#include "Windows.h"
#include "Control.h"
;
class ExperimentTimer
{
	public:
		void initialize( POINT& inputLoc, CWnd* parent, bool isTriggerModeSensitive, int& id, cToolTips& toolTips );
		void update( ULONGLONG currentRepNumber, ULONGLONG repsPerVariation, ULONGLONG numberOfVariations, 
					 UINT picsPerRep );
		void rearrange( int width, int height, fontMap fonts );
		void setTimerDisplay( std::string newText );
	private:
		Control<CStatic> timeDisplay;
		Control<CProgressCtrl> variationProgress;
		Control<CProgressCtrl> overallProgress;
		long long firstTime;
		std::vector<double> recentDataPoints;
};