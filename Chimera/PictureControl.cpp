﻿#include "stdafx.h"
#include "PictureControl.h"


bool PictureControl::isActive()
{
	return active;
}


void PictureControl::setSliderPositions(UINT min, UINT max)
{
	sliderMax.SetPos(max);
	sliderMin.SetPos(min);
}


/*
 * Used during initialization & when used when transitioning between 1 and >1 pictures per repetition. 
 * Sets the unscaled background area and the scaled area.
 */
void PictureControl::setPictureArea( POINT loc, int width, int height )
{
	// this is important for the control to know where it should draw controls.
	unscaledBackgroundArea = { loc.x, loc.y, loc.x + width, loc.y + height };
	// reserve some area for the texts.
	unscaledBackgroundArea.right -= 100;
	scaledBackgroundArea = unscaledBackgroundArea;
	scaledBackgroundArea.left *= width;
	scaledBackgroundArea.right *= width;
	scaledBackgroundArea.top *= height;
	scaledBackgroundArea.bottom *= height;

	double widthPicScale;
	double heightPicScale;
	if (unofficialImageParameters.width > unofficialImageParameters.height)
	{
		widthPicScale = 1;
		heightPicScale = double(unofficialImageParameters.height) / unofficialImageParameters.width;
	}
	else
	{
		heightPicScale = 1;
		widthPicScale = double(unofficialImageParameters.width) / unofficialImageParameters.height;
	}
	ULONG picWidth = ULONG( (scaledBackgroundArea.right - scaledBackgroundArea.left)*widthPicScale );
	ULONG picHeight = scaledBackgroundArea.bottom - scaledBackgroundArea.top;
	POINT mid = { (scaledBackgroundArea.left + scaledBackgroundArea.right) / 2,
				  (scaledBackgroundArea.top + scaledBackgroundArea.bottom) / 2 };
	pictureArea.left = mid.x - picWidth / 2;
	pictureArea.right = mid.x + picWidth / 2;
	pictureArea.top = mid.y - picHeight / 2;
	pictureArea.bottom = mid.y + picHeight / 2;
}


/* used when transitioning between single and multiple pictures. It sets it based on the background size, so make 
 * sure to change the background size before using this.
 * ********/
void PictureControl::setSliderLocations(CWnd* parent)
{
	CRect rect;
	parent->GetWindowRect(&rect);
	long width = rect.right - rect.left;
	long height = rect.bottom - rect.top;
	double widthScale = width / 1920.0;
	double heightScale = height / 997.0;
	widthScale = 1;
	heightScale = 1;
	POINT loc = { long(unscaledBackgroundArea.right * widthScale), long(unscaledBackgroundArea.top * heightScale) };
	long collumnWidth = long(50 * widthScale);
	long blockHeight = long(30 * heightScale);
	labelMin.sPos = { loc.x, loc.y, loc.x + collumnWidth , loc.y + blockHeight };
	// minimum number text
	editMin.sPos = { loc.x, loc.y + blockHeight, loc.x + collumnWidth , loc.y + 2* blockHeight };
	// minimum slider
	sliderMin.sPos = { loc.x, loc.y + 2* blockHeight, loc.x + collumnWidth ,
					   long(loc.y + (unscaledBackgroundArea.bottom - unscaledBackgroundArea.top) * heightScale) };
	// "max" text
	loc.x += collumnWidth;
	labelMax.sPos = { loc.x, loc.y, loc.x + collumnWidth , loc.y + blockHeight };
	// maximum number text
	editMax.sPos = { loc.x, loc.y + blockHeight, loc.x + collumnWidth , loc.y + 2* blockHeight };
	// maximum slider
	sliderMax.sPos = { loc.x, loc.y + 2* blockHeight, loc.x + collumnWidth ,
					   long(loc.y + (unscaledBackgroundArea.bottom - unscaledBackgroundArea.top)*heightScale) };
}


/*
 * Called in order to see if a right click is above a camera pixel. Returns coordinates of the camera pixel.
 */
std::pair<int, int> PictureControl::checkClickLocation( CPoint clickLocation )
{
	CPoint test;
	for (UINT horizontalInc = 0; horizontalInc < grid.size(); horizontalInc++)
	{
		for (UINT verticalInc = 0; verticalInc < grid[horizontalInc].size(); verticalInc++)
		{
			RECT relevantRect = grid[horizontalInc][verticalInc];
			// check if inside box
			if (clickLocation.x <= relevantRect.right && clickLocation.x >= relevantRect.left
				 && clickLocation.y <= relevantRect.bottom && clickLocation.y >= relevantRect.top)
			{
				return { horizontalInc , verticalInc };
				// then click was inside a box so this should do something.
			}
		}
	}
	// null result. only first number is checked.
	return { -1, -1 };
}


/*
 * change the colormap used for a given picture.
 */
void PictureControl::updatePalette( HPALETTE palette )
{
	imagePalette = palette;
}


/*
 * called when the user changes either the min or max edit.
 */
void PictureControl::handleEditChange( int id )
{
	if (!editMax)
	{
		return;
	}
	if (id == editMax.GetDlgCtrlID())
	{
		int max;
		CString tempStr;
		editMax.GetWindowTextA(tempStr);
		try
		{
			max = std::stoi( str(tempStr) );
		}
		catch (std::invalid_argument&)
		{
			thrower( "Please enter an integer." ); 
		}
		sliderMax.SetPos( max );
		maxSliderPosition = max;
	}
	if (id == editMin.GetDlgCtrlID())
	{
		int min;
		CString tempStr;
		editMin.GetWindowTextA(tempStr);
		try
		{
			min = std::stoi( str(tempStr) );
		}
		catch (std::invalid_argument&)
		{
			thrower( "Please enter an integer." );
		}
		sliderMin.SetPos( min );
		minSliderPosition = min;
	}
}


std::pair<UINT, UINT> PictureControl::getSliderLocations()
{
	return { sliderMin.GetPos(), sliderMax.GetPos() };
}


/*
 * called when the user drags the scroll bar.
 */
void PictureControl::handleScroll(int id, UINT nPos)
{
	if (id == sliderMax.GetDlgCtrlID())
	{
		sliderMax.SetPos(nPos);
		editMax.SetWindowTextA(cstr(nPos));
		maxSliderPosition = nPos;
	}
	else if (id == sliderMin.GetDlgCtrlID())
	{
		sliderMin.SetPos(nPos);
		editMin.SetWindowTextA(cstr(nPos));
		minSliderPosition = nPos;
	}
}

/*
 * initialize all controls associated with single picture.
 */
void PictureControl::initialize(POINT& loc, CWnd* parent, int& id, int width, int height, std::array<UINT, 2> minMaxIds)
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

	setPictureArea( loc, width, height );

	loc.x += unscaledBackgroundArea.right - unscaledBackgroundArea.left;
	// "min" text
	labelMin.sPos = { loc.x, loc.y, loc.x + 50, loc.y + 30 };
	labelMin.Create("MIN", WS_CHILD | WS_VISIBLE | SS_CENTER, labelMin.sPos, parent, id++ );
	// minimum number text
	editMin.sPos = { loc.x, loc.y + 30, loc.x + 50, loc.y + 60 };
	editMin.Create(WS_CHILD | WS_VISIBLE | SS_LEFT | ES_AUTOHSCROLL, editMin.sPos, parent, minMaxIds[0] );
	// minimum slider
	sliderMin.sPos = { loc.x, loc.y + 60, loc.x + 50, loc.y + unscaledBackgroundArea.bottom - unscaledBackgroundArea.top};
	sliderMin.Create(WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_VERT, sliderMin.sPos, parent, id++ );
	sliderMin.SetRange(0, 2000);
	sliderMin.SetPageSize(UINT((minSliderPosition - minSliderPosition)/10.0));
	// "max" text
	labelMax.sPos = { loc.x + 50, loc.y, loc.x + 100, loc.y + 30 };
	labelMax.Create("MAX", WS_CHILD | WS_VISIBLE | SS_CENTER, labelMax.sPos, parent, id++ );
	// maximum number text
	editMax.sPos = { loc.x + 50, loc.y + 30, loc.x + 100, loc.y + 60 };
	editMax.Create(WS_CHILD | WS_VISIBLE | SS_LEFT | ES_AUTOHSCROLL, editMax.sPos, parent, minMaxIds[1] );
	// maximum slider
	sliderMax.sPos = { loc.x + 50, loc.y + 60, loc.x + 100, loc.y + unscaledBackgroundArea.bottom - unscaledBackgroundArea.top};
	sliderMax.Create(WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_VERT, sliderMax.sPos, parent, id++ );
	sliderMax.SetRange(0, 2000);
	sliderMax.SetPageSize( int((minSliderPosition - minSliderPosition) / 10.0));
	// reset this.
	loc.x -= unscaledBackgroundArea.right - unscaledBackgroundArea.left;
	// manually scroll the objects to initial positions.
	handleScroll( sliderMin.GetDlgCtrlID(), 95);
	handleScroll( sliderMax.GetDlgCtrlID(), 395);
}


/*
 * Recalculate the grid of pixels, which needs to be done e.g. when changing number of pictures or re-sizing the 
 * picture. Does not draw the grid.
 */
void PictureControl::recalculateGrid(imageParameters newParameters)
{
	// not strictly necessary.
	grid.clear();
	// find the maximum dimension.
	unofficialImageParameters = newParameters;
	double widthPicScale;
	double heightPicScale;
	if (unofficialImageParameters.width > unofficialImageParameters.height)
	{
		widthPicScale = 1;
		heightPicScale = double(unofficialImageParameters.height) / unofficialImageParameters.width;
	}
	else
	{
		heightPicScale = 1;
		widthPicScale = double(unofficialImageParameters.width) / unofficialImageParameters.height;
	}
	long width = long((scaledBackgroundArea.right - scaledBackgroundArea.left)*widthPicScale);
	long height = long((scaledBackgroundArea.bottom - scaledBackgroundArea.top)*heightPicScale);
	POINT mid = { (scaledBackgroundArea.left + scaledBackgroundArea.right) / 2,
				  (scaledBackgroundArea.top + scaledBackgroundArea.bottom) / 2 };
	pictureArea.left = mid.x - width / 2;
	pictureArea.right = mid.x + width / 2;
	pictureArea.top = mid.y - height / 2;
	pictureArea.bottom = mid.y + height / 2;
	//

	grid.resize(newParameters.width);
	for (UINT widthInc = 0; widthInc < grid.size(); widthInc++)
	{
		grid[widthInc].resize(newParameters.height);
		for (UINT heightInc = 0; heightInc < grid[widthInc].size(); heightInc++)
		{
			// for all 4 pictures...
			grid[widthInc][heightInc].left = (int)(pictureArea.left
				+ (double)widthInc * (pictureArea.right - pictureArea.left) / (double)grid.size() + 2);
			grid[widthInc][heightInc].right = (int)(pictureArea.left
				+ (double)(widthInc + 1) * (pictureArea.right - pictureArea.left) / (double)grid.size() + 2);
			grid[widthInc][heightInc].top = (int)(pictureArea.top
				+ (double)(heightInc)* (pictureArea.bottom - pictureArea.top) / (double)grid[widthInc].size());
			grid[widthInc][heightInc].bottom = (int)(pictureArea.top
				+ (double)(heightInc + 1)* (pictureArea.bottom - pictureArea.top) / (double)grid[widthInc].size());
		}
	}
}

/* 
 * sets the state of the picture and changes visibility of controls depending on that state.
 */
void PictureControl::setActive( bool activeState )
{
	active = activeState;
	if (!active)
	{
		sliderMax.ShowWindow(SW_HIDE);
		sliderMin.ShowWindow(SW_HIDE);
		//
		labelMax.ShowWindow(SW_HIDE);
		labelMin.ShowWindow(SW_HIDE);
		//
		editMax.ShowWindow(SW_HIDE);
		editMin.ShowWindow(SW_HIDE);
	}
	else
	{
		sliderMax.ShowWindow(SW_SHOW);
		sliderMin.ShowWindow(SW_SHOW);
		//
		labelMax.ShowWindow(SW_SHOW);
		labelMin.ShowWindow(SW_SHOW);
		//
		editMax.ShowWindow(SW_SHOW);
		editMin.ShowWindow(SW_SHOW);
	}
}

/*
 * redraws the background and image. 
 */
void PictureControl::redrawImage( CDC* easel)
{
	drawBackground(easel);
	if (active && mostRecentImage.size() != 0)
	{
		drawPicture(easel, mostRecentImage, mostRecentAutoscaleInfo, mostRecentSpecialMinSetting,
					mostRecentSpecialMaxSetting );
	}

	// TODO?
	// drawGrid(parent, brush);
}

void PictureControl::resetStorage()
{
	mostRecentImage = std::vector<long>{};
}
/*
 * draw the picture that the camera took. The camera's data is inputted as a 1D vector of long here. The control needs
 * the camera window context since there's no direct control associated with the picture itself. Could probably change 
 * that.
 */
void PictureControl::drawPicture(CDC* deviceContext, std::vector<long> picData, 
								 std::tuple<bool, int/*min*/, int/*max*/> autoScaleInfo, bool specialMin, 
								 bool specialMax)
{
	mostRecentImage = picData;
	float yscale;
	long colorRange;
	long minColor;
	// first element containst whether autoscaling or not.
	if (std::get<0>(autoScaleInfo))
	{
		// third element contains max, second contains min.
		colorRange = std::get<2>(autoScaleInfo) - std::get<1>(autoScaleInfo);
		minColor = std::get<1>(autoScaleInfo);
	}
	else
	{
		colorRange = maxSliderPosition - minSliderPosition;
		minColor = minSliderPosition;
	}

	mostRecentAutoscaleInfo = autoScaleInfo;
	mostRecentSpecialMinSetting = specialMin;
	mostRecentSpecialMaxSetting = specialMax;


	double dTemp = 1;
	int pixelsAreaWidth;
	int pixelsAreaHeight;
	int dataWidth, dataHeight;
	int iTemp;
	PBITMAPINFO pbmi;
	WORD argbq[PICTURE_PALETTE_SIZE];
	BYTE *DataArray;
	// Rotated
	SelectPalette( deviceContext->GetSafeHdc(), (HPALETTE)this->imagePalette, true );
	RealizePalette( deviceContext->GetSafeHdc() );

	pixelsAreaWidth = pictureArea.right - pictureArea.left + 1;
	pixelsAreaHeight = pictureArea.bottom - pictureArea.top + 1;
	
	dataWidth = grid.size();
	// assumes non-zero size...
	dataHeight = grid[0].size();

	// imageBoxWidth must be a multiple of 4, otherwise StretchDIBits has problems apparently T.T
	if (pixelsAreaWidth % 4)
	{
		pixelsAreaWidth += (4 - pixelsAreaWidth % 4);
	}

	yscale = (256.0f) / (float)colorRange;

	for (int paletteIndex = 0; paletteIndex < PICTURE_PALETTE_SIZE; paletteIndex++)
	{
		argbq[paletteIndex] = (WORD)paletteIndex;
	}

	//hloc = LocalAlloc(LMEM_ZEROINIT | LMEM_MOVEABLE, sizeof(BITMAPINFOHEADER) + (sizeof(WORD)*PICTURE_PALETTE_SIZE));
	//hloc = LocalAlloc( LMEM_ZEROINIT | LMEM_MOVEABLE, sizeof( pbmi ) );
	//hloc = LocalAlloc( LMEM_ZEROINIT | LMEM_MOVEABLE, sizeof( BITMAPINFOHEADER ));
	//pbmi = (PBITMAPINFO)LocalLock(hloc);
	pbmi = (PBITMAPINFO)LocalAlloc( LPTR,
									sizeof( BITMAPINFOHEADER ) +
									sizeof( RGBQUAD ) * (1 << 8) );
	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biPlanes = 1;
	pbmi->bmiHeader.biBitCount = 8;
	pbmi->bmiHeader.biCompression = BI_RGB;
	pbmi->bmiHeader.biClrUsed = PICTURE_PALETTE_SIZE;
	pbmi->bmiHeader.biSizeImage = 0;// ((pbmi->bmiHeader.biWidth * 8 + 31) & ~31) / 8 * pbmi->bmiHeader.biHeight;

	pbmi->bmiHeader.biHeight = dataHeight;
	memcpy(pbmi->bmiColors, argbq, sizeof(WORD) * PICTURE_PALETTE_SIZE);

	//errBox( str( sizeof( DataArray ) / sizeof( DataArray[0] ) ) );
	//DataArray = (BYTE*)malloc(dataWidth * dataHeight * sizeof(BYTE));
	//memset(DataArray, 0, dataWidth * dataHeight);

	DataArray = (BYTE*)malloc( (dataWidth * dataHeight) * sizeof( BYTE ) );
	memset( DataArray, 255, (dataWidth * dataHeight) * sizeof( BYTE ) );
	for (int heightInc = 0; heightInc < dataHeight; heightInc++)
	{
		for (int widthInc = 0; widthInc < dataWidth; widthInc++)
		{
			// get temporary value for color of the pixel.

			dTemp = ceil(yscale * (picData[widthInc + heightInc * dataWidth] - minColor));

			// interpret the value depending on the range of values it can take.
			if (dTemp < 1)
			{
				// raise value to zero which is the floor of values this parameter can take.
				if (specialMin)
				{
					// the absolute lowest color is a special color that doesn't match the rest of the pallete. 
					// Typically a bright blue.
					iTemp = 0;
				}
				else
				{
					iTemp = 1;
				}
			}
			else if (dTemp > PICTURE_PALETTE_SIZE - 2)
			{
				// round to maximum value.
				if (specialMax)
				{
					// the absolute highest color is a special color that doesn't match the rest of the pallete.
					// typically a bright red.
					iTemp = PICTURE_PALETTE_SIZE - 1;
				}
				else
				{
					iTemp = PICTURE_PALETTE_SIZE - 2;
				}
			}
			else
			{
				// no rounding or flooring to min or max needed.
				iTemp = (int)dTemp;
			}
			// store the value.
			DataArray[widthInc + heightInc * dataWidth] = (BYTE)iTemp;
		}
	}
	SetStretchBltMode( deviceContext->GetSafeHdc(), COLORONCOLOR );
	// eCurrentAccumulationNumber starts at 1.
	BYTE *finalDataArray = NULL;
	switch (dataWidth)
	{
		case 0:
		{
			
			//pixelsAreaHeight -= 1;
			pbmi->bmiHeader.biWidth = dataWidth;
			pbmi->bmiHeader.biSizeImage = 1;// pbmi->bmiHeader.biWidth * pbmi->bmiHeader.biHeight;// * sizeof( BYTE );
			//memset( DataArray, 0, (dataWidth*dataHeight) * sizeof( *DataArray ) );
			StretchDIBits( deviceContext->GetSafeHdc(), pictureArea.left, pictureArea.top,
						   pixelsAreaWidth, pixelsAreaHeight, 0, 0, dataWidth,
						   dataHeight, DataArray, (BITMAPINFO FAR*)pbmi, DIB_PAL_COLORS, SRCCOPY );
			break;
		}
		case 2:
		{
			// make array that is twice as long.
			finalDataArray = (BYTE*)malloc(dataWidth * dataHeight * 2);
			memset(finalDataArray, 255, dataWidth * dataHeight * 2);

			for (int dataInc = 0; dataInc < dataWidth * dataHeight; dataInc++)
			{
				finalDataArray[2 * dataInc] = DataArray[dataInc];
				finalDataArray[2 * dataInc + 1] = DataArray[dataInc];
			}
			pbmi->bmiHeader.biWidth = dataWidth * 2;
			StretchDIBits( *deviceContext, pictureArea.left, pictureArea.top, pixelsAreaWidth, pixelsAreaHeight, 0, 0, dataWidth * 2, dataHeight,
						   finalDataArray, (BITMAPINFO FAR*)pbmi, DIB_PAL_COLORS, SRCCOPY );
			free(finalDataArray);
			break;
		}
		default:
		{
			// make array that is 4X as long.
			finalDataArray = (BYTE*)malloc(dataWidth * dataHeight * 4);
			memset(finalDataArray, 255, dataWidth * dataHeight * 4);
			for (int dataInc = 0; dataInc < dataWidth * dataHeight; dataInc++)
			{
				int data = DataArray[dataInc];
				finalDataArray[4 * dataInc] = data;
				finalDataArray[4 * dataInc + 1] = data;
				finalDataArray[4 * dataInc + 2] = data;
				finalDataArray[4 * dataInc + 3] = data;
			}
			pbmi->bmiHeader.biWidth = dataWidth * 4;
			StretchDIBits( *deviceContext, pictureArea.left, pictureArea.top, pixelsAreaWidth, pixelsAreaHeight, 0, 0, dataWidth * 4, dataHeight,
						   finalDataArray, (BITMAPINFO FAR*)pbmi, DIB_PAL_COLORS, SRCCOPY );
			free(finalDataArray);
			break;
		}
	}
	delete[] DataArray;
}


/*
 * recolor the background box, clearing last run.
 */
void PictureControl::drawBackground(CDC* easel)
{	
	easel->SelectObject(GetStockObject(DC_BRUSH));
	easel->SelectObject(GetStockObject(DC_PEN));
	// dark green brush
	easel->SetDCBrushColor(RGB(0, 10, 0));
	// Set the Pen to White
	easel->SetDCPenColor(RGB(255, 255, 255));
	// Drawing a rectangle with the current Device Context
	// (slightly larger than the image zone).
	RECT rectArea = { scaledBackgroundArea.left, scaledBackgroundArea.top, scaledBackgroundArea.right, 
					  scaledBackgroundArea.bottom };
	easel->Rectangle(&rectArea);
}

/* 
 * draw the grid which outlines where each pixel is.  Especially needs to be done when selecting pixels and no picture
 * is displayed. 
 */
void PictureControl::drawGrid(CDC* easel, CBrush* brush)
{
	if (!active)
	{
		return;
	}

	if (grid.size() != 0)
	{
		// hard set to 5000. Could easily change this to be able to see finer grids.
		// Tested before and 5000 seems reasonable.
		if (grid.size() * grid.front().size() > 5000)
		{
			return;
		}
	}
	easel->SelectObject(GetStockObject(DC_BRUSH));
	easel->SetDCBrushColor(RGB(255, 255, 255));
	// draw rectangles indicating where the pixels are.
	for (UINT widthInc = 0; widthInc < grid.size(); widthInc++)
	{
		for (UINT heightInc = 0; heightInc < grid[widthInc].size(); heightInc++)
		{
			easel->FrameRect(&grid[widthInc][heightInc], brush);
		}
	}
}

/*
 * draws the circle which denotes the selected pixel that the user wants to know the counts for. 
 */
void PictureControl::drawCircle(CDC* dc, std::pair<int, int> selectedLocation)
{
	if (grid.size() == 0)
	{
		// this hasn't been set yet, presumably this got called by the camera window as the camera window
		// was drawing itself before the control was initialized.
		return;
	}
	if (!active)
	{
		// don't draw anything if the window isn't active.
		return;
	}

	RECT smallRect;
	RECT relevantRect = grid[selectedLocation.first][selectedLocation.second];
	smallRect.left = relevantRect.left + long(7.0 * (relevantRect.right - relevantRect.left) / 16.0);
	smallRect.right = relevantRect.left + long( 9.0 * (relevantRect.right - relevantRect.left) / 16.0);
	smallRect.top = relevantRect.top + long( 7.0 * (relevantRect.bottom - relevantRect.top) / 16.0);
	smallRect.bottom = relevantRect.top + long( 9.0 * (relevantRect.bottom - relevantRect.top) / 16.0);
	// get appropriate brush and pen
	if (dc == NULL)
	{
		thrower("dc was null!");
	}
	dc->SelectObject( GetStockObject( HOLLOW_BRUSH ) );
	dc->SelectObject( GetStockObject( DC_PEN ) );
	
	if (colorIndicator == 0 || colorIndicator == 2)
	{
		dc->SetDCPenColor( RGB( 255, 0, 0 ) );
		dc->Ellipse( relevantRect.left, relevantRect.top, relevantRect.right, relevantRect.bottom );
		dc->SelectObject( GetStockObject( DC_BRUSH ) );
		dc->SetDCBrushColor( RGB( 255, 0, 0 ) );
	}
	else
	{
		dc->SetDCPenColor( RGB( 0, 255, 0 ) );
		dc->Ellipse( relevantRect.left, relevantRect.top, relevantRect.right, relevantRect.bottom );
		dc->SelectObject( GetStockObject( DC_BRUSH ) );
		dc->SetDCBrushColor( RGB( 0, 255, 0 ) );
	}
	dc->Ellipse( smallRect.left, smallRect.top, smallRect.right, smallRect.bottom );
}

void PictureControl::drawAnalysisMarkers(CDC* dc, std::vector<std::pair<UINT, UINT>> analysisLocs)
{
	if (active)
	{
		// draw and set.
		HPEN crossPen;
		crossPen = CreatePen(0, 1, RGB(255, 0, 0));
		
		dc->SelectObject(crossPen);

		RECT smallRect;
		RECT relevantRect;
		UINT count = 1;
		for (auto loc : analysisLocs)
		{
			relevantRect = grid[loc.first][loc.second];
			smallRect.left = relevantRect.left + long( 7.0 * (relevantRect.right - relevantRect.left) / 16.0);
			smallRect.right = relevantRect.left + long( 9.0 * (relevantRect.right - relevantRect.left) / 16.0);
			smallRect.top = relevantRect.top + long( 7.0 * (relevantRect.bottom - relevantRect.top) / 16.0);
			smallRect.bottom = relevantRect.top + long( 9.0 * (relevantRect.bottom - relevantRect.top) / 16.0);

			dc->MoveTo({ relevantRect.left, relevantRect.top });

			dc->SetBkMode(TRANSPARENT);
			dc->SetTextColor(RGB(200, 200, 200));

			dc->LineTo(relevantRect.right, relevantRect.top);
			dc->LineTo(relevantRect.right, relevantRect.bottom);
			dc->LineTo(relevantRect.left, relevantRect.bottom);
			dc->LineTo(relevantRect.left, relevantRect.top);

			dc->DrawTextEx( const_cast<char *>(cstr(count)), str(count).size(), &relevantRect, DT_CENTER |
							DT_SINGLELINE | DT_VCENTER, NULL );
			count++;
		}
		DeleteObject(crossPen);
	}
}


/*
 * re-arrange the controls associated with this picture. Neccessary e.g. when the window is resized. 
 */
void PictureControl::rearrange(std::string cameraMode, std::string triggerMode, int width, int height, fontMap fonts)
{
	if (active)
	{
		editMax.rearrange(cameraMode, triggerMode, width, height, fonts);
		editMin.rearrange(cameraMode, triggerMode, width, height, fonts);
		labelMax.rearrange(cameraMode, triggerMode, width, height, fonts);
		labelMin.rearrange(cameraMode, triggerMode, width, height, fonts);
		sliderMax.rearrange(cameraMode, triggerMode, width, height, fonts);
		sliderMin.rearrange(cameraMode, triggerMode, width, height, fonts);
		scaledBackgroundArea.bottom = long(unscaledBackgroundArea.bottom * height / 997.0);
		scaledBackgroundArea.top = long(unscaledBackgroundArea.top * height / 997.0);
		scaledBackgroundArea.left = long(unscaledBackgroundArea.left * width / 1920.0);
		scaledBackgroundArea.right = long(unscaledBackgroundArea.right * width / 1920.0);

		double widthPicScale;
		double heightPicScale;
		if (unofficialImageParameters.width > unofficialImageParameters.height)
		{
			widthPicScale = 1;
			heightPicScale = double(unofficialImageParameters.height) / unofficialImageParameters.width;
		}
		else
		{
			heightPicScale = 1;
			widthPicScale = double(unofficialImageParameters.width) / unofficialImageParameters.height;
		}
		long width = long((scaledBackgroundArea.right - scaledBackgroundArea.left)*widthPicScale);
		// why isn't this scaled???
		long height = scaledBackgroundArea.bottom - scaledBackgroundArea.top;
		POINT mid = { (scaledBackgroundArea.left + scaledBackgroundArea.right) / 2,
			(scaledBackgroundArea.top + scaledBackgroundArea.bottom) / 2 };
		pictureArea.left = mid.x - width / 2;
		pictureArea.right = mid.x + width / 2;
		pictureArea.top = mid.y - height / 2;
		pictureArea.bottom = mid.y + height / 2;

	}	
}
