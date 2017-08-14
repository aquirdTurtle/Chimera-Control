#pragma once
#include "Windows.h"
#include "Control.h"
#include <vector>

struct cameraPositions;

class ExperimentTimer
{
	public:
		void initialize( cameraPositions& inputLoc, CWnd* parent, bool isTriggerModeSensitive, int& id, fontMap fonts, 
						 cToolTips& toolTips );
		void update( int currentRepNumber, int repsPerVariation, int numberOfVariations, int picsPerRep );
		void rearrange( std::string cameraMode, std::string triggerMode, int width, int height, fontMap fonts );
		void setTimerDisplay( std::string newText );
	private:
		Control<CStatic> timeDisplay;
		Control<CProgressCtrl> variationProgress;
		Control<CProgressCtrl> overallProgress;
		long long firstTime;
		std::vector<double> recentDataPoints;
};