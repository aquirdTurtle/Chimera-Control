#pragma once
#include "Windows.h"
#include "Control.h"
#include <vector>

struct cameraPositions;

class ExperimentTimer
{
	public:
		ExperimentTimer();
		~ExperimentTimer();
		void initialize( cameraPositions& inputLoc, CWnd* parent, bool isTriggerModeSensitive, int& id,
						 std::unordered_map<std::string, CFont*> fonts, std::vector<CToolTipCtrl*> toolTips );
		void update( int currentRepNumber, int repsPerVariation, int numberOfVariations, int picsPerRep,
					 CWnd* parentWindow );
		void reorganizeControls( std::string cameraMode, std::string triggerMode, int width, int height,
								 std::unordered_map<std::string, CFont* > fonts );
		std::string getColor();
		void setColor( std::string newColor );
		void setTimerDisplay( std::string newText );
	private:
		Control<CStatic> timeDisplay;
		Control<CProgressCtrl> variationProgress;
		Control<CProgressCtrl> overallProgress;
		long long firstTime;
		std::string timeColor;
		std::vector<double> recentDataPoints;
};