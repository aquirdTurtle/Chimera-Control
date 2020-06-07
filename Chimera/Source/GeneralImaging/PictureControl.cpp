// created by Mark O. Brown
#include "stdafx.h"
#include "PictureControl.h"
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


void PictureControl::paint (CRect size, CBrush* bgdBrush )
{
	return;
	if ( !active )
	{
		return;
	}
	long width = size.right - size.left, height = size.bottom - size.top;
	// each dc gets initialized with the rect for the corresponding plot. That way, each dc only overwrites the area 
	// for a single plot.
	horGraph->refreshData ();
	vertGraph->refreshData ();
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
void PictureControl::initialize( POINT loc, int width, int height, IChimeraWindowWidget* parent)
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
	if ( histOption ){
		POINT pt{ 300,0 };
		vertGraph = new PlotCtrl ( 1, plotStyle::VertHist, std::vector<int>(), "", true );
		vertGraph->init (pt, 65, 860, parent );
		loc.x += 65;
	}
	if ( histOption ){
		horGraph = new PlotCtrl ( 1, plotStyle::HistPlot, std::vector<int> ( ), "", true );
		POINT pt{ 365, LONG (860) };
		horGraph->init ( pt, 1565 - 50, 65, parent );
	}
	pictureObject = new QLabel (parent);
	pictureObject->setGeometry (loc.x, loc.y, width, height);
	setPictureArea (loc, maxWidth, maxHeight);

	std::vector<unsigned char> data (20000);
	for (auto& pt : data)
	{
		pt = rand () % 255;
	}
	
	loc.x += unscaledBackgroundArea.right - unscaledBackgroundArea.left;
	sliderMin.initialize(loc, parent, 50, unscaledBackgroundArea.bottom - unscaledBackgroundArea.top, "MIN" );
	sliderMin.setValue ( 0 );
	loc.x += 25;
	sliderMax.initialize ( loc, parent, 50, unscaledBackgroundArea.bottom - unscaledBackgroundArea.top, "MAX" );
	sliderMax.setValue ( 300 );
	// reset this.
	loc.x -= unscaledBackgroundArea.right - unscaledBackgroundArea.left;
	
	loc.y += height - 25;
	coordinatesText = new QLabel ("Coordinates:", parent);
	coordinatesText->setGeometry (loc.x, loc.y, 100, 20);
	coordinatesDisp = new QLabel ("", parent);
	coordinatesDisp->setGeometry (loc.x+100, loc.y, 100, 20);
	valueText = new QLabel ("Value", parent);
	valueText->setGeometry (loc.x + 200, loc.y, 100, 20);
	valueDisp = new QLabel ("", parent);
	valueDisp->setGeometry (loc.x + 300, loc.y, 100, 20);
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
	auto& sBA = scaledBackgroundArea;
	unscaledBackgroundArea = { loc.x, loc.y, loc.x + width, loc.y + height };
	// reserve some area for the texts.
	unscaledBackgroundArea.right -= 100;
	sBA = unscaledBackgroundArea;
	/*
	sBA.left *= width;
	sBA.right *= width;
	sBA.top *= height;
	sBA.bottom *= height;*/
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
	auto& uIP = unofficialImageParameters;
	double w_to_h_ratio = double (uIP.width ()) / uIP.height ();
	double sba_w = sBA.right - sBA.left;
	double sba_h = sBA.bottom - sBA.top;
	if (w_to_h_ratio > sba_w/sba_h)
	{
		widthPicScale = 1;
		heightPicScale = (1.0/ w_to_h_ratio) * (sba_w / sba_h);
	}
	else
	{
		heightPicScale = 1;
		widthPicScale = w_to_h_ratio / (sba_w / sba_h);
	}

	ULONG picWidth = ULONG( (sBA.right - sBA.left)*widthPicScale );
	ULONG picHeight = (sBA.bottom - sBA.top)*heightPicScale;
	POINT mid = { (sBA.left + sBA.right) / 2, (sBA.top + sBA.bottom) / 2 };
	pictureArea.left = mid.x - picWidth / 2;
	pictureArea.right = mid.x + picWidth / 2;
	pictureArea.top = mid.y - picHeight / 2;
	pictureArea.bottom = mid.y + picHeight / 2;
	
	if (pictureObject)
	{
		pictureObject->setGeometry (loc.x, loc.y, width, height);
		pictureObject->raise ();
	}
}


/* used when transitioning between single and multiple pictures. It sets it based on the background size, so make 
 * sure to change the background size before using this.
 * ********/
void PictureControl::setSliderControlLocs (POINT pos, int height)
{
	sliderMin.reposition ( pos, height);
	pos.x += 25;
	sliderMax.reposition ( pos, height );
}

/* used when transitioning between single and multiple pictures. It sets it based on the background size, so make
* sure to change the background size before using this.
* ********/
void PictureControl::setCursorValueLocations( CWnd* parent )
{
	if (!coordinatesText || !coordinatesDisp) {
		return;
	}
	CRect rect;
	parent->GetWindowRect( &rect );
	long width = rect.right - rect.left;
	long height = rect.bottom - rect.top;
	double widthScale = width / 1920.0;
	double heightScale = height / 997.0;
	widthScale = 1;
	heightScale = 1;
	POINT loc = { long( unscaledBackgroundArea.left * widthScale ), long( unscaledBackgroundArea.bottom * heightScale ) };
	coordinatesText->setGeometry (loc.x, loc.y, 100, 25);
	coordinatesDisp->setGeometry (loc.x+100, loc.y, 100, 25);
	valueText->setGeometry (loc.x+200, loc.y, 100, 25);
	valueDisp->setGeometry (loc.x+300, loc.y, 100, 25);
}

/*
 * Called in order to see if a right click is above a camera pixel. Returns coordinates of the camera pixel.
 */
coordinate PictureControl::checkClickLocation( CPoint clickLocation )
{
	CPoint test;
	for (UINT colInc = 0; colInc < grid.size(); colInc++){
		for (UINT rowInc = 0; rowInc < grid[colInc].size(); rowInc++){
			RECT relevantRect = grid[colInc][rowInc];
			// check if inside box
			if (clickLocation.x <= relevantRect.right && clickLocation.x >= relevantRect.left
				 && clickLocation.y <= relevantRect.bottom && clickLocation.y >= relevantRect.top){
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
void PictureControl::updatePalette( HPALETTE palette ){
	imagePalette = palette;
}


/*
 * called when the user changes either the min or max edit.
 */
void PictureControl::handleEditChange( int id ){
	if ( id == sliderMax.getEditId() )
	{
		sliderMax.handleEdit ( );
	}
	if ( id == sliderMin.getEditId() )
	{
		sliderMin.handleEdit ( );
	}
}


std::pair<UINT, UINT> PictureControl::getSliderLocations(){
	return { sliderMin.getValue (), sliderMax.getValue() };
}


/*
 * called when the user drags the scroll bar.
 */
void PictureControl::handleScroll(int id, UINT nPos){
	if ( id == sliderMax.getSliderId ( ) ){
		sliderMax.handleSlider ( nPos );
	}
	if ( id == sliderMin.getSliderId ( ) ){
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
	/*if (unofficialImageParameters.width ()> unofficialImageParameters.height())
	{
		widthPicScale = 1;
		heightPicScale = double(unofficialImageParameters.height()) / unofficialImageParameters.width();
	}
	else
	{
		heightPicScale = 1;
		widthPicScale = double(unofficialImageParameters.width()) / unofficialImageParameters.height();
	}*/
	auto& uIP = unofficialImageParameters;
	double w_to_h_ratio = double (uIP.width ()) / uIP.height ();
	auto& sBA = scaledBackgroundArea;
	double sba_w = sBA.right - sBA.left;
	double sba_h = sBA.bottom - sBA.top;
	if (w_to_h_ratio > sba_w / sba_h)
	{
		widthPicScale = 1;
		heightPicScale = (1.0 / w_to_h_ratio) * (sba_w / sba_h);
	}
	else
	{
		heightPicScale = 1;
		widthPicScale = w_to_h_ratio / (sba_w / sba_h);
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
	for (UINT colInc = 0; colInc < grid.size(); colInc++){
		grid[colInc].resize(newParameters.height());
		for (UINT rowInc = 0; rowInc < grid[colInc].size(); rowInc++){
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
	if (!coordinatesText || !coordinatesDisp)	{
		return;
	}
	active = activeState;
	if (!active){
		sliderMax.hide ( SW_HIDE );
		sliderMin.hide ( SW_HIDE );
		//
		coordinatesText->hide( );
		coordinatesDisp->hide( );
		valueText->hide( );
		valueDisp->hide(  );
	}
	else{
		sliderMax.hide ( SW_SHOW );
		sliderMin.hide ( SW_SHOW );
		coordinatesText->show();
		coordinatesDisp->show();
		valueText->show( );
		valueDisp->show();
	}
}

/*
 * redraws the background and image. 
 */
void PictureControl::redrawImage(bool bkgd){
	if ( bkgd ){
		drawBackground (  );
	}
	if ( active && mostRecentImage_m.size ( ) != 0 ){
		drawBitmap( mostRecentImage_m, mostRecentAutoscaleInfo, mostRecentSpecialMinSetting,
			mostRecentSpecialMaxSetting);
	}
}

void PictureControl::resetStorage(){
	mostRecentImage_m = Matrix<long>(0,0);
}


void PictureControl::setSoftwareAccumulationOption ( softwareAccumulationOption opt ){
	saOption = opt;
	accumPicData.clear ( );
	accumNum = 0;
}


/* 
  Version of this from the Basler camera control Code. I will consolidate these shortly.
*/
void PictureControl::drawBitmap ( const Matrix<long>& picData, std::tuple<bool, int, int> autoScaleInfo, 
	bool specialMin, bool specialMax )
{
	mostRecentImage_m = picData;
	unsigned int minColor = sliderMin.getValue ( );
	unsigned int maxColor = sliderMax.getValue ( );
	mostRecentAutoscaleInfo = autoScaleInfo;
	int pixelsAreaWidth = pictureArea.right - pictureArea.left + 1;
	int pixelsAreaHeight = pictureArea.bottom - pictureArea.top + 1;
	int dataWidth = grid.size ( );
	// first element containst whether autoscaling or not.
	long colorRange;
	if ( std::get<0> ( autoScaleInfo ) ){
		// third element contains max, second contains min.
		colorRange = std::get<2> ( autoScaleInfo ) - std::get<1> ( autoScaleInfo );
		minColor = std::get<1> ( autoScaleInfo );
	}
	else{
		colorRange = sliderMax.getValue ( ); - sliderMin.getValue ( );
		minColor = sliderMin.getValue ( );
	}
	// assumes non-zero size...
	if ( grid.size ( ) == 0 ){
		thrower  ( "Tried to draw bitmap without setting grid size!" );
	}
	int dataHeight = grid[ 0 ].size ( );
	int totalGridSize = dataWidth * dataHeight;
	if ( picData.size ( ) != totalGridSize ){
		thrower  ( "Picture data didn't match grid size!" );
	}
	
	pixmap = new QPixmap (dataWidth, dataHeight);

	// imageBoxWidth must be a multiple of 4, otherwise StretchDIBits has problems apparently T.T
	if ( pixelsAreaWidth % 4 ){
		pixelsAreaWidth += ( 4 - pixelsAreaWidth % 4 );
	}
	float yscale = ( 256.0f ) / (float) colorRange;
	WORD argbq[ PICTURE_PALETTE_SIZE ];
	for ( int paletteIndex = 0; paletteIndex < PICTURE_PALETTE_SIZE; paletteIndex++ ){
		argbq[ paletteIndex ] = (WORD) paletteIndex;
	}
	std::vector<uchar> dataArray2 ( dataWidth * dataHeight, 255 );
	std::vector<QRgb> rgbArray (dataWidth * dataHeight, 255);
	int iTemp;
	double dTemp = 1;
	for (int heightInc = 0; heightInc < dataHeight; heightInc++){
		for (int widthInc = 0; widthInc < dataWidth; widthInc++){
			dTemp = ceil (yscale * (picData (heightInc, widthInc) - minColor));
			if (dTemp <= 0)	{
				// raise value to zero which is the floor of values this parameter can take.
				iTemp = 1;
			}
			else if (dTemp >= PICTURE_PALETTE_SIZE - 1)	{
				// round to maximum value.
				iTemp = PICTURE_PALETTE_SIZE - 2;
			}
			else{
				// no rounding or flooring to min or max needed.
				iTemp = (int)dTemp;
			}
			// store the value.
			QColor color (iTemp, iTemp, iTemp);
			dataArray2[widthInc + heightInc * dataWidth] = (unsigned char)iTemp;
			rgbArray[widthInc + heightInc * dataWidth] = color.rgb();
		}
	}
	QImage* pixmap = new QImage (dataWidth, dataHeight, QImage::Format_Grayscale8 );
	pixmap->fill (0);
	auto ct = 0;
	for (int y = 0; y < pixmap->height (); y++)	{
		memcpy (pixmap->scanLine (y), dataArray2.data() + y * dataWidth, pixmap->bytesPerLine ());
	}
	if (pictureObject->width () > pictureObject->height ())	{
		pictureObject->setPixmap (QPixmap::fromImage (*pixmap).scaledToHeight (pictureObject->height ()));
	}
	else {
		pictureObject->setPixmap (QPixmap::fromImage (*pixmap).scaledToWidth (pictureObject->width()));
	}
	// update this with the new picture.
	setHoverValue ( );
}



void PictureControl::setHoverValue( )
{
	int loc = (grid.size( ) - 1 - mouseCoordinates.x) * grid.size( ) + mouseCoordinates.y;
	if ( loc >= mostRecentImage_m.size( ) )	{
		return;
	}
	valueDisp->setText( cstr( mostRecentImage_m.data[loc] ) );
}


void PictureControl::handleMouse( CPoint p )
{
	int rowCount = 0;
	int colCount = 0;
	for ( auto col : grid ){
		for ( auto box : col ){
			if ( p.x < box.right && p.x > box.left && p.y > box.top && p.y < box.bottom )
			{
				coordinatesDisp->setText( (str( rowCount ) + ", " + str( colCount )).c_str( ) );
				mouseCoordinates = { rowCount, colCount };
				if ( mostRecentImage_m.size( ) != 0 && grid.size( ) != 0 ){
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
void PictureControl::drawBackground (){
	return;
}


/* 
 * draw the grid which outlines where each pixel is.  Especially needs to be done when selecting pixels and no picture
 * is displayed. 
 */
void PictureControl::drawGrid(CBrush* brush)
{
	/*
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
	}*/
}


/*
 * draws the circle which denotes the selected pixel that the user wants to know the counts for. 
 */
void PictureControl::drawCircle(coordinate selectedLocation)
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
	/*
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
	dc->Ellipse( smallRect.left, smallRect.top, smallRect.right, smallRect.bottom );*/
}


void PictureControl::drawPicNum( UINT picNum )
{
	//HPEN textPen = CreatePen( 0, 1, RGB(100, 100, 120) );
	//dc->SelectObject( textPen );
	RECT rect = grid[0][0];
	rect.right += 50;
	//dc->DrawTextEx( const_cast<char *>(cstr( picNum )), str( picNum ).size( ), &grid[0][0],
	//		DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
	//DeleteObject( textPen );
}


void PictureControl::drawAnalysisMarkers(std::vector<coordinate> analysisLocs, std::vector<atomGrid> gridInfo )
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
		//dc->SelectObject( markerPen );

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

					drawRectangle( drawGrid );
					//dc->SetTextColor( colors[gridCount % 4] );
					//dc->DrawTextEx( const_cast<char *>(cstr( count )), str( count ).size( ), 
					//				&drawGrid, DT_CENTER | DT_SINGLELINE | DT_VCENTER, NULL );
					count++;
				}
			}
		}
		DeleteObject( markerPen );
		gridCount++;
	}
}


void PictureControl::drawRectangle(RECT pixelRect )
{
	/*
	dc->MoveTo( { pixelRect.left, pixelRect.top } );

	dc->SetBkMode( TRANSPARENT );
	dc->SetTextColor( RGB( 200, 200, 200 ) );

	dc->LineTo( pixelRect.right, pixelRect.top );
	dc->LineTo( pixelRect.right, pixelRect.bottom );
	dc->LineTo( pixelRect.left, pixelRect.bottom );
	dc->LineTo( pixelRect.left, pixelRect.top );*/
}

