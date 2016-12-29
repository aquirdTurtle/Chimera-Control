#include "stdafx.h"
#include "PictureManager.h"

void PictureManager::handleScroll(UINT nSBCode, UINT nPos, CScrollBar* scrollbar)
{
	if (nSBCode == SB_THUMBPOSITION || nSBCode == SB_THUMBTRACK)
	{
		int id = scrollbar->GetDlgCtrlID();
		for (auto& control : pictures)
		{
			control.handleScroll(id, nPos);
		}
	}
	return;
}

void PictureManager::initialize(POINT& loc, CWnd* parent, int& id, std::unordered_map<std::string, CFont*> fonts,
	std::vector<CToolTipCtrl*>& tooltips)
{
	this->pictures[0].initialize(loc, parent, id, 570, 460);
	loc.x += 570;
	this->pictures[1].initialize(loc, parent, id, 570, 460);
	loc.x -= 570;
	// leave space for timer.
	loc.y += 500;
	this->pictures[2].initialize(loc, parent, id, 570, 460);
	loc.x += 570;
	this->pictures[3].initialize(loc, parent, id, 570, 460);
}

void PictureManager::refreshBackgrounds(CWnd* parent)
{
	for (auto& picture : this->pictures)
	{
		picture.drawBackground(parent);
	}
	return;
}

void PictureManager::drawGrids(CWnd* parent, CBrush* brush)
{
	for (auto& picture : this->pictures)
	{
		picture.drawGrid(parent, brush);
	}
}

void PictureManager::setParameters(imageParameters parameters)
{
	for (auto& picture : this->pictures)
	{
		picture.updateGridSpecs(parameters);
	}
}

void PictureManager::rearrange(std::string cameraMode, std::string triggerMode, int width, int height, std::unordered_map<std::string, CFont*> fonts)
{
	for (auto& control : this->pictures)
	{
		control.rearrange(cameraMode, triggerMode, width, height, fonts);
	}
}
