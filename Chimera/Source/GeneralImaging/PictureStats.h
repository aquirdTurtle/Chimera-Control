// created by Mark O. Brown
#pragma once

#include "Control.h"
#include "GeneralObjects/coordinate.h"
#include "Andor/AndorRunMode.h"
#include "Andor/AndorTriggerModes.h"
#include <GeneralObjects/Matrix.h>
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

struct conversions{
	const double countToCameraPhoton = 0.697798;
	const double countToScatteredPhoton = 0.697798 / 0.07;
	// for em gain of 200 only! Could add more arbitrary approximation, but I specifically calibrated this value (I 
	// also calibrated X50, just didn't include that here. (for some reason?)
	const double countToCameraPhotonEM200 = 0.018577;
	const double countToScatteredPhotonEM200 = 0.018577 / 0.07;
	const double conventionalBackgroundCount = 88;
	const double EMGain200BackgroundCount = 105;
};

struct statPoint{
	double minv;
	double maxv;
	double avgv;
	double selv;
	statPoint operator* ( double x ){
		return { this->minv*x, this->maxv*x, this->avgv*x, this->selv*x };
	}
	statPoint operator- ( double x ){
		return { this->minv-x, this->maxv-x, this->avgv-x, this->selv-x };
	}
};


class PictureStats{
	public:
		static constexpr auto RAW_COUNTS = "Raw Counts";
		static constexpr auto CAMERA_PHOTONS = "Camera Photons";
		static constexpr auto ATOM_PHOTONS = "Atom Photons";
		::PictureStats(){
			displayDataType = RAW_COUNTS;
		}
		void initialize( QPoint& pos, IChimeraQtWindow* parent );
		template <class type>
		std::pair<int, int> update ( Matrix<type> image, unsigned imageNumber, coordinate selectedPixel,
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
};

template <class type>
std::pair<int, int> PictureStats::update (Matrix<type> image, unsigned imageNumber, coordinate selectedPixel,
	int currentRepetitionNumber, int totalRepetitionCount) {
	repetitionIndicator->setText (cstr ("Repetition " + str (currentRepetitionNumber) + "/"
		+ str (totalRepetitionCount)));
	if (image.size () == 0) {
		// hopefully this helps with stupid imaging bug...
		return { 0,0 };
	}
	statPoint currentStatPoint;
	currentStatPoint.selv = image (selectedPixel.row, selectedPixel.column);
	currentStatPoint.minv = 65536;
	currentStatPoint.maxv = 1;
	// for all pixels... find the max and min of the picture.
	for (auto pixel : image) {
		try {
			if (pixel > currentStatPoint.maxv) {
				currentStatPoint.maxv = pixel;
			}
			if (pixel < currentStatPoint.minv) {
				currentStatPoint.minv = pixel;
			}
		}
		catch (std::out_of_range&) {
			// I haven't seen this error in a while, but it was a mystery when we did.
			errBox ("ERROR: caught std::out_of_range while updating picture statistics! experimentImagesInc = "
				+ str (imageNumber) + ", pixelInc = " + str ("NA") + ", image.size() = " + str (image.size ())
				+ ". Attempting to continue...");
			return { 0,0 };
		}
	}
	currentStatPoint.avgv = std::accumulate (image.data.begin (), image.data.end (), 0.0) / image.size ();
	if (displayDataType == RAW_COUNTS) {
		maxCounts[imageNumber]->setText (qstr (currentStatPoint.maxv, 1));
		qDebug() << maxCounts[imageNumber]->text();
		minCounts[imageNumber]->setText (qstr (currentStatPoint.minv, 1));
		qDebug() << minCounts[imageNumber]->text();
		selCounts[imageNumber]->setText (qstr (currentStatPoint.selv, 1));
		qDebug() << selCounts[imageNumber]->text();
		avgCounts[imageNumber]->setText (qstr (currentStatPoint.avgv, 5));
		qDebug() << avgCounts[imageNumber]->text();
		mostRecentStat = currentStatPoint;
	}
	else if (displayDataType == CAMERA_PHOTONS) {
		statPoint camPoint;
		//double selPhotons, maxPhotons, minPhotons, avgPhotons;
		//if (eEMGainMode)
		if (false) {
			camPoint = (currentStatPoint - convs.EMGain200BackgroundCount) * convs.countToCameraPhotonEM200;
		}
		else {
			camPoint = (currentStatPoint - convs.conventionalBackgroundCount) * convs.countToCameraPhoton;
		}
		maxCounts[imageNumber]->setText (cstr (camPoint.maxv, 1));
		minCounts[imageNumber]->setText (cstr (camPoint.minv, 1));
		selCounts[imageNumber]->setText (cstr (camPoint.selv, 1));
		avgCounts[imageNumber]->setText (cstr (camPoint.avgv, 1));
		mostRecentStat = camPoint;
	}
	else if (displayDataType == ATOM_PHOTONS) {
		statPoint atomPoint;
		//if (eEMGainMode)
		if (false) {
			atomPoint = (currentStatPoint - convs.EMGain200BackgroundCount) * convs.countToScatteredPhotonEM200;
		}
		else {
			atomPoint = (currentStatPoint - convs.conventionalBackgroundCount) * convs.countToScatteredPhoton;
		}
		maxCounts[imageNumber]->setText (cstr (atomPoint.maxv, 1));
		minCounts[imageNumber]->setText (cstr (atomPoint.minv, 1));
		selCounts[imageNumber]->setText (cstr (atomPoint.selv, 1));
		avgCounts[imageNumber]->setText (cstr (atomPoint.avgv, 1));
		mostRecentStat = atomPoint;
	}
	return { currentStatPoint.minv, currentStatPoint.maxv };
}


