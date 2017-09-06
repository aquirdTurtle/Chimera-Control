#pragma once
#include <array>
#include "Control.h"

// contains experimentally calibrated conversion factors. See the onenote section on the camera
// for more conversion factors and notes on the calibration.
/*
const double countToCameraPhoton = 0.697798;
const double countToScatteredPhoton = 0.697798 / 0.07;
// for em gain of 200 only! Could add more arbitrary approximation, but I specifically calibrated this value (I				 
// also calibrated X50, just didn't include that here.
const double countToCameraPhotonEM200 = 0.018577;
const double countToScatteredPhotonEM200 = 0.018577 / 0.07;
const double conventionalBackgroundCount = 88;
const double EMGain200BackgroundCount = 105;
*/
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
		PictureStats::PictureStats()
		{
			displayDataType = RAW_COUNTS;
		}
		void initialize( POINT& pos, CWnd* parent, int& id, cToolTips& tooltips );
		void rearrange( std::string cameraMode, std::string trigMode, int width, int height, fontMap fonts );
		std::pair<int, int> update( std::vector<long> image, UINT imageNumber, coordinate selectedPixel,
									int pictureWidth, int pictureHeight, int currentRepetitionNumbar,
									int totalRepetitionCount );
		void reset();
		void updateType( std::string typeText );

	private:
		std::string displayDataType;
		conversions convs;
		Control<CStatic> pictureStatsHeader;
		Control<CStatic> repetitionIndicator;
		std::array<Control<CStatic>, 5> collumnHeaders;
		std::array<Control<CStatic>, 4> picNumberIndicators;
		std::array<Control<CStatic>, 4> maxCounts;
		std::array<Control<CStatic>, 4> minCounts;
		// average counts in the picture.
		std::array<Control<CStatic>, 4> avgCounts;
		std::array<Control<CStatic>, 4> selCounts;

}
;


