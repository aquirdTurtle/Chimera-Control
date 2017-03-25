#include "stdafx.h"
#include "PictureStats.h"
#include "reorganizeControl.h"
#include "doubleToString.h"
#include <algorithm>
#include <numeric>

// as of right now, the position of this control is not affected by the mode or the trigger mode.
bool PictureStats::initialize( POINT& pos, CWnd* parent, int& id, std::unordered_map<std::string, CFont*> fonts,
							   std::vector<CToolTipCtrl*>& tooltips )
{
	pictureStatsHeader.sPos = { pos.x, pos.y, pos.x + 272, pos.y + 25 };
	pictureStatsHeader.ID = id++;
	pictureStatsHeader.Create( "Raw Counts", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_CENTER,
							   pictureStatsHeader.sPos, parent, pictureStatsHeader.ID );
	pictureStatsHeader.fontType = "Heading";
	pos.y += 25;
	/// CURRENT IMAGE DATA
	// Current Accumulation Number Display
	repetitionIndicator.sPos = { pos.x, pos.y, pos.x + 272, pos.y + 25 };
	repetitionIndicator.ID = id++;
	repetitionIndicator.Create( "Repetition ?/?", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_CENTER, repetitionIndicator.sPos,
								parent, repetitionIndicator.ID );
	repetitionIndicator.fontType = "Normal";
	pos.y += 25;
	/// Picture labels ////////////////////////////////////////////////////////////

	//ePictureText
	collumnHeaders[0].sPos = { pos.x, pos.y, pos.x + 54, pos.y + 25 };
	collumnHeaders[0].ID = id++;
	collumnHeaders[0].Create( "Pic:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, collumnHeaders[0].sPos, parent, collumnHeaders[0].ID );
	collumnHeaders[0].fontType = "Small";
	pos.y += 25;
	int inc = 0;
	for (auto& control : picNumberIndicators)
	{
		inc++;
		control.sPos = { pos.x, pos.y, pos.x + 54, pos.y + 25 };
		control.ID = id++;
		control.Create( ("#" + str( inc ) + ":").c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, control.sPos, parent, control.ID );
		control.fontType = "Small";
		pos.y += 25;
	}
	pos.y -= 125;

	/// Max Count Edits
	// Max Count Display 742 - 480 )/2 = 131 
	collumnHeaders[1].sPos = { pos.x + 54, pos.y, pos.x + 108, pos.y + 25 };
	collumnHeaders[1].ID = id++;
	collumnHeaders[1].Create( "Max:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, collumnHeaders[1].sPos,
							  parent, collumnHeaders[1].ID );
	collumnHeaders[1].fontType = "Small";
	pos.y += 25;
	// #1
	for (auto& control : maxCounts)
	{
		control.sPos = { pos.x + 54, pos.y, pos.x + 108, pos.y + 25 };
		control.ID = id++;
		control.Create( "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, control.sPos, parent, control.ID );
		control.fontType = "Small";
		pos.y += 25;
	}
	// back to top.
	pos.y -= 125;
	/// Min Counts
	// Min Count Display	
	collumnHeaders[2].sPos = { pos.x + 108, pos.y, pos.x + 162, pos.y + 25 };
	collumnHeaders[2].ID = id++;
	collumnHeaders[2].Create( "Min:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, collumnHeaders[2].sPos,
							  parent, collumnHeaders[2].ID );
	collumnHeaders[2].fontType = "Small";
	pos.y += 25;

	for (auto& control : minCounts)
	{
		control.sPos = { pos.x + 108, pos.y, pos.x + 162, pos.y + 25 };
		control.ID = id++;
		control.Create( "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, control.sPos, parent, control.ID );
		control.fontType = "Small";
		pos.y += 25;
	}
	pos.y -= 125;
	/// Average Counts
	collumnHeaders[3].sPos = { pos.x + 162, pos.y, pos.x + 216, pos.y + 25 };
	collumnHeaders[3].ID = id++;
	collumnHeaders[3].Create( "Avg:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, collumnHeaders[3].sPos,
							  parent, collumnHeaders[3].ID );
	collumnHeaders[3].fontType = "Small";
	pos.y += 25;
	// 
	for (auto& control : avgCounts)
	{
		control.sPos = { pos.x + 162, pos.y, pos.x + 216, pos.y + 25 };
		control.ID = id++;
		control.Create( "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, control.sPos, parent, control.ID );
		control.fontType = "Small";
		pos.y += 25;
	}

	pos.y -= 125;
	/// Selection Counts
	collumnHeaders[4].sPos = { pos.x + 216, pos.y, pos.x + 272, pos.y + 25 };
	collumnHeaders[4].ID = id++;
	collumnHeaders[4].Create( "Sel:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, collumnHeaders[4].sPos,
							  parent, collumnHeaders[4].ID );
	collumnHeaders[4].fontType = "Small";
	pos.y += 25;
	// #1
	for (auto& control : selCounts)
	{
		control.sPos = { pos.x + 216, pos.y, pos.x + 272, pos.y + 25 };
		control.ID = id++;
		control.Create( "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, control.sPos, parent, control.ID );
		control.fontType = "Small";
		pos.y += 25;
	}
	return true;
}

bool PictureStats::rearrange(std::string cameraMode, std::string trigMode, int width, int height, std::unordered_map<std::string, CFont*> fonts)
{
	pictureStatsHeader.rearrange(cameraMode, trigMode, width, height, fonts);
	repetitionIndicator.rearrange(cameraMode, trigMode, width, height, fonts);
	for (auto& control : collumnHeaders)
	{
		control.rearrange(cameraMode, trigMode, width, height, fonts);
	}
	for (auto& control : maxCounts)
	{
		control.rearrange(cameraMode, trigMode, width, height, fonts);
	}
	for (auto& control : minCounts)
	{
		control.rearrange(cameraMode, trigMode, width, height, fonts);
	}
	for (auto& control : picNumberIndicators)
	{
		control.rearrange(cameraMode, trigMode, width, height, fonts);
	}
	for (auto& control : selCounts)
	{
		control.rearrange(cameraMode, trigMode, width, height, fonts);
	}
	for (auto& control : avgCounts)
	{
		control.rearrange(cameraMode, trigMode, width, height, fonts);
	}
	return true;
}


bool PictureStats::reset()
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
	return true;
}

bool PictureStats::updateType(std::string typeText)
{
	pictureStatsHeader.SetWindowText(typeText.c_str());
	return true;
}



void PictureStats::update( std::vector<long> image, unsigned int imageNumber, std::pair<int, int> selectedPixel, int pictureWidth, 
						   int currentRepetitionNumber, int totalRepetitionCount)
{
	///
	repetitionIndicator.SetWindowTextA( ("Repetition " + str( currentRepetitionNumber ) + "/" + str( totalRepetitionCount )).c_str() );
	///
	long selCounts = image[selectedPixel.first + selectedPixel.second * pictureWidth];
	long maxCounts = 1;
	long minCounts = 65536;
	double avgCounts;
	// for all pixels... find the max and min of the picture.
	for (int pixelInc = 0; pixelInc < image.size(); pixelInc++)
	{
		try
		{
			if (image[pixelInc] > maxCounts)
			{
				maxCounts = image[pixelInc];
			}
			if (image[pixelInc] < minCounts)
			{
				minCounts = image[pixelInc];
			}
		}
		catch (std::out_of_range&)
		{
			errBox( "ERROR: caught std::out_of_range in this->drawDataWindow! experimentImagesInc = " + str( imageNumber )
					+ ", pixelInc = " + str( pixelInc ) + ", image.size() = " + str( image.size() ) + ". Attempting to continue..." );
			return;
		}
	}
	avgCounts = std::accumulate( image.begin(), image.end(), 0.0 ) / image.size();

	if (displayDataType == RAW_COUNTS)
	{
		this->maxCounts[imageNumber].SetWindowTextA(str(maxCounts).c_str());
		this->minCounts[imageNumber].SetWindowTextA(str(minCounts).c_str());
		this->selCounts[imageNumber].SetWindowTextA(str(selCounts).c_str());
		this->avgCounts[imageNumber].SetWindowTextA(doubleToString( avgCounts, 1).c_str());
	}
	else if (displayDataType == CAMERA_PHOTONS)
	{
		double selPhotons, maxPhotons, minPhotons, avgPhotons;
		//if (eEMGainMode)
		if (false)
		{
			selPhotons = (selCounts - convs.EMGain200BackgroundCount) * convs.countToCameraPhotonEM200;
			maxPhotons = (maxCounts - convs.EMGain200BackgroundCount) * convs.countToCameraPhotonEM200;
			minPhotons = (minCounts - convs.EMGain200BackgroundCount) * convs.countToCameraPhotonEM200;
			avgPhotons = (avgCounts - convs.EMGain200BackgroundCount) * convs.countToCameraPhotonEM200;
		}
		else
		{
			selPhotons = (selCounts - convs.conventionalBackgroundCount) * convs.countToCameraPhoton;
			maxPhotons = (maxCounts - convs.conventionalBackgroundCount) * convs.countToCameraPhoton;
			minPhotons = (minCounts - convs.conventionalBackgroundCount) * convs.countToCameraPhoton;
			avgPhotons = (avgCounts - convs.conventionalBackgroundCount) * convs.countToCameraPhoton;
		}
		this->maxCounts[imageNumber].SetWindowTextA(doubleToString(maxPhotons, 1).c_str());
		this->minCounts[imageNumber].SetWindowTextA(doubleToString(minPhotons, 1).c_str());
		this->selCounts[imageNumber].SetWindowTextA(doubleToString(selPhotons, 1).c_str());
		this->avgCounts[imageNumber].SetWindowTextA(doubleToString(avgPhotons, 1).c_str());
	}
	else if (this->displayDataType == ATOM_PHOTONS)
	{
		double selPhotons, maxPhotons, minPhotons, avgPhotons;
		//if (eEMGainMode)
		if (false)
		{
			selPhotons = (selCounts - convs.EMGain200BackgroundCount) * convs.countToScatteredPhotonEM200;
			maxPhotons = (maxCounts - convs.EMGain200BackgroundCount) * convs.countToScatteredPhotonEM200;
			minPhotons = (minCounts - convs.EMGain200BackgroundCount) * convs.countToScatteredPhotonEM200;
			avgPhotons = (avgCounts - convs.EMGain200BackgroundCount) * convs.countToScatteredPhotonEM200;
		}
		else
		{
			selPhotons = (selCounts - convs.conventionalBackgroundCount) * convs.countToScatteredPhoton;
			maxPhotons = (maxCounts - convs.conventionalBackgroundCount) * convs.countToScatteredPhoton;
			minPhotons = (minCounts - convs.conventionalBackgroundCount) * convs.countToScatteredPhoton;
			avgPhotons = (avgCounts - convs.conventionalBackgroundCount) * convs.countToScatteredPhoton;
		}
		this->maxCounts[imageNumber].SetWindowTextA(doubleToString(maxPhotons, 1).c_str());
		this->minCounts[imageNumber].SetWindowTextA(doubleToString(minPhotons, 1).c_str());
		this->selCounts[imageNumber].SetWindowTextA(doubleToString(selPhotons, 1).c_str());
		this->avgCounts[imageNumber].SetWindowTextA(doubleToString(avgPhotons, 1).c_str());
	}
	return;
}