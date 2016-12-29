#include "stdafx.h"
#include "PictureControl.h"

void PictureControl::handleScroll(int id, UINT nPos)
{
	if (id == this->sliderMax.ID)
	{
		sliderMax.SetPos(nPos);
		editMax.SetWindowTextA(std::to_string(nPos).c_str());
	}
	else if (id == this->sliderMin.ID)
	{
		sliderMin.SetPos(nPos);
		editMin.SetWindowTextA(std::to_string(nPos).c_str());
	}
	return;
}

void PictureControl::initialize(POINT& loc, CWnd* parent, int& id, int width, int height)
{
	if (width < 100)
	{
		throw std::invalid_argument("Pictures must be greater than 100 in width because this is the size of the max/min"
			"controls.");
	}
	if (height < 100)
	{
		throw std::invalid_argument("Pictures must be greater than 100 in height because this is the minimum height "
			"of the max/min controls.");
	}
	// this is important for the control to know where it should draw controls.
	this->backgroundArea = { loc.x, loc.y, loc.x + width, loc.y + height};
	// reserve some area for the texts.
	backgroundArea.right -= 100;
	loc.x += backgroundArea.right - backgroundArea.left;
	// "min" text
	labelMin.sPos = { loc.x, loc.y, loc.x + 50, loc.y + 30 };
	labelMin.ID = id++;
	labelMin.Create("MIN", WS_CHILD | WS_VISIBLE | SS_CENTER, labelMin.sPos, parent, labelMin.ID);
	labelMin.fontType = "Normal";
	// minimum number text
	editMin.sPos = { loc.x, loc.y + 30, loc.x + 50, loc.y + 60 };
	editMin.ID = id++;
	editMin.Create(WS_CHILD | WS_VISIBLE | SS_LEFT | ES_AUTOHSCROLL, editMin.sPos, parent, editMin.ID);
	editMin.SetWindowTextA("95");
	editMin.fontType = "Normal";
	// minimum slider
	sliderMin.sPos = { loc.x, loc.y + 60, loc.x + 50, loc.y + backgroundArea.bottom - backgroundArea.top};
	sliderMin.ID = id++;
	sliderMin.Create(WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_VERT, sliderMin.sPos, parent, sliderMin.ID);
	sliderMin.SetRange(0, 20000);
	sliderMin.SetPageSize((minSliderPosition - minSliderPosition)/10);
	// "max" text
	labelMax.sPos = { loc.x + 50, loc.y, loc.x + 100, loc.y + 30 };
	labelMax.ID = id++;
	labelMax.Create("MAX", WS_CHILD | WS_VISIBLE | SS_CENTER, labelMax.sPos, parent, labelMax.ID);
	labelMax.fontType = "Normal";
	// maximum number text
	editMax.sPos = { loc.x + 50, loc.y + 30, loc.x + 100, loc.y + 60 };
	editMax.ID = id++;
	editMax.Create(WS_CHILD | WS_VISIBLE | SS_LEFT | ES_AUTOHSCROLL, editMax.sPos, parent, editMax.ID);
	editMax.SetWindowTextA("395");
	editMax.fontType = "Normal";
	// maximum slider
	sliderMax.sPos = { loc.x + 50, loc.y + 60, loc.x + 100, loc.y + backgroundArea.bottom - backgroundArea.top};
	sliderMax.ID = id++;
	sliderMax.Create(WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_VERT, sliderMax.sPos, parent, sliderMax.ID);
	sliderMax.SetRange(0, 20000);
	sliderMax.SetPageSize((minSliderPosition - minSliderPosition) / 10);
	// reset this.
	loc.x -= backgroundArea.right - backgroundArea.left;
	return;
}

void PictureControl::updateGridSpecs(imageParameters newParameters)
{
	// not strictly necessary.
	grid.clear();
	//
	this->grid.resize(newParameters.width);
	for (int widthInc = 0; widthInc < grid.size(); widthInc++)
	{
		grid[widthInc].resize(newParameters.height);
		for (int heightInc = 0; heightInc < grid[widthInc].size(); heightInc++)
		{
			// for all 4 pictures...
			grid[widthInc][heightInc].left = (int)(backgroundArea.left
				+ (double)widthInc * (backgroundArea.right - backgroundArea.left) / (double)grid.size() + 2);
			grid[widthInc][heightInc].right = (int)(backgroundArea.left
				+ (double)(widthInc + 1) * (backgroundArea.right - backgroundArea.left) / (double)grid.size() + 2);
			grid[widthInc][heightInc].top = (int)(backgroundArea.top
				+ (double)(heightInc)* (backgroundArea.bottom - backgroundArea.top) / (double)grid[widthInc].size());
			grid[widthInc][heightInc].bottom = (int)(backgroundArea.top
				+ (double)(heightInc + 1)* (backgroundArea.bottom - backgroundArea.top) / (double)grid[widthInc].size());
		}
	}

}

void PictureControl::drawBitmap()
{

}

/*
 * recolor the box, clearing last run
 */
void PictureControl::drawBackground(CWnd* parent)
{	
	CDC* colorObj = parent->GetDC();
	colorObj->SelectObject(GetStockObject(DC_BRUSH));
	colorObj->SelectObject(GetStockObject(DC_PEN));
	// dark green brush
	colorObj->SetDCBrushColor(RGB(0, 10, 0));
	// Set the Pen to White
	colorObj->SetDCPenColor(RGB(255, 255, 255));
	// Drawing a rectangle with the current Device Context
	// (slightly larger than the image zone).
	RECT rectArea = { this->backgroundArea.left, backgroundArea.top, backgroundArea.right, backgroundArea.bottom};
	colorObj->Rectangle(&rectArea);
	parent->ReleaseDC(colorObj);
}

void PictureControl::drawGrid(CWnd* parent, CBrush* brush)
{
	CDC* easel = parent->GetDC();
	easel->SelectObject(GetStockObject(DC_BRUSH));
	easel->SetDCBrushColor(RGB(255, 255, 255));
	// draw rectangles indicating where the pixels are.
	for (int widthInc = 0; widthInc < grid.size(); widthInc++)
	{
		for (int heightInc = 0; heightInc < grid[widthInc].size(); heightInc++)
		{
			easel->FrameRect(&grid[widthInc][heightInc], brush);
		}
	}
	return;
}

void PictureControl::drawRectangle(CWnd* parent, CBrush* brush)
{

}

void PictureControl::drawCircle(CWnd* parent, CBrush* brush)
{

}

void PictureControl::rearrange(std::string cameraMode, std::string triggerMode, int width, int height, std::unordered_map<std::string, CFont*> fonts)
{
	editMax.rearrange(cameraMode, triggerMode, width, height, fonts);
	editMin.rearrange(cameraMode, triggerMode, width, height, fonts);
	labelMax.rearrange(cameraMode, triggerMode, width, height, fonts);
	labelMin.rearrange(cameraMode, triggerMode, width, height, fonts);
	sliderMax.rearrange(cameraMode, triggerMode, width, height, fonts);
	sliderMin.rearrange(cameraMode, triggerMode, width, height, fonts);
	this->backgroundArea.bottom *= height / 997;
	this->backgroundArea.top *= height / 997;
	this->backgroundArea.left *= width / 1920;
	this->backgroundArea.right *= width / 1920;
	// deal with draw areas & stuff...
	return;
}