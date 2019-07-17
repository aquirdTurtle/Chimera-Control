// created by Mark O. Brown
#include "stdafx.h"
#include "PictureControl.h"
#include "Thrower.h"
#include <algorithm>
#include <numeric>
#include <boost/lexical_cast.hpp>

PictureControl::PictureControl ( bool histogramOption ) : histOption( histogramOption )
{
	active = true;
	if ( histOption )
	{
		horData.resize ( 1 );
		horData[ 0 ] = pPlotDataVec ( new plotDataVec ( 100, { 0, -1, 0 } ) );
		vertData.resize ( 1 );
		vertData[ 0 ] = pPlotDataVec ( new plotDataVec ( 100, { 0, -1, 0 } ) );
		updatePlotData ( );
	}
}


void PictureControl::paint ( CDC* cdc, CRect size, CBrush* bgdBrush )
{
	if ( !active )
	{
		return;
	}
	cdc->SetBkColor ( RGB ( 0, 0, 0 ) );
	long width = size.right - size.left, height = size.bottom - size.top;
	// each dc gets initialized with the rect for the corresponding plot. That way, each dc only overwrites the area 
	// for a single plot.
	horGraph->setCurrentDims ( width, height );
	horGraph->drawPlot ( cdc, bgdBrush, bgdBrush );
	vertGraph->setCurrentDims ( width, height );
	vertGraph->drawPlot ( cdc, bgdBrush, bgdBrush );
}



void PictureControl::updatePlotData ( )
{
	if ( !histOption )
	{
		return;
	}
	horData[ 0 ]->resize ( mostRecentImage_m.getCols ( ) );
	vertData[ 0 ]->resize ( mostRecentImage_m.getRows ( ) );
	UINT count = 0;

	std::vector<long> dataRow;
	for ( auto& data : *horData[ 0 ] )
	{
		data.x = count;
		// integrate the column
		double p = 0.0;
		for ( auto row : range ( mostRecentImage_m.getRows ( ) ) )
		{
			p += mostRecentImage_m ( row, count );
		}
		count++;
		dataRow.push_back ( p );
	}
	count = 0;
	auto avg = std::accumulate ( dataRow.begin ( ), dataRow.end ( ), 0.0 ) / dataRow.size ( );
	for ( auto& data : *horData[ 0 ] )
	{
		data.y = dataRow[ count++ ] - avg;
	}
	count = 0;
	std::vector<long> dataCol;
	for ( auto& data : *vertData[ 0 ] )
	{
		data.x = count;
		// integrate the row
		double p = 0.0;
		for ( auto col : range ( mostRecentImage_m.getCols ( ) ) )
		{
			p += mostRecentImage_m ( count, col );
		}
		count++;
		dataCol.push_back ( p );
	}
	count = 0;
	auto avgCol = std::accumulate ( dataCol.begin ( ), dataCol.end ( ), 0.0 ) / dataCol.size ( );
	for ( auto& data : *vertData[ 0 ] )
	{
		data.y = dataCol[ count++ ] - avgCol;
	}
}


/*
* initialize all controls associated with single picture.
*/
void PictureControl::initialize( POINT loc, CWnd* parent, int& id, int width, int height, std::array<UINT, 2> minMaxIds,
								 std::vector<Gdiplus::Pen*> graphPens, CFont* font,
								 std::vector<Gdiplus::SolidBrush*> graphBrushes )
{
	if ( width < 100 )
	{
		thrower ( "Pictures must be greater than 100 in width because this is the size of the max/min"
									 "controls." );
	}
	if ( height < 100 )
	{
		thrower ( "Pictures must be greater than 100 in height because this is the minimum height "
									 "of the max/min controls." );
	}
	maxWidth = width;
	maxHeight = height;
	if ( histOption )
	{
		vertGraph = new PlotCtrl ( vertData, plotStyle::VertHist, graphPens, font, graphBrushes, std::vector<int>(), "", true );
		vertGraph->init ( { 300, 0 }, 65, 860, parent );
		loc.x += 65;
	}
	setPictureArea ( loc, maxWidth, maxHeight );
	if ( histOption )
	{
		horGraph = new PlotCtrl ( horData, plotStyle::HistPlot, graphPens, font, graphBrushes, std::vector<int> ( ), "", true );
		horGraph->init ( { 365, LONG(860) }, 1565 - 50, 65, parent );
	}

	//setPictureArea( loc, width, height-25 );
	loc.x += unscaledBackgroundArea.right - unscaledBackgroundArea.left;
	sliderMin.initialize(loc, parent, id, 50, unscaledBackgroundArea.bottom - unscaledBackgroundArea.top, minMaxIds[0],
						  "MIN" );
	sliderMin.setValue ( 0 );
	loc.x += 50;
	sliderMax.initialize ( loc, parent, id, 50, unscaledBackgroundArea.bottom - unscaledBackgroundArea.top, minMaxIds[1],
						   "MAX" );
	sliderMax.setValue ( 300 );
	// reset this.
	loc.x -= unscaledBackgroundArea.right - unscaledBackgroundArea.left;
	
	loc.y += height - 25;
	coordinatesText.sPos = { loc.x, loc.y, loc.x += 100, loc.y + 20 };
	coordinatesText.Create( "Coordinates: ", WS_CHILD | WS_VISIBLE, coordinatesText.sPos, parent, id++ );
	coordinatesDisp.sPos = { loc.x, loc.y, loc.x += 100, loc.y + 20 };
	coordinatesDisp.Create( "", WS_CHILD | WS_VISIBLE | ES_READONLY, coordinatesDisp.sPos, parent, id++ );
	valueText.sPos = { loc.x, loc.y, loc.x += 100, loc.y + 20 };
	valueText.Create( "Value: ", WS_CHILD | WS_VISIBLE, valueText.sPos, parent, id++ );
	valueDisp.sPos = { loc.x, loc.y, loc.x += 100, loc.y + 20 };
	valueDisp.Create( "", WS_CHILD | WS_VISIBLE | ES_READONLY, valueDisp.sPos, parent, id++ );
	loc.y += 25;
}



bool PictureControl::isActive()
{
	return active;
}


void PictureControl::setSliderPositions(UINT min, UINT max)
{
	sliderMin.setValue ( min );
	sliderMax.setValue ( max );
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
	if ( horGraph )
	{
		//horGraph->setControlLocation ( { scaledBackgroundArea.left, scaledBackgroundArea.bottom }, 
		//							   scaledBackgroundArea.right - scaledBackgroundArea.left, 65 );
	}
	if ( vertGraph )
	{
		//vertGraph->setControlLocation ( { scaledBackgroundArea.left - 65, scaledBackgroundArea.bottom },
		//							      65, scaledBackgroundArea.bottom - scaledBackgroundArea.top );
	}
	double widthPicScale;
	double heightPicScale;
	if (unofficialImageParameters.width() > unofficialImageParameters.height())
	{
		widthPicScale = 1;
		heightPicScale = double(unofficialImageParameters.height()) / unofficialImageParameters.width();
	}
	else
	{
		heightPicScale = 1;
		widthPicScale = double(unofficialImageParameters.width()) / unofficialImageParameters.height();
	}
	ULONG picWidth = ULONG( (scaledBackgroundArea.right - scaledBackgroundArea.left)*widthPicScale );
	ULONG picHeight = scaledBackgroundArea.bottom - scaledBackgroundArea.top;
	POINT mid = { (scaledBackgroundArea.left + scaledBackgroundArea.right) / 2,
				  (scaledBackgroundArea.top + scaledBackgroundArea.bottom) / 2 };
	pictureArea.left = mid.x - picWidth / 2;
	pictureArea.right = mid.x + picWidth / 2;
	pictureArea.top = mid.y - picHeight / 2;
	pictureArea.bottom = mid.y + picHeight / 2;
	horGraph;
}


/* used when transitioning between single and multiple pictures. It sets it based on the background size, so make 
 * sure to change the background size before using this.
 * ********/
void PictureControl::setSliderControlLocs(CWnd* parent)
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
	sliderMin.reposition ( loc, collumnWidth, blockHeight, 
		( unscaledBackgroundArea.bottom - unscaledBackgroundArea.top ) * heightScale);
	loc.x += collumnWidth;
	sliderMax.reposition ( loc, collumnWidth, blockHeight,
		( unscaledBackgroundArea.bottom - unscaledBackgroundArea.top ) * heightScale );
}

/* used when transitioning between single and multiple pictures. It sets it based on the background size, so make
* sure to change the background size before using this.
* ********/
void PictureControl::setCursorValueLocations( CWnd* parent )
{
	CRect rect;
	parent->GetWindowRect( &rect );
	long width = rect.right - rect.left;
	long height = rect.bottom - rect.top;
	double widthScale = width / 1920.0;
	double heightScale = height / 997.0;
	widthScale = 1;
	heightScale = 1;
	POINT loc = { long( unscaledBackgroundArea.left * widthScale ), long( unscaledBackgroundArea.bottom * heightScale ) };
	coordinatesText.sPos = { loc.x, loc.y, loc.x += 100, loc.y + 25  };
	coordinatesDisp.sPos = { loc.x, loc.y, loc.x += 100, loc.y + 25 };
	valueText.sPos = { loc.x, loc.y, loc.x += 100, loc.y + 25 };
	valueDisp.sPos = { loc.x, loc.y, loc.x += 100, loc.y + 25 };
}

/*
 * Called in order to see if a right click is above a camera pixel. Returns coordinates of the camera pixel.
 */
coordinate PictureControl::checkClickLocation( CPoint clickLocation )
{
	CPoint test;
	for (UINT colInc = 0; colInc < grid.size(); colInc++)
	{
		for (UINT rowInc = 0; rowInc < grid[colInc].size(); rowInc++)
		{
			RECT relevantRect = grid[colInc][rowInc];
			// check if inside box
			if (clickLocation.x <= relevantRect.right && clickLocation.x >= relevantRect.left
				 && clickLocation.y <= relevantRect.bottom && clickLocation.y >= relevantRect.top)
			{
				// returns row x column
				coordinate location;
				location.row = rowInc+1;
				location.column = colInc+1;
				return location;
				// then click was inside a box so this should do something.
			}
		}
	}
	// null result. only first number is checked.
	thrower ( "click location not found" );
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
	if ( id == sliderMax.getEditId() )
	{
		sliderMax.handleEdit ( );
	}
	if ( id == sliderMin.getEditId() )
	{
		sliderMin.handleEdit ( );
	}
}


std::pair<UINT, UINT> PictureControl::getSliderLocations()
{
	
	return { sliderMin.getValue (), sliderMax.getValue() };
}


/*
 * called when the user drags the scroll bar.
 */
void PictureControl::handleScroll(int id, UINT nPos)
{
	if ( id == sliderMax.getSliderId ( ) )
	{
		sliderMax.handleSlider ( nPos );
	}
	if ( id == sliderMin.getSliderId ( ) )
	{
		sliderMin.handleSlider ( nPos );
	}
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
	if (unofficialImageParameters.width ()> unofficialImageParameters.height())
	{
		widthPicScale = 1;
		heightPicScale = double(unofficialImageParameters.height()) / unofficialImageParameters.width();
	}
	else
	{
		heightPicScale = 1;
		widthPicScale = double(unofficialImageParameters.width()) / unofficialImageParameters.height();
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

	grid.resize(newParameters.width());
	for (UINT colInc = 0; colInc < grid.size(); colInc++)
	{
		grid[colInc].resize(newParameters.height());
		for (UINT rowInc = 0; rowInc < grid[colInc].size(); rowInc++)
		{
			// for all 4 pictures...
			grid[colInc][rowInc].left = int(pictureArea.left
											 + (double)colInc * (pictureArea.right - pictureArea.left) 
											 / (double)grid.size( ) + 2);
			grid[colInc][rowInc].right = int(pictureArea.left
				+ (double)(colInc + 1) * (pictureArea.right - pictureArea.left) / (double)grid.size() + 2);
			grid[colInc][rowInc].top = int(pictureArea.top
				+ (double)(rowInc)* (pictureArea.bottom - pictureArea.top) / (double)grid[colInc].size());
			grid[colInc][rowInc].bottom = int(pictureArea.top
				+ (double)(rowInc + 1)* (pictureArea.bottom - pictureArea.top) / (double)grid[colInc].size());
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
		sliderMax.hide ( SW_HIDE );
		sliderMin.hide ( SW_HIDE );
		//
		coordinatesText.ShowWindow( SW_HIDE );
		coordinatesDisp.ShowWindow( SW_HIDE );
		valueText.ShowWindow( SW_HIDE );
		valueDisp.ShowWindow( SW_HIDE );
	}
	else
	{
		sliderMax.hide ( SW_SHOW );
		sliderMin.hide ( SW_SHOW );
		coordinatesText.ShowWindow( SW_SHOW );
		coordinatesDisp.ShowWindow( SW_SHOW );
		valueText.ShowWindow( SW_SHOW );
		valueDisp.ShowWindow( SW_SHOW );
	}
}

/*
 * redraws the background and image. 
 */
void PictureControl::redrawImage( CDC* easel, bool bkgd)
{
	if ( bkgd )
	{
		drawBackground ( easel );
	}
	if (active && mostRecentImage.size() != 0)
	{
		drawPicture(easel, mostRecentImage, mostRecentAutoscaleInfo, mostRecentSpecialMinSetting,
					mostRecentSpecialMaxSetting );
	}
	if ( active && mostRecentImage_m.size ( ) != 0 )
	{
		drawBitmap( easel, mostRecentImage_m, mostRecentAutoscaleInfo );
	}
}

void PictureControl::resetStorage()
{
	mostRecentImage = std::vector<long>{};
}


void PictureControl::setSoftwareAccumulationOption ( softwareAccumulationOption opt )
{
	saOption = opt;
	accumPicData.clear ( );
	accumNum = 0;
}


/* 
  Version of this from the Basler camera control Code. I will consolidate these shortly.
*/
void PictureControl::drawBitmap ( CDC* dc, const Matrix<long>& picData, std::tuple<bool, int, int> autoScaleInfo )
{
	mostRecentImage_m = picData;
	unsigned int minColor = sliderMin.getValue ( );
	unsigned int maxColor = sliderMax.getValue ( );
	mostRecentAutoscaleInfo = autoScaleInfo;
	dc->SelectPalette ( CPalette::FromHandle ( imagePalette ), true );
	dc->RealizePalette ( );
	int pixelsAreaWidth = pictureArea.right - pictureArea.left + 1;
	int pixelsAreaHeight = pictureArea.bottom - pictureArea.top + 1;
	int dataWidth = grid.size ( );
	// first element containst whether autoscaling or not.
	long colorRange;
	if ( std::get<0> ( autoScaleInfo ) )
	{
		// third element contains max, second contains min.
		colorRange = std::get<2> ( autoScaleInfo ) - std::get<1> ( autoScaleInfo );
		minColor = std::get<1> ( autoScaleInfo );
	}
	else
	{
		colorRange = sliderMax.getValue ( ); - sliderMin.getValue ( );
		minColor = sliderMin.getValue ( );
	}
	// assumes non-zero size...
	if ( grid.size ( ) == 0 )
	{
		thrower  ( "Tried to draw bitmap without setting grid size!" );
	}
	int dataHeight = grid[ 0 ].size ( );
	int totalGridSize = dataWidth * dataHeight;
	if ( picData.size ( ) != totalGridSize )
	{
		thrower  ( "Picture data didn't match grid size!" );
	}
	// imageBoxWidth must be a multiple of 4, otherwise StretchDIBits has problems apparently T.T
	if ( pixelsAreaWidth % 4 )
	{
		pixelsAreaWidth += ( 4 - pixelsAreaWidth % 4 );
	}
	float yscale = ( 256.0f ) / (float) colorRange;
	WORD argbq[ PICTURE_PALETTE_SIZE ];
	for ( int paletteIndex = 0; paletteIndex < PICTURE_PALETTE_SIZE; paletteIndex++ )
	{
		argbq[ paletteIndex ] = (WORD) paletteIndex;
	}
	PBITMAPINFO pbmi = (PBITMAPINFO) LocalAlloc ( LPTR, sizeof ( BITMAPINFOHEADER ) + sizeof ( RGBQUAD ) * ( 1 << 8 ) );
	pbmi->bmiHeader.biSize = sizeof ( BITMAPINFOHEADER );
	pbmi->bmiHeader.biPlanes = 1;
	pbmi->bmiHeader.biBitCount = 8;
	pbmi->bmiHeader.biCompression = BI_RGB;
	pbmi->bmiHeader.biClrUsed = PICTURE_PALETTE_SIZE;
	pbmi->bmiHeader.biSizeImage = 0;// ((pbmi->bmiHeader.biWidth * 8 + 31) & ~31) / 8 * pbmi->bmiHeader.biHeight;
	pbmi->bmiHeader.biHeight = dataHeight;
	memcpy ( pbmi->bmiColors, argbq, sizeof ( WORD ) * PICTURE_PALETTE_SIZE );
	std::vector<BYTE> dataArray ( dataWidth * dataHeight, 255 );
	int iTemp;
	double dTemp = 1;
	for ( int heightInc = 0; heightInc < dataHeight; heightInc++ )
	{
		for ( int widthInc = 0; widthInc < dataWidth; widthInc++ )
		{
			dTemp = ceil ( yscale * ( picData ( heightInc, widthInc ) - minColor ) );
			if ( dTemp <= 0 )
			{
				// raise value to zero which is the floor of values this parameter can take.
				iTemp = 1;
			}
			else if ( dTemp >= PICTURE_PALETTE_SIZE - 1 )
			{
				// round to maximum value.
				iTemp = PICTURE_PALETTE_SIZE - 2;
			}
			else
			{
				// no rounding or flooring to min or max needed.
				iTemp = (int) dTemp;
			}
			// store the value.
			dataArray[ widthInc + heightInc * dataWidth ] = (BYTE) iTemp;
		}
	}
	SetStretchBltMode ( dc->GetSafeHdc ( ), COLORONCOLOR );
	switch ( dataWidth )
	{
		case 0:
		{
			pbmi->bmiHeader.biWidth = dataWidth;
			pbmi->bmiHeader.biSizeImage = 1;
			StretchDIBits ( dc->GetSafeHdc ( ), pictureArea.left, pictureArea.top,
							pixelsAreaWidth, pixelsAreaHeight, 0, 0, dataWidth,
							dataHeight, dataArray.data ( ), ( BITMAPINFO FAR* )pbmi, DIB_PAL_COLORS, SRCCOPY );
			break;
		}
		case 2:
		{
			// make array that is twice as long.
			std::vector<BYTE> finalDataArray ( dataWidth * dataHeight * 2, 255 );
			for ( int dataInc = 0; dataInc < dataWidth * dataHeight; dataInc++ )
			{
				finalDataArray[ 2 * dataInc ] = dataArray[ dataInc ];
				finalDataArray[ 2 * dataInc + 1 ] = dataArray[ dataInc ];
			}
			pbmi->bmiHeader.biWidth = dataWidth * 2;
			StretchDIBits ( *dc, pictureArea.left, pictureArea.top, pixelsAreaWidth,
							pixelsAreaHeight, 0, 0, dataWidth * 2, dataHeight, finalDataArray.data ( ),
							( BITMAPINFO FAR* )pbmi, DIB_PAL_COLORS, SRCCOPY );
			break;
		}
		default:
		{
			// make array that is 4X as long.
			std::vector<BYTE> finalDataArray ( dataWidth * dataHeight * 4, 255 );
			for ( int dataInc = 0; dataInc < dataWidth * dataHeight; dataInc++ )
			{
				int data = dataArray[ dataInc ];
				finalDataArray[ 4 * dataInc ] = data;
				finalDataArray[ 4 * dataInc + 1 ] = data;
				finalDataArray[ 4 * dataInc + 2 ] = data;
				finalDataArray[ 4 * dataInc + 3 ] = data;
			}
			pbmi->bmiHeader.biWidth = dataWidth * 4;
			StretchDIBits ( *dc, pictureArea.left, pictureArea.top, pixelsAreaWidth,
							pixelsAreaHeight, 0, 0, dataWidth * 4, dataHeight, finalDataArray.data ( ),
							( BITMAPINFO FAR* )pbmi, DIB_PAL_COLORS, SRCCOPY );
			break;
		}
	}
	LocalFree ( pbmi );
	// update this with the new picture.
	setHoverValue ( );
}



/*
 * draw the picture that the camera took. The camera's data is inputted as a 1D vector of long here. The control needs
 * the camera window context since there's no direct control associated with the picture itself. Could probably change 
 * that.
 */
void PictureControl::drawPicture( CDC* deviceContext, std::vector<long> picData, 
								  std::tuple<bool, int/*min*/, int/*max*/> autoScaleInfo, bool specialMin, 
								  bool specialMax )
{
	/// initialize various structures
	float yscale;
	long colorRange;
	long minColor;
	std::vector<long> drawData;
	if ( saOption.accumAll )
	{
		if ( accumPicData.size ( ) == 0 )
		{
			accumPicData.resize ( picData.size ( ) );
			accumNum = 0;
		}
		accumNum++;
		if ( accumPicData.size ( ) != picData.size ( ) )
		{
			thrower ( "Size mismatch between software accumulated picture and picture input!" );
		}
		std::vector<long> accumPicLongData ( picData.size ( ) );
		for ( auto pixelInc : range(accumPicData.size()) )
		{
			// suppose 16th image. accumNum = 16. new data = 15/16 * old data + new data / 16.
			accumPicData[ pixelInc ] = ( ( accumNum - 1 )*accumPicData[ pixelInc ] + picData[ pixelInc ] ) / accumNum;
			accumPicLongData[ pixelInc ] = long ( accumPicData[ pixelInc ] );
		}
		drawData = accumPicLongData;
	}
	else
	{
		// TODO: should handle "normal" accumulation (i.e. not accumulate all but accumulate, say, 5) here
		drawData = picData;
	}
	mostRecentImage = drawData;
	// first element containst whether autoscaling or not.
	if (std::get<0>(autoScaleInfo))
	{
		// third element contains max, second contains min.
		colorRange = std::get<2>(autoScaleInfo) - std::get<1>(autoScaleInfo);
		minColor = std::get<1>(autoScaleInfo);
	}
	else
	{
		colorRange = sliderMax.getValue ( ) - sliderMin.getValue ( );
		minColor = sliderMin.getValue ( );
	}

	mostRecentAutoscaleInfo = autoScaleInfo;
	mostRecentSpecialMinSetting = specialMin;
	mostRecentSpecialMaxSetting = specialMax;
	int pixelsAreaWidth;
	int pixelsAreaHeight;
	int dataWidth, dataHeight;
	PBITMAPINFO bitmapInfoPtr;
	WORD argbq[PICTURE_PALETTE_SIZE];
	BYTE *DataArray;
	// this should probably be rewritten to use the deviceContext directly instead of this win32 style call.
	// Rotated
	SelectPalette( deviceContext->GetSafeHdc(), imagePalette, true );
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

	bitmapInfoPtr = (PBITMAPINFO)LocalAlloc( LPTR, sizeof( BITMAPINFOHEADER ) + sizeof( RGBQUAD ) * (1 << 8) );
	bitmapInfoPtr->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfoPtr->bmiHeader.biPlanes = 1;
	bitmapInfoPtr->bmiHeader.biBitCount = 8;
	bitmapInfoPtr->bmiHeader.biCompression = BI_RGB;
	bitmapInfoPtr->bmiHeader.biClrUsed = PICTURE_PALETTE_SIZE;
	bitmapInfoPtr->bmiHeader.biSizeImage = 0;
	bitmapInfoPtr->bmiHeader.biHeight = dataHeight;
	memcpy(bitmapInfoPtr->bmiColors, argbq, sizeof(WORD) * PICTURE_PALETTE_SIZE);
	DataArray = (BYTE*)malloc( (dataWidth * dataHeight) * sizeof( BYTE ) );
	memset( DataArray, 255, (dataWidth * dataHeight) * sizeof( BYTE ) );
	double tempDouble = 1;
	int tempInteger;
	/// convert image data to correspond to colors, i.e. convert to being between 0 and 255.
	for (int heightInc = 0; heightInc < dataHeight; heightInc++)
	{
		for (int widthInc = 0; widthInc < dataWidth; widthInc++)
		{
			// get temporary value for color of the pixel.
			if ( widthInc + heightInc * dataWidth >= drawData.size())
			{
				return;
			}
			tempDouble = ceil(yscale * (drawData[widthInc + heightInc * dataWidth] - minColor));

			// interpret the value depending on the range of values it can take.
			if (tempDouble < 1)
			{
				// raise value to zero which is the floor of values this parameter can take.
				if (specialMin)
				{
					// the absolute lowest color is a special color that doesn't match the rest of the pallete. 
					// Typically a bright blue.
					tempInteger = 0;
				}
				else
				{
					tempInteger = 1;
				}
			}
			else if (tempDouble > PICTURE_PALETTE_SIZE - 2)
			{
				// round to maximum value.
				if (specialMax)
				{
					// the absolute highest color is a special color that doesn't match the rest of the pallete.
					// typically a bright red.
					tempInteger = PICTURE_PALETTE_SIZE - 1;
				}
				else
				{
					tempInteger = PICTURE_PALETTE_SIZE - 2;
				}
			}
			else
			{
				// no rounding or flooring to min or max needed.
				tempInteger = (int)tempDouble;
			}
			// store the value.
			DataArray[widthInc + heightInc * dataWidth] = (BYTE)tempInteger;
		}
	}

	SetStretchBltMode( deviceContext->GetSafeHdc(), COLORONCOLOR );
	// I think that this should be possible to do witha  std::vector<BYTE> and getting the pointer to that vector using
	// .data() member.
	BYTE *finalDataArray = NULL;
	/// draw the final data.
	// handle the 0 (simple), 2 and 1/3 cases separately, scaling the latter three so that the data width is a multiple
	// of 4 pixels wide. There might be a faster way to do this. If you don't do this however, StretchDIBits fails in
	// very strange ways.
	switch (dataWidth)
	{
		case 0:
		{
			bitmapInfoPtr->bmiHeader.biWidth = dataWidth;
			bitmapInfoPtr->bmiHeader.biSizeImage = 1;
			StretchDIBits( deviceContext->GetSafeHdc(), pictureArea.left, pictureArea.top,
						   pixelsAreaWidth, pixelsAreaHeight, 0, 0, dataWidth,
						   dataHeight, DataArray, (BITMAPINFO FAR*)bitmapInfoPtr, DIB_PAL_COLORS, SRCCOPY );
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
			bitmapInfoPtr->bmiHeader.biWidth = dataWidth * 2;
			StretchDIBits( *deviceContext, pictureArea.left, pictureArea.top, pixelsAreaWidth, pixelsAreaHeight, 0, 0, 
						   dataWidth * 2, dataHeight, finalDataArray, (BITMAPINFO FAR*)bitmapInfoPtr, DIB_PAL_COLORS, 
						   SRCCOPY );
			free(finalDataArray);
			break;
		}
		default:
		{
			// scale by a factor of 4.
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
			bitmapInfoPtr->bmiHeader.biWidth = dataWidth * 4;
			StretchDIBits( *deviceContext, pictureArea.left, pictureArea.top, pixelsAreaWidth, pixelsAreaHeight, 0, 0, dataWidth * 4, dataHeight,
						   finalDataArray, (BITMAPINFO FAR*)bitmapInfoPtr, DIB_PAL_COLORS, SRCCOPY );
			free(finalDataArray);
			break;
		}
	}
	delete[] DataArray;
	LocalFree( bitmapInfoPtr );
	setHoverValue( );
}


void PictureControl::setHoverValue( )
{
	int loc = (grid.size( ) - 1 - mouseCoordinates.x) * grid.size( ) + mouseCoordinates.y;
	if ( loc >= mostRecentImage.size( ) )
	{
		return;
	}
	valueDisp.SetWindowTextA( cstr( mostRecentImage[loc] ) );
}


void PictureControl::handleMouse( CPoint p )
{
	int rowCount = 0;
	int colCount = 0;
	for ( auto col : grid )
	{
		for ( auto box : col )
		{
			if ( p.x < box.right && p.x > box.left && p.y > box.top && p.y < box.bottom )
			{
				coordinatesDisp.SetWindowTextA( (str( rowCount ) + ", " + str( colCount )).c_str( ) );
				mouseCoordinates = { rowCount, colCount };
				if ( mostRecentImage.size( ) != 0 && grid.size( ) != 0 )
				{
					setHoverValue( );
				}
			}
			rowCount += 1;
		}
		colCount += 1;
		rowCount = 0;
	}
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
	for (UINT columnInc = 0; columnInc < grid.size(); columnInc++)
	{
		for (UINT rowInc = 0; rowInc < grid[columnInc].size(); rowInc++)
		{
			easel->FrameRect(&grid[columnInc][rowInc], brush);
		}
	}
}


/*
 * draws the circle which denotes the selected pixel that the user wants to know the counts for. 
 */
void PictureControl::drawCircle(CDC* dc, coordinate selectedLocation)
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

	if ( selectedLocation.column > grid.size( ) || selectedLocation.row > grid[0].size() 
		 || selectedLocation.row <=0 || selectedLocation.column <= 0 )
	{
		// quietly don't try to draw.
		return;
	}
	RECT smallRect;
	RECT relevantRect = grid[selectedLocation.column-1][selectedLocation.row-1];
	smallRect.left = relevantRect.left + long(7.0 * (relevantRect.right - relevantRect.left) / 16.0);
	smallRect.right = relevantRect.left + long( 9.0 * (relevantRect.right - relevantRect.left) / 16.0);
	smallRect.top = relevantRect.top + long( 7.0 * (relevantRect.bottom - relevantRect.top) / 16.0);
	smallRect.bottom = relevantRect.top + long( 9.0 * (relevantRect.bottom - relevantRect.top) / 16.0);
	// get appropriate brush and pen
	if (dc == NULL)
	{
		thrower ("dc was null!");
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


void PictureControl::drawPicNum( CDC* dc, UINT picNum )
{
	HPEN textPen = CreatePen( 0, 1, RGB(100, 100, 120) );
	dc->SelectObject( textPen );
	RECT rect = grid[0][0];
	rect.right += 50;
			dc->DrawTextEx( const_cast<char *>(cstr( picNum )), str( picNum ).size( ), &grid[0][0],
					DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
	DeleteObject( textPen );
}


void PictureControl::drawAnalysisMarkers( CDC* dc, std::vector<coordinate> analysisLocs, 
										  std::vector<atomGrid> gridInfo )
{
	if ( !active )
	{
		return;
	}
	HPEN markerPen;
	std::vector<COLORREF> colors = { RGB( 100, 100, 100 ), RGB( 0, 100, 0 ), RGB( 0, 0, 100), RGB( 100, 0, 0 ) };
	UINT gridCount = 0;
	for ( auto atomGrid : gridInfo )
	{
		markerPen = CreatePen( 0, 1, colors[gridCount % 4] );
		dc->SelectObject( markerPen );

		if ( atomGrid.topLeftCorner == coordinate( 0, 0 ) )
		{
			// atom grid is empty, not to be used.
			UINT count = 1;
			DeleteObject( markerPen );
		}
		else
		{
			// use the atom grid.
			UINT count = 1;
			for ( auto columnInc : range( atomGrid.width ) )
			{
				for ( auto rowInc : range( atomGrid.height ) )
				{
					UINT pixelRow = atomGrid.topLeftCorner.row - 1 + rowInc * atomGrid.pixelSpacing;
					UINT pixelColumn = atomGrid.topLeftCorner.column - 1 + columnInc * atomGrid.pixelSpacing;
					if ( pixelColumn >= grid.size( ) || pixelRow >= grid[0].size( ) )
					{
						// just quietly don't try to draw. Could also have this throw, haven't decided exactly how I 
						// want to deal with this yet.
						continue;
					}
					RECT drawGrid, origGrid = grid[pixelColumn][pixelRow];
					drawGrid.left = origGrid.left + (origGrid.right - origGrid.left) * gridCount / 10;
					drawGrid.right = origGrid.right - (origGrid.right - origGrid.left) * gridCount / 10;
					drawGrid.top = origGrid.top + (origGrid.bottom - origGrid.top) * gridCount / 10;
					drawGrid.bottom = origGrid.bottom - (origGrid.bottom - origGrid.top) * gridCount / 10;

					drawRectangle( dc, drawGrid );
					dc->SetTextColor( colors[gridCount % 4] );
					dc->DrawTextEx( const_cast<char *>(cstr( count )), str( count ).size( ), 
									&drawGrid, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
					count++;
				}
			}
		}
		DeleteObject( markerPen );
		gridCount++;
	}
}


void PictureControl::drawRectangle( CDC* dc, RECT pixelRect )
{
	dc->MoveTo( { pixelRect.left, pixelRect.top } );

	dc->SetBkMode( TRANSPARENT );
	dc->SetTextColor( RGB( 200, 200, 200 ) );

	dc->LineTo( pixelRect.right, pixelRect.top );
	dc->LineTo( pixelRect.right, pixelRect.bottom );
	dc->LineTo( pixelRect.left, pixelRect.bottom );
	dc->LineTo( pixelRect.left, pixelRect.top );
}


/*
 * re-arrange the controls associated with this picture. Neccessary e.g. when the window is resized. 
 */
void PictureControl::rearrange( int width, int height, fontMap fonts)
{
	if (active)
	{
		sliderMax.rearrange(width, height, fonts);
		sliderMin.rearrange(width, height, fonts);
		coordinatesText.rearrange( width, height, fonts );
		coordinatesDisp.rearrange( width, height, fonts );
		valueText.rearrange( width, height, fonts );
		valueDisp.rearrange( width, height, fonts );
		scaledBackgroundArea.bottom = long(unscaledBackgroundArea.bottom * height / 997.0);
		scaledBackgroundArea.top = long(unscaledBackgroundArea.top * height / 997.0);
		scaledBackgroundArea.left = long(unscaledBackgroundArea.left * width / 1920.0);
		scaledBackgroundArea.right = long(unscaledBackgroundArea.right * width / 1920.0);
		if ( vertGraph )
		{
			vertGraph->rearrange ( width, height, fonts );
		}
		if ( horGraph )
		{
			horGraph->rearrange ( width, height, fonts );
		}
		double widthPicScale;
		double heightPicScale;
		if (unofficialImageParameters.width() > unofficialImageParameters.height())
		{
			widthPicScale = 1;
			heightPicScale = double(unofficialImageParameters.height()) / unofficialImageParameters.width();
		}
		else
		{
			heightPicScale = 1;
			widthPicScale = double(unofficialImageParameters.width()) / unofficialImageParameters.height();
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
