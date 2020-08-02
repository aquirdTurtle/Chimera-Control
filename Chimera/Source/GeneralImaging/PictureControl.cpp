// created by Mark O. Brown
#include "stdafx.h"
#include "PictureControl.h"
#include <algorithm>
#include <numeric>
#include <boost/lexical_cast.hpp>

PictureControl::PictureControl ( bool histogramOption, Qt::TransformationMode mode) 
	: histOption( histogramOption ), QWidget (), transformationMode(mode){
	active = true;
	if ( histOption ){
		horData.resize ( 1 );
		horData[ 0 ] = pPlotDataVec ( new plotDataVec ( 100, { 0, -1, 0 } ) );
		vertData.resize ( 1 );
		vertData[ 0 ] = pPlotDataVec ( new plotDataVec ( 100, { 0, -1, 0 } ) );
		updatePlotData ( );
	}
	repaint ();
}

void PictureControl::updatePlotData ( ){
	if ( !histOption ){
		return;
	}
	horData[ 0 ]->resize ( mostRecentImage_m.getCols ( ) );
	vertData[ 0 ]->resize ( mostRecentImage_m.getRows ( ) );
	unsigned count = 0;

	std::vector<long> dataRow;
	for ( auto& data : *horData[ 0 ] ){
		data.x = count;
		// integrate the column
		double p = 0.0;
		for ( auto row : range ( mostRecentImage_m.getRows ( ) ) ){
			p += mostRecentImage_m ( row, count );
		}
		count++;
		dataRow.push_back ( p );
	}
	count = 0;
	auto avg = std::accumulate ( dataRow.begin ( ), dataRow.end ( ), 0.0 ) / dataRow.size ( );
	for ( auto& data : *horData[ 0 ] ){
		data.y = dataRow[ count++ ] - avg;
	}
	count = 0;
	std::vector<long> dataCol;
	for ( auto& data : *vertData[ 0 ] ){
		data.x = count;
		// integrate the row
		double p = 0.0;
		for ( auto col : range ( mostRecentImage_m.getCols ( ) ) ){
			p += mostRecentImage_m ( count, col );
		}
		count++;
		dataCol.push_back ( p );
	}
	count = 0;
	auto avgCol = std::accumulate ( dataCol.begin ( ), dataCol.end ( ), 0.0 ) / dataCol.size ( );
	for ( auto& data : *vertData[ 0 ] ){
		data.y = dataCol[ count++ ] - avgCol;
	}
}

/*
* initialize all controls associated with single picture.
*/
void PictureControl::initialize( POINT loc, int width, int height, IChimeraQtWindow* parent, int picScaleFactorIn){
	picScaleFactor = picScaleFactorIn;
	if ( width < 100 ){
		thrower ( "Pictures must be greater than 100 in width because this is the size of the max/min"
									 "controls." );
	}
	if ( height < 100 ){
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
	pictureObject = new ImageLabel (parent);
	pictureObject->setGeometry (loc.x, loc.y, width, height);
	parent->connect (pictureObject, &ImageLabel::mouseReleased, [this](QMouseEvent* event) {handleMouse (event); });
	setPictureArea (loc, maxWidth, maxHeight);

	std::vector<unsigned char> data (20000);
	for (auto& pt : data){
		pt = rand () % 255;
	}
	
	loc.x += unscaledBackgroundArea.right - unscaledBackgroundArea.left;
	sliderMin.initialize(loc, parent, 50, unscaledBackgroundArea.bottom - unscaledBackgroundArea.top, "MIN" );
	sliderMin.setValue ( 0 );
	parent->connect (sliderMin.slider, &QSlider::valueChanged, [this]() {redrawImage (); });
	loc.x += 25;
	sliderMax.initialize ( loc, parent, 50, unscaledBackgroundArea.bottom - unscaledBackgroundArea.top, "MAX" );
	sliderMax.setValue ( 300 );
	parent->connect (sliderMax.slider, &QSlider::valueChanged, [this]() {redrawImage (); });
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



bool PictureControl::isActive(){
	return active;
}


void PictureControl::setSliderPositions(unsigned min, unsigned max){
	sliderMin.setValue ( min );
	sliderMax.setValue ( max );
}

/*
 * Used during initialization & when used when transitioning between 1 and >1 pictures per repetition. 
 * Sets the unscaled background area and the scaled area.
 */
void PictureControl::setPictureArea( POINT loc, int width, int height ){
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
	if ( horGraph ){
		//horGraph->setControlLocation ( { scaledBackgroundArea.left, scaledBackgroundArea.bottom }, 
		//							   scaledBackgroundArea.right - scaledBackgroundArea.left, 65 );
	}
	if ( vertGraph ){
		//vertGraph->setControlLocation ( { scaledBackgroundArea.left - 65, scaledBackgroundArea.bottom },
		//							      65, scaledBackgroundArea.bottom - scaledBackgroundArea.top );
	}
	double widthPicScale;
	double heightPicScale;
	auto& uIP = unofficialImageParameters;
	double w_to_h_ratio = double (uIP.width ()) / uIP.height ();
	double sba_w = sBA.right - sBA.left;
	double sba_h = sBA.bottom - sBA.top;
	if (w_to_h_ratio > sba_w/sba_h){
		widthPicScale = 1;
		heightPicScale = (1.0/ w_to_h_ratio) * (sba_w / sba_h);
	}
	else{
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
	
	if (pictureObject){
		pictureObject->setGeometry (loc.x, loc.y, width, height);
		pictureObject->raise ();
	}
}


/* used when transitioning between single and multiple pictures. It sets it based on the background size, so make 
 * sure to change the background size before using this.
 * ********/
void PictureControl::setSliderControlLocs (POINT pos, int height){
	sliderMin.reposition ( pos, height);
	pos.x += 25;
	sliderMax.reposition ( pos, height );
}

/* used when transitioning between single and multiple pictures. It sets it based on the background size, so make
* sure to change the background size before using this.
* ********/
void PictureControl::setCursorValueLocations( CWnd* parent ){
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
	for (unsigned colInc = 0; colInc < grid.size(); colInc++){
		for (unsigned rowInc = 0; rowInc < grid[colInc].size(); rowInc++){
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
void PictureControl::updatePalette( QVector<QRgb> palette ){
	imagePalette = palette;
}


/*
 * called when the user changes either the min or max edit.
 */
void PictureControl::handleEditChange( int id ){
	if ( id == sliderMax.getEditId() ){
		sliderMax.handleEdit ( );
	}
	if ( id == sliderMin.getEditId() ){
		sliderMin.handleEdit ( );
	}
}


std::pair<unsigned, unsigned> PictureControl::getSliderLocations(){
	return { sliderMin.getValue (), sliderMax.getValue() };
}


/*
 * called when the user drags the scroll bar.
 */
void PictureControl::handleScroll(int id, unsigned nPos){
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
	if (w_to_h_ratio > sba_w / sba_h){
		widthPicScale = 1;
		heightPicScale = (1.0 / w_to_h_ratio) * (sba_w / sba_h);
	}
	else{
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
	for (unsigned colInc = 0; colInc < grid.size(); colInc++){
		grid[colInc].resize(newParameters.height());
		for (unsigned rowInc = 0; rowInc < grid[colInc].size(); rowInc++){
			// for all 4 pictures...
			grid[colInc][rowInc].left = int(pictureArea.left
											 + (double)(colInc+1) * (pictureArea.right - pictureArea.left) 
											 / (double)grid.size( ) + 2);
			grid[colInc][rowInc].right = int(pictureArea.left
				+ (double)(colInc + 2) * (pictureArea.right - pictureArea.left) / (double)grid.size() + 2);
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
void PictureControl::redrawImage(){
	if ( active && mostRecentImage_m.size ( ) != 0 ){
		drawBitmap (mostRecentImage_m, mostRecentAutoscaleInfo, mostRecentSpecialMinSetting,
			mostRecentSpecialMaxSetting, mostRecentAnalysisLocs, mostRecentGrids, mostRecentPicNum, true);
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
								  bool specialMin, bool specialMax, std::vector<coordinate> analysisLocs,
								  std::vector<atomGrid> grids, unsigned pictureNumber, bool includingAnalysisMarkers ){

	mostRecentImage_m = picData;
	mostRecentPicNum = pictureNumber;
	mostRecentAnalysisLocs = analysisLocs;
	mostRecentGrids = grids;

	auto minColor = sliderMin.getValue ( );
	auto maxColor = sliderMax.getValue ( );
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
	
	//if ( pixelsAreaWidth % 4 ){
	//	pixelsAreaWidth += ( 4 - pixelsAreaWidth % 4 );
	//}
	float yscale = ( 256.0f ) / (float) colorRange;
	WORD argbq[ PICTURE_PALETTE_SIZE ];
	for ( int paletteIndex = 0; paletteIndex < PICTURE_PALETTE_SIZE; paletteIndex++ ){
		argbq[ paletteIndex ] = (WORD) paletteIndex;
	}
	std::vector<uchar> dataArray2 ( dataWidth * dataHeight, 255 );
	int iTemp;
	double dTemp = 1;
	for (int heightInc = 0; heightInc < dataHeight; heightInc++){
		for (int widthInc = 0; widthInc < dataWidth; widthInc++){
			dTemp = ceil (yscale * double(picData (heightInc, widthInc) - minColor));
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
			dataArray2[widthInc + heightInc * dataWidth] = (unsigned char)iTemp;
		}
	}
	int sf = picScaleFactor;
	QImage img (sf * dataWidth, sf * dataHeight, QImage::Format_Indexed8);
	img.setColorTable (imagePalette);
	img.fill (0);
	auto ct = 0;
	for (auto rowInc : range(dataHeight)){
		std::vector<uchar> singleRow (sf * dataWidth);
		for (auto val : range (dataWidth)){
			for (auto rep : range (sf)) {
				singleRow[sf * val + rep] = dataArray2[rowInc * dataWidth + val];
			}
		}
		for (auto repRow : range (sf)){
			memcpy (img.scanLine (rowInc * sf + repRow), singleRow.data(), img.bytesPerLine ());
		}
	}
	// need to convert to an rgb format in order to draw on top. drawing on top using qpainter isn't supported with the 
	// indexed format. 
	img = img.convertToFormat (QImage::Format_RGB888);
	QPainter painter;
	painter.begin (&img);
	drawDongles (painter, analysisLocs, grids, pictureNumber, includingAnalysisMarkers);
	painter.end ();	
	
	if (img.width () / img.height () > pictureObject->width () / pictureObject->height ())	{
		pictureObject->setPixmap (QPixmap::fromImage (img).scaledToWidth (pictureObject->width (), transformationMode));
	}
	else {
		pictureObject->setPixmap (QPixmap::fromImage (img).scaledToHeight (pictureObject->height (), transformationMode));
	}
	// update this with the new picture.
	setHoverValue ( );
}

void PictureControl::setHoverValue( ){
	int loc = (grid.size( ) - 1 - selectedLocation.column) * grid.size( ) + selectedLocation.row;
	if ( loc >= mostRecentImage_m.size( ) )	{
		return;
	}
	valueDisp->setText( cstr( mostRecentImage_m.data[loc] ) );
}

void PictureControl::handleMouse (QMouseEvent* event){
	auto loc = event->windowPos ();
	unsigned rowCount = 0;
	unsigned colCount = 0;
	for ( auto col : grid ){
		for ( auto box : col ){
			if (loc.x() < box.right && loc.x() > box.left && loc.y() > box.top && loc.y() < box.bottom ) {
				coordinatesDisp->setText( (str( rowCount ) + ", " + str( colCount )).c_str( ) );
				selectedLocation = { rowCount, colCount };
				if ( mostRecentImage_m.size( ) != 0 && grid.size( ) != 0 ){
					setHoverValue( );
				}
			}
			rowCount += 1;
		}
		colCount += 1;
		rowCount = 0;
	}
	redrawImage ();
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
	for (unsigned columnInc = 0; columnInc < grid.size(); columnInc++)
	{
		for (unsigned rowInc = 0; rowInc < grid[columnInc].size(); rowInc++)
		{
			easel->FrameRect(&grid[columnInc][rowInc], brush);
		}
	}*/
}


/*
 * draws the circle which denotes the selected pixel that the user wants to know the counts for. 
 */
void PictureControl::drawCircle(coordinate selectedLocation, QPainter& painter)
{
	if (grid.size() == 0){
		// this hasn't been set yet, presumably this got called by the camera window as the camera window
		// was drawing itself before the control was initialized.
		return;
	}
	if (!active){
		// don't draw anything if the window isn't active.
		return;
	}
	QRect smallRect( selectedLocation.column * picScaleFactor, selectedLocation.row * picScaleFactor, 
					 picScaleFactor-1, picScaleFactor-1 );
	painter.drawEllipse (smallRect);
}

void PictureControl::drawPicNum( unsigned picNum, QPainter& painter ){
	QFont font = painter.font ();
	// I think this is the font height in pixels on the pixmap basically. 
	font.setPointSize (20);
	painter.setFont (font);
	painter.setPen (Qt::white);
	painter.drawText (QPoint ( int(picScaleFactor)/5, picScaleFactor), cstr (picNum));
}

void PictureControl::drawAnalysisMarkers( std::vector<coordinate> analysisLocs, std::vector<atomGrid> gridInfo,
										  QPainter& painter ){
	if ( !active ){
		return;
	}
	painter.setPen (Qt::white);
	//std::vector<COLORREF> colors = { RGB( 100, 100, 100 ), RGB( 0, 100, 0 ), RGB( 0, 0, 100), RGB( 100, 0, 0 ) };
	unsigned gridCount = 0;
	for ( auto atomGrid : gridInfo ){
		if ( atomGrid.topLeftCorner == coordinate( 0, 0 ) ){
			// atom grid is empty, not to be used.
			unsigned count = 1;
		}
		else {
			// use the atom grid.
			unsigned count = 1;
			for ( auto columnInc : range( atomGrid.width ) ){
				for ( auto rowInc : range( atomGrid.height ) ){
					unsigned pixelRow = picScaleFactor*(atomGrid.topLeftCorner.row + rowInc * atomGrid.pixelSpacing);
					unsigned pixelColumn = picScaleFactor * (atomGrid.topLeftCorner.column 
														 + columnInc * atomGrid.pixelSpacing);
					QRect rect = QRect (QPoint(pixelColumn, pixelRow), 
										QPoint (pixelColumn+picScaleFactor-2, pixelRow + picScaleFactor - 2));
					painter.drawRect (rect);					
					painter.drawText (rect, Qt::AlignCenter, cstr(count++));
				}
			}
		}
		gridCount++;
	}
}

void PictureControl::drawDongles (QPainter& painter, std::vector<coordinate> analysisLocs,
	std::vector<atomGrid> grids, unsigned pictureNumber, bool includingAnalysisMarkers){
	drawPicNum (pictureNumber, painter);
	if (includingAnalysisMarkers) {
		drawAnalysisMarkers (  analysisLocs, grids, painter );
	}
	painter.setPen (Qt::red);
	drawCircle (selectedLocation, painter);
}


