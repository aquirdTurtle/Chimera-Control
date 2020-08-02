// created by Mark O. Brown
#pragma once

#include "Control.h"
#include "GeneralObjects/coordinate.h"
#include "Andor/AndorRunMode.h"
#include "Andor/AndorTriggerModes.h"
#include <vector>
#include <string>
#include <array>
#include "LowLevel/constants.h"
#include "PrimaryWindows/IChimeraQtWindow.h"
#include <qlabel.h>

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
	// also calibrated X50, just didn't include that here. (for some reason?)
	const double countToCameraPhotonEM200 = 0.018577;
	const double countToScatteredPhotonEM200 = 0.018577 / 0.07;
	const double conventionalBackgroundCount = 88;
	const double EMGain200BackgroundCount = 105;
};


struct statPoint
{
	double minv;
	double maxv;
	double avgv;
	double selv;
	statPoint operator* ( double x )
	{
		return { this->minv*x, this->maxv*x, this->avgv*x, this->selv*x };
	}
	statPoint operator- ( double x )
	{
		return { this->minv-x, this->maxv-x, this->avgv-x, this->selv-x };
	}
};


class PictureStats
{
	public:
		PictureStats::PictureStats()
		{
			displayDataType = RAW_COUNTS;
		}
		void initialize( POINT& pos, IChimeraQtWindow* parent );
		std::pair<int, int> update ( Matrix<long> image, unsigned imageNumber, coordinate selectedPixel,
									 int currentRepetitionNumber, int totalRepetitionCount );
		void reset();
		void updateType( std::string typeText );
		statPoint getMostRecentStats ( );
	private:
		statPoint mostRecentStat;
		std::string displayDataType;
		conversions convs;
		QLabel* pictureStatsHeader;
		QLabel* repetitionIndicator;
		std::array<QLabel*, 5> collumnHeaders;
		std::array<QLabel*, 4> picNumberIndicators;
		std::array<QLabel*, 4> maxCounts;
		std::array<QLabel*, 4> minCounts;
		// average counts in the picture.
		std::array<QLabel*, 4> avgCounts;
		std::array<QLabel*, 4> selCounts;
}
;


