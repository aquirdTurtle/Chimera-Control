// created by Mark O. Brown
#include "stdafx.h"
#include <algorithm>
#include <numeric>
#include "PictureStats.h"


// as of right now, the position of this control is not affected by the mode or the trigger mode.
void PictureStats::initialize( QPoint& pos, IChimeraQtWindow* parent )
{
	auto& px = pos.rx (), & py = pos.ry ();
	LONG size = 315;
	pictureStatsHeader = new QLabel ("Raw Counts", parent);
	pictureStatsHeader->setGeometry (px, py, size, 25);
	repetitionIndicator = new QLabel ("Repetition ?/?", parent);
	repetitionIndicator->setGeometry (px, py+=25, size, 25);
	/// Picture labels ////////////////////////////////////////////////////////////
	collumnHeaders[0] = new QLabel ("Pic:", parent);
	collumnHeaders[0]->setGeometry (px, py += 25, size / 5, 25);
	int inc = 0;
	for (auto& control : picNumberIndicators)
	{
		inc++;
		control = new QLabel (cstr ("#" + str (inc) + ":"), parent);
		control->setGeometry (px, py+=25, size / 5, 25);
	}
	/// Max Count 
	py -= 100;
	px += size / 5;
	collumnHeaders[1] = new QLabel ("Max:", parent);
	collumnHeaders[1]->setGeometry (px, py, size / 5,25);
	for (auto& control : maxCounts) {
		control = new QLabel ("-", parent);
		control->setGeometry (px, py += 25, size / 5, 25);
	}
	/// Min Counts
	py -= 100;
	px += size / 5;
	collumnHeaders[2] = new QLabel ("Min:", parent);
	collumnHeaders[2]->setGeometry (px, py, size / 5, 25);
	for (auto& control : minCounts) {
		control = new QLabel ("-", parent);
		control->setGeometry (px, py += 25, size / 5, 25);
	}
	/// Average Counts
	py -= 100;
	px += size / 5;
	collumnHeaders[3] = new QLabel ("Avg:", parent);
	collumnHeaders[3]->setGeometry (px, py, size / 5, 25);
	for (auto& control : avgCounts) {
		control = new QLabel ("-", parent);
		control->setGeometry (px, py += 25, size / 5, 25);
	}
	/// Selection Counts
	py -= 100;
	px += size / 5;
	collumnHeaders[4] = new QLabel ("Avg:", parent);
	collumnHeaders[4]->setGeometry (px, py, size / 5, 25);
	for (auto& control : selCounts) {
		control = new QLabel ("-", parent);
		control->setGeometry (px, py += 25, size / 5, 25);
	}
	px -= 4./5*size;
}


void PictureStats::reset(){
	for (auto& control : maxCounts)	{
		control->setText("-");
	}
	for (auto& control : minCounts)	{
		control->setText ("-");
	}
	for (auto& control : avgCounts)	{
		control->setText ("-");
	}
	for (auto& control : selCounts){
		control->setText ("-");
	}
	repetitionIndicator->setText ( "Repetition ---/---" );
}


void PictureStats::updateType(std::string typeText){
	displayDataType = typeText;
	pictureStatsHeader->setText (cstr(typeText));
}


statPoint PictureStats::getMostRecentStats ( ){
	return mostRecentStat;
}


std::pair<int, int> PictureStats::update ( Matrix<long> image, unsigned imageNumber, coordinate selectedPixel, 
										   int currentRepetitionNumber, int totalRepetitionCount ){
	repetitionIndicator->setText ( cstr ( "Repetition " + str ( currentRepetitionNumber ) + "/"
												+ str ( totalRepetitionCount ) ) );
	if ( image.size ( ) == 0 ){
		// hopefully this helps with stupid imaging bug...
		return { 0,0 };
	}
	statPoint currentStatPoint;
	currentStatPoint.selv = image ( selectedPixel.row, selectedPixel.column );
	currentStatPoint.minv = 65536;
	currentStatPoint.maxv = 1;
	// for all pixels... find the max and min of the picture.
	for (auto pixel : image)	{
		try	{
			if ( pixel > currentStatPoint.maxv ){
				currentStatPoint.maxv = pixel;
			}
			if ( pixel < currentStatPoint.minv ){
				currentStatPoint.minv = pixel;
			}
		}
		catch ( std::out_of_range& ){
			// I haven't seen this error in a while, but it was a mystery when we did.
			errBox ( "ERROR: caught std::out_of_range while updating picture statistics! experimentImagesInc = "
					 + str ( imageNumber ) + ", pixelInc = " + str ( "NA" ) + ", image.size() = " + str ( image.size ( ) )
					 + ". Attempting to continue..." );
			return { 0,0 };
		}
	}
	currentStatPoint.avgv = std::accumulate ( image.data.begin ( ), image.data.end ( ), 0.0 ) / image.size ( );

	if ( displayDataType == RAW_COUNTS ){
		maxCounts[ imageNumber ]->setText ( cstr ( currentStatPoint.maxv, 1 ) );
		minCounts[ imageNumber ]->setText ( cstr ( currentStatPoint.minv, 1 ) );
		selCounts[ imageNumber ]->setText ( cstr ( currentStatPoint.selv, 1 ) );
		avgCounts[ imageNumber ]->setText ( cstr ( currentStatPoint.avgv, 5 ) );
		mostRecentStat = currentStatPoint;
	}
	else if ( displayDataType == CAMERA_PHOTONS ){
		statPoint camPoint;
		//double selPhotons, maxPhotons, minPhotons, avgPhotons;
		//if (eEMGainMode)
		if ( false ){
			camPoint = (currentStatPoint - convs.EMGain200BackgroundCount ) * convs.countToCameraPhotonEM200;
		}
		else{
			camPoint = ( currentStatPoint - convs.conventionalBackgroundCount ) * convs.countToCameraPhoton;
		}
		maxCounts[ imageNumber ]->setText ( cstr ( camPoint.maxv, 1 ) );
		minCounts[ imageNumber ]->setText ( cstr ( camPoint.minv, 1 ) );
		selCounts[ imageNumber ]->setText ( cstr ( camPoint.selv, 1 ) );
		avgCounts[ imageNumber ]->setText ( cstr ( camPoint.avgv, 1 ) );
		mostRecentStat = camPoint;
	}
	else if ( displayDataType == ATOM_PHOTONS ){
		statPoint atomPoint;
		//if (eEMGainMode)
		if ( false ){
			atomPoint = ( currentStatPoint - convs.EMGain200BackgroundCount ) * convs.countToScatteredPhotonEM200;
		}
		else{
			atomPoint = ( currentStatPoint - convs.conventionalBackgroundCount ) * convs.countToScatteredPhoton;
		}
		maxCounts[ imageNumber ]->setText ( cstr ( atomPoint.maxv, 1 ) );
		minCounts[ imageNumber ]->setText ( cstr ( atomPoint.minv, 1 ) );
		selCounts[ imageNumber ]->setText ( cstr ( atomPoint.selv, 1 ) );
		avgCounts[ imageNumber ]->setText ( cstr ( atomPoint.avgv, 1 ) );
		mostRecentStat = atomPoint;
	}	
	return { currentStatPoint.minv, currentStatPoint.maxv };
}