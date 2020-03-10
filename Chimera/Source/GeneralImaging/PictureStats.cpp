// created by Mark O. Brown
#include "stdafx.h"
#include <algorithm>
#include <numeric>
#include "PictureStats.h"


// as of right now, the position of this control is not affected by the mode or the trigger mode.
void PictureStats::initialize( POINT& pos, CWnd* parent, int& id, cToolTips& tooltips )
{
	LONG size = 315;
	pictureStatsHeader.sPos = { pos.x, pos.y, pos.x + size, pos.y + 25 };
	pictureStatsHeader.Create( "Raw Counts", NORM_STATIC_OPTIONS, pictureStatsHeader.sPos, parent, id++ );
	pos.y += 25;
	repetitionIndicator.sPos = { pos.x, pos.y, pos.x + size, pos.y + 25 };
	repetitionIndicator.Create( "Repetition ?/?", NORM_STATIC_OPTIONS, repetitionIndicator.sPos, parent, id++ );
	pos.y += 25;
	/// Picture labels ////////////////////////////////////////////////////////////
	collumnHeaders[0].sPos = { pos.x, pos.y, pos.x + LONG (size/5), pos.y + 25 };
	collumnHeaders[0].Create( "Pic:", NORM_STATIC_OPTIONS, collumnHeaders[0].sPos, parent, id++ );
	collumnHeaders[0].fontType = fontTypes::SmallFont;
	pos.y += 25;
	int inc = 0;
	for (auto& control : picNumberIndicators)
	{
		inc++;
		control.sPos = { pos.x, pos.y, pos.x + LONG (size / 5), pos.y + 25 };
		control.Create( cstr("#" + str( inc ) + ":"), NORM_STATIC_OPTIONS, control.sPos, parent, id++);
		control.fontType = fontTypes::SmallFont;
		pos.y += 25;
	}
	pos.y -= 125;

	/// Max Count Edits
	// Max Count Display 742 - 480 )/2 = 131 
	collumnHeaders[1].sPos = { pos.x + LONG (size / 5), pos.y, pos.x + LONG (2*size / 5), pos.y + 25 };
	collumnHeaders[1].Create( "Max:", NORM_STATIC_OPTIONS, collumnHeaders[1].sPos, parent, id++ );
	collumnHeaders[1].fontType = fontTypes::SmallFont;
	pos.y += 25;
	// #1
	for (auto& control : maxCounts)
	{
		control.sPos = { pos.x + LONG (size / 5), pos.y, pos.x + LONG (2 * size / 5), pos.y + 25 };
		control.Create( "-", NORM_STATIC_OPTIONS, control.sPos, parent, id++ );
		control.fontType = fontTypes::SmallFont;
		pos.y += 25;
	}
	// back to top.
	pos.y -= 125;
	/// Min Counts
	// Min Count Display	
	collumnHeaders[2].sPos = { pos.x + LONG (2*size / 5), pos.y, pos.x + LONG (3*size / 5), pos.y + 25 };
	collumnHeaders[2].Create( "Min:", NORM_STATIC_OPTIONS, collumnHeaders[2].sPos, parent, id++ );
	collumnHeaders[2].fontType = fontTypes::SmallFont;
	pos.y += 25;

	for (auto& control : minCounts)
	{
		control.sPos = { pos.x + LONG (2*size / 5), pos.y, pos.x + LONG (3*size / 5), pos.y + 25 };
		control.Create( "-", NORM_STATIC_OPTIONS, control.sPos, parent, id++ );
		control.fontType = fontTypes::SmallFont;
		pos.y += 25;
	}
	pos.y -= 125;
	/// Average Counts
	collumnHeaders[3].sPos = { pos.x + LONG (3*size / 5), pos.y, pos.x + LONG (4*size / 5), pos.y + 25 };
	collumnHeaders[3].Create( "Avg:", NORM_STATIC_OPTIONS, collumnHeaders[3].sPos, parent, id++ );
	collumnHeaders[3].fontType = fontTypes::SmallFont;
	pos.y += 25;
	// 
	for (auto& control : avgCounts)
	{
		control.sPos = { pos.x + LONG (3*size / 5), pos.y, pos.x + LONG (4*size / 5), pos.y + 25 };
		control.Create( "-", NORM_STATIC_OPTIONS, control.sPos, parent, id++ );
		control.fontType = fontTypes::NormalFont;
		pos.y += 25;
	}

	pos.y -= 125;
	/// Selection Counts
	collumnHeaders[4].sPos = { pos.x + LONG (4*size / 5), pos.y, pos.x + size, pos.y + 25 };
	collumnHeaders[4].Create( "Sel:", NORM_STATIC_OPTIONS, collumnHeaders[4].sPos, parent, id++ );
	collumnHeaders[4].fontType = fontTypes::SmallFont;
	pos.y += 25;
	// #1
	for (auto& control : selCounts)
	{
		control.sPos = { pos.x + LONG (4*size / 5), pos.y, pos.x + size, pos.y += 25 };
		control.Create( "-", NORM_STATIC_OPTIONS, control.sPos, parent, id++ );
		control.fontType = fontTypes::SmallFont;
	}
}

void PictureStats::rearrange(int width, int height, fontMap fonts)
{
	pictureStatsHeader.rearrange(width, height, fonts);
	repetitionIndicator.rearrange(width, height, fonts);
	for (auto& control : collumnHeaders)
	{
		control.rearrange(width, height, fonts);
	}
	for (auto& control : maxCounts)
	{
		control.rearrange(width, height, fonts);
	}
	for (auto& control : minCounts)
	{
		control.rearrange(width, height, fonts);
	}
	for (auto& control : picNumberIndicators)
	{
		control.rearrange(width, height, fonts);
	}
	for (auto& control : selCounts)
	{
		control.rearrange(width, height, fonts);
	}
	for (auto& control : avgCounts)
	{
		control.rearrange(width, height, fonts);
	}
}


void PictureStats::reset()
{
	for (auto& control : maxCounts)
	{
		control.SetWindowText("-");
	}
	for (auto& control : minCounts)
	{
		control.SetWindowText("-");
	}
	for (auto& control : avgCounts)
	{
		control.SetWindowText("-");
	}
	for (auto& control : selCounts)
	{
		control.SetWindowText("-");
	}
	repetitionIndicator.SetWindowTextA( "Repetition ---/---" );
}


void PictureStats::updateType(std::string typeText)
{
	displayDataType = typeText;
	pictureStatsHeader.SetWindowText(cstr(typeText));
}


statPoint PictureStats::getMostRecentStats ( )
{
	return mostRecentStat;
}


std::pair<int, int> PictureStats::update ( Matrix<long> image, UINT imageNumber, coordinate selectedPixel, 
										   int currentRepetitionNumber, int totalRepetitionCount )
{
	repetitionIndicator.SetWindowTextA ( cstr ( "Repetition " + str ( currentRepetitionNumber ) + "/"
												+ str ( totalRepetitionCount ) ) );
	if ( image.size ( ) == 0 )
	{
		// hopefully this helps with stupid imaging bug...
		return { 0,0 };
	}
	statPoint currentStatPoint;
	currentStatPoint.selv = image ( selectedPixel.row, selectedPixel.column );
	currentStatPoint.minv = 65536;
	currentStatPoint.maxv = 1;
	// for all pixels... find the max and min of the picture.
	for (auto pixel : image)
	{
		try
		{
			if ( pixel > currentStatPoint.maxv )
			{
				currentStatPoint.maxv = pixel;
			}
			if ( pixel < currentStatPoint.minv )
			{
				currentStatPoint.minv = pixel;
			}
		}
		catch ( std::out_of_range& )
		{
			// I haven't seen this error in a while, but it was a mystery when we did.
			errBox ( "ERROR: caught std::out_of_range while updating picture statistics! experimentImagesInc = "
					 + str ( imageNumber ) + ", pixelInc = " + str ( "NA" ) + ", image.size() = " + str ( image.size ( ) )
					 + ". Attempting to continue..." );
			return { 0,0 };
		}
	}
	currentStatPoint.avgv = std::accumulate ( image.data.begin ( ), image.data.end ( ), 0.0 ) / image.size ( );

	if ( displayDataType == RAW_COUNTS )
	{
		maxCounts[ imageNumber ].SetWindowTextA ( cstr ( currentStatPoint.maxv ) );
		minCounts[ imageNumber ].SetWindowTextA ( cstr ( currentStatPoint.minv ) );
		selCounts[ imageNumber ].SetWindowTextA ( cstr ( currentStatPoint.selv ) );
		avgCounts[ imageNumber ].SetWindowTextA ( cstr ( currentStatPoint.avgv, 5 ) );
		mostRecentStat = currentStatPoint;
	}
	else if ( displayDataType == CAMERA_PHOTONS )
	{
		statPoint camPoint;
		//double selPhotons, maxPhotons, minPhotons, avgPhotons;
		//if (eEMGainMode)
		if ( false )
		{
			camPoint = (currentStatPoint - convs.EMGain200BackgroundCount ) * convs.countToCameraPhotonEM200;
		}
		else
		{
			camPoint = ( currentStatPoint - convs.conventionalBackgroundCount ) * convs.countToCameraPhoton;
		}
		maxCounts[ imageNumber ].SetWindowTextA ( cstr ( camPoint.maxv, 1 ) );
		minCounts[ imageNumber ].SetWindowTextA ( cstr ( camPoint.minv, 1 ) );
		selCounts[ imageNumber ].SetWindowTextA ( cstr ( camPoint.selv, 1 ) );
		avgCounts[ imageNumber ].SetWindowTextA ( cstr ( camPoint.avgv, 1 ) );
		mostRecentStat = camPoint;
	}
	else if ( displayDataType == ATOM_PHOTONS )
	{
		statPoint atomPoint;
		//if (eEMGainMode)
		if ( false )
		{
			atomPoint = ( currentStatPoint - convs.EMGain200BackgroundCount ) * convs.countToScatteredPhotonEM200;
		}
		else
		{
			atomPoint = ( currentStatPoint - convs.conventionalBackgroundCount ) * convs.countToScatteredPhoton;
		}
		maxCounts[ imageNumber ].SetWindowTextA ( cstr ( atomPoint.maxv, 1 ) );
		minCounts[ imageNumber ].SetWindowTextA ( cstr ( atomPoint.minv, 1 ) );
		selCounts[ imageNumber ].SetWindowTextA ( cstr ( atomPoint.selv, 1 ) );
		avgCounts[ imageNumber ].SetWindowTextA ( cstr ( atomPoint.avgv, 1 ) );
		mostRecentStat = atomPoint;
	}
	
	return { currentStatPoint.minv, currentStatPoint.maxv };
}