#pragma once
#include <array>
#include "Control.h"

// contains experimentally calibrated conversion factors. See the onenote for more conversion factors and notes on the 
// calibration.
struct conversions
{
	const double countToCameraPhoton = 0.697798;
	const double countToScatteredPhoton = 0.697798 / 0.07;
	// for em gain of 200 only! Could add more arbitrary approximation, but I specifically calibrated this value (I 
	// also calibrated X50, just didn't include that here.
	const double countToCameraPhotonEM200 = 0.018577;
	const double countToScatteredPhotonEM200 = 0.018577 / 0.07;
	const double conventionalBackgroundCount = 88;
	const double EMGain200BackgroundCount = 105;
};

class PictureStats
{
	public:
		bool initialize(POINT& kineticPos, POINT& accumulatePos, POINT& continuousPos, HWND parentWindow);
		bool reorganizeControls(RECT parentRectangle, std::string mode);
		bool update(unsigned long selCounts, unsigned long maxCounts, unsigned long minCounts, double avgCounts, unsigned int image);
		bool reset();
		bool PictureStats::updateType(std::string typeText);
	private:
		conversions convs;
		HwndControl pictureStatsHeader;
		HwndControl repetitionIndicator;
		std::array<HwndControl, 5> collumnHeaders;
		std::array<HwndControl, 4> picNumberIndicators;
		std::array<HwndControl, 4> maxCounts;
		std::array<HwndControl, 4> minCounts;
		// average counts in the picture.
		std::array<HwndControl, 4> avgCounts;
		std::array<HwndControl, 4> selCounts;

};



