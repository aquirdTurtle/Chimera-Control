#include "stdafx.h"
#include "PictureStats.h"
#include "reorganizeControl.h"
#include "doubleToString.h"

// as of right now, the position of this control is not affected by the mode or the trigger mode.
bool PictureStats::initialize(POINT& pos, CWnd* parent, int& id)
{
	this->pictureStatsHeader.sPos = { pos.x, pos.y, pos.x + 272, pos.y + 25 };
	pictureStatsHeader.ID = id++;
	pictureStatsHeader.Create("Raw Counts", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_CENTER,
		pictureStatsHeader.sPos, parent, pictureStatsHeader.ID);
	pictureStatsHeader.fontType = "Heading";
	pos.y += 25;
	/// CURRENT IMAGE DATA
	// Current Accumulation Number Display
	this->repetitionIndicator.sPos = {pos.x, pos.y, pos.x + 272, pos.y + 25};
	repetitionIndicator.ID = id++;
	repetitionIndicator.Create("Repetition ?/?", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_CENTER, repetitionIndicator.sPos,
		parent, repetitionIndicator.ID);
	repetitionIndicator.fontType = "Normal";
	pos.y += 25;
	/// Picture labels ////////////////////////////////////////////////////////////
	
	//ePictureText
	this->collumnHeaders[0].sPos = { pos.x, pos.y, pos.x + 54, pos.y + 25 };
	collumnHeaders[0].ID = id++;
	collumnHeaders[0].Create("Pic:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, collumnHeaders[0].sPos,
		parent, collumnHeaders[0].ID);
	collumnHeaders[0].fontType = "Small";
	pos.y += 25;
	int inc = 0;
	for (auto& control : picNumberIndicators)
	{
		inc++;
		control.sPos = { pos.x, pos.y, pos.x + 54, pos.y + 25 };
		control.ID = id++;
		control.Create(("#" + std::to_string(inc) + ":").c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, 
			control.sPos,	parent, control.ID);
		control.fontType = "Small";
		pos.y += 25;
	}
	pos.y -= 125;

	/// Max Count Edits
	// Max Count Display 742 - 480 )/2 = 131 
	this->collumnHeaders[1].sPos = { pos.x + 54, pos.y, pos.x + 108, pos.y + 25};
	collumnHeaders[1].ID = id++;
	collumnHeaders[1].Create("Max:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, collumnHeaders[1].sPos,
		parent, collumnHeaders[1].ID);
	collumnHeaders[1].fontType = "Small";
	pos.y += 25;
	// #1
	for (auto& control : this->maxCounts)
	{
		control.sPos = { pos.x + 54, pos.y, pos.x + 108, pos.y + 25 };
		control.ID = id++;
		control.Create("-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, control.sPos, parent, control.ID);
		control.fontType = "Small";
		pos.y += 25;
	}
	// back to top.
	pos.y -= 125;
	/// Min Counts
	// Min Count Display	
	this->collumnHeaders[2].sPos = { pos.x + 108, pos.y, pos.x + 162, pos.y + 25 };
	collumnHeaders[2].ID = id++;
	collumnHeaders[2].Create("Min:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, collumnHeaders[2].sPos,
		parent, collumnHeaders[2].ID);
	collumnHeaders[2].fontType = "Small";
	pos.y += 25;
	
	for (auto& control : this->minCounts)
	{
		control.sPos = { pos.x + 108, pos.y, pos.x + 162, pos.y + 25 };
		control.ID = id++;
		control.Create("-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, control.sPos, parent, control.ID);
		control.fontType = "Small";
		pos.y += 25;
	}
	pos.y -= 125;
	/// Average Counts
	this->collumnHeaders[3].sPos = { pos.x + 162, pos.y, pos.x + 216, pos.y + 25 };
	collumnHeaders[3].ID = id++;
	collumnHeaders[3].Create("Avg:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, collumnHeaders[3].sPos,
		parent, collumnHeaders[3].ID);
	collumnHeaders[3].fontType = "Small";
	pos.y += 25;	
	// 
	for (auto& control : this->avgCounts)
	{
		control.sPos = { pos.x + 162, pos.y, pos.x + 216, pos.y + 25 };
		control.ID = id++;
		control.Create("-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, control.sPos, parent, control.ID);
		control.fontType = "Small";
		pos.y += 25;
	}

	pos.y -= 125;
	/// Selection Counts
	this->collumnHeaders[4].sPos = { pos.x + 216, pos.y, pos.x + 272, pos.y + 25 };
	collumnHeaders[4].ID = id++;
	collumnHeaders[4].Create("Sel:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, collumnHeaders[4].sPos,
		parent, collumnHeaders[4].ID);
	collumnHeaders[4].fontType = "Small";
	pos.y += 25;
	// #1
	for (auto& control : selCounts)
	{
		control.sPos = { pos.x + 216, pos.y, pos.x + 272, pos.y + 25 };
		control.ID = id++;
		control.Create("-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY, control.sPos, parent, control.ID);
		control.fontType = "Small";
		pos.y += 25;
	}
	return true;
}

bool PictureStats::reorganizeControls(std::string cameraMode, std::string trigMode, int width, int height)
{
	pictureStatsHeader.rearrange(cameraMode, trigMode, width, height);
	repetitionIndicator.rearrange(cameraMode, trigMode, width, height);
	for (auto& control : this->collumnHeaders)
	{
		control.rearrange(cameraMode, trigMode, width, height);
	}
	for (auto& control : this->maxCounts)
	{
		control.rearrange(cameraMode, trigMode, width, height);
	}
	for (auto& control : this->minCounts)
	{
		control.rearrange(cameraMode, trigMode, width, height);
	}
	for (auto& control : this->picNumberIndicators)
	{
		control.rearrange(cameraMode, trigMode, width, height);
	}
	for (auto& control : this->selCounts)
	{
		control.rearrange(cameraMode, trigMode, width, height);
	}
	for (auto& control : this->avgCounts)
	{
		control.rearrange(cameraMode, trigMode, width, height);
	}
	return true;
}
bool PictureStats::reset()
{
	for (auto& control : this->maxCounts)
	{
		control.SetWindowText("-");
	}
	for (auto& control : this->minCounts)
	{
		control.SetWindowText("-");
	}
	for (auto& control : this->avgCounts)
	{
		control.SetWindowText("-");
	}
	for (auto& control : this->selCounts)
	{
		control.SetWindowText("-");
	}
	return true;
}

bool PictureStats::updateType(std::string typeText)
{
	pictureStatsHeader.SetWindowText(typeText.c_str());
	return true;
}


bool PictureStats::update(unsigned long selCounts, unsigned long maxCounts, unsigned long minCounts, double avgCounts, unsigned int image)
{
	if (this->displayDataType == RAW_COUNTS)
	{
		this->maxCounts[image].SetWindowTextA(std::to_string(maxCounts).c_str());
		this->minCounts[image].SetWindowTextA(std::to_string(minCounts).c_str());
		this->selCounts[image].SetWindowTextA(std::to_string(selCounts).c_str());
		this->avgCounts[image].SetWindowTextA(doubleToString(avgCounts, 1).c_str());
	}
	else if (this->displayDataType == CAMERA_PHOTONS)
	{
		double selPhotons, maxPhotons, minPhotons, avgPhotons;
		//if (eEMGainMode)
		if (false)
		{
			selPhotons = (selCounts - convs.EMGain200BackgroundCount) * this->convs.countToCameraPhotonEM200;
			maxPhotons = (maxCounts - convs.EMGain200BackgroundCount) * this->convs.countToCameraPhotonEM200;
			minPhotons = (minCounts - convs.EMGain200BackgroundCount) * this->convs.countToCameraPhotonEM200;
			avgPhotons = (avgCounts - convs.EMGain200BackgroundCount) * this->convs.countToCameraPhotonEM200;
		}
		else
		{
			selPhotons = (selCounts - convs.conventionalBackgroundCount) * this->convs.countToCameraPhoton;
			maxPhotons = (maxCounts - convs.conventionalBackgroundCount) * this->convs.countToCameraPhoton;
			minPhotons = (minCounts - convs.conventionalBackgroundCount) * this->convs.countToCameraPhoton;
			avgPhotons = (avgCounts - convs.conventionalBackgroundCount) * this->convs.countToCameraPhoton;
		}
		this->maxCounts[image].SetWindowTextA(doubleToString(maxPhotons, 1).c_str());
		this->minCounts[image].SetWindowTextA(doubleToString(minPhotons, 1).c_str());
		this->selCounts[image].SetWindowTextA(doubleToString(selPhotons, 1).c_str());
		this->avgCounts[image].SetWindowTextA(doubleToString(avgPhotons, 1).c_str());
	}
	else if (this->displayDataType == ATOM_PHOTONS)
	{
		double selPhotons, maxPhotons, minPhotons, avgPhotons;
		//if (eEMGainMode)
		if (false)
		{
			selPhotons = (selCounts - convs.EMGain200BackgroundCount) * this->convs.countToScatteredPhotonEM200;
			maxPhotons = (maxCounts - convs.EMGain200BackgroundCount) * this->convs.countToScatteredPhotonEM200;
			minPhotons = (minCounts - convs.EMGain200BackgroundCount) * this->convs.countToScatteredPhotonEM200;
			avgPhotons = (avgCounts - convs.EMGain200BackgroundCount) * this->convs.countToScatteredPhotonEM200;
		}
		else
		{
			selPhotons = (selCounts - convs.conventionalBackgroundCount) * this->convs.countToScatteredPhoton;
			maxPhotons = (maxCounts - convs.conventionalBackgroundCount) * this->convs.countToScatteredPhoton;
			minPhotons = (minCounts - convs.conventionalBackgroundCount) * this->convs.countToScatteredPhoton;
			avgPhotons = (avgCounts - convs.conventionalBackgroundCount) * this->convs.countToScatteredPhoton;
		}
		this->maxCounts[image].SetWindowTextA(doubleToString(maxPhotons, 1).c_str());
		this->minCounts[image].SetWindowTextA(doubleToString(minPhotons, 1).c_str());
		this->selCounts[image].SetWindowTextA(doubleToString(selPhotons, 1).c_str());
		this->avgCounts[image].SetWindowTextA(doubleToString(avgPhotons, 1).c_str());
	}
	return true;
}