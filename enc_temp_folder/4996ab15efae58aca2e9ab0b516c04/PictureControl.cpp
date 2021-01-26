// created by Mark O. Brown
#include "stdafx.h"
#include <GeneralImaging/PictureManager.h>
#include "PictureControl.h"
#include <algorithm>
#include <numeric>
#include <boost/lexical_cast.hpp>
#include <qmenu.h>

PictureControl::PictureControl ( bool histogramOption, Qt::TransformationMode mode) 
	: histOption( histogramOption ), QWidget (), transformationMode(mode){
	active = true;
	if ( histOption ){
		horData.resize ( 1 );
		vertData.resize ( 1 );
		updatePlotData ( );
	}
	repaint ();
}

void PictureControl::updatePlotData ( ){
	if ( !histOption ){
		return;
	}
	horData[ 0 ].resize ( mostRecentImage_m.getCols ( ) );
	vertData[ 0 ].resize ( mostRecentImage_m.getRows ( ) );
	unsigned count = 0;

	std::vector<long> dataRow;
	for ( auto& data : horData[ 0 ] ){
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
	for ( auto& data : horData[ 0 ] ){
		data.y = dataRow[ count++ ] - avg;
	}
	count = 0;
	std::vector<long> dataCol;
	for ( auto& data : vertData[ 0 ] ){
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
	for ( auto& data : vertData[ 0 ] ){
		data.y = dataCol[ count++ ] - avgCol;
	}
}


void PictureControl::handleContextMenu (const QPoint& pos, PictureManager* managerParent) {
	QMenu menu;
	menu.setStyleSheet (chimeraStyleSheets::stdStyleSheet ());
	if (managerParent->autoScalePictures) {
		auto* noAutoScale = new QAction ("No Autoscale", pictureObject);
		pictureObject->connect (noAutoScale, &QAction::triggered, 
			[this, managerParent]() {managerParent->autoScalePictures = false; });
		menu.addAction (noAutoScale);
	}
	else {
		auto* setAutoScale = new QAction ("Set Autoscale", pictureObject);
		pictureObject->connect (setAutoScale, &QAction::triggered,
			[this, managerParent]() {managerParent->autoScalePictures = true; });
		menu.addAction (setAutoScale);
	}
	menu.exec (pictureObject->mapToGlobal (pos));
}

/*
 * initialize all controls associated with single picture.
 */
void PictureControl::initialize( QPoint loc, int width, int height, IChimeraQtWindow* parent, 
	PictureManager* managerParent, int picScaleFactorIn){
	picScaleFactor = picScaleFactorIn;
	if ( width < 100 ){
		thrower ( "Pictures must be greater than 100 in width because this is the size of the max/min"
									 "controls." );
	}
	if ( height < 100 ){
		thrower ( "Pictures must be greater than 100 in height because this is the minimum height "
									 "of the max/min controls." );
	}
	auto& px = loc.rx (), & py = loc.ry ();
	maxWidth = width;
	maxHeight = height;
	if ( histOption ){
		QPoint pt{ 300,0 };
		vertGraph = new PlotCtrl ( 1, plotStyle::VertHist, std::vector<int>(), "", true );
		vertGraph->init (pt, 65, 860, parent );
		px += 65;
	}
	if ( histOption ){
		horGraph = new PlotCtrl ( 1, plotStyle::HistPlot, std::vector<int> ( ), "", true );
		QPoint pt{ 365, LONG (860) };
		horGraph->init ( pt, 1565 - 50, 65, parent );
	}
	pictureObject = new ImageLabel (parent);
	pictureObject->setGeometry (px, py, width, height);
	pictureObject->setContextMenuPolicy (Qt::CustomContextMenu);
	parent->connect (pictureObject, &ImageLabel::mouseReleased, [this](QMouseEvent* event) {handleMouse (event); });
	setPictureArea (loc, maxWidth, maxHeight);
	parent->connect (pictureObject, &QLabel::customContextMenuRequested,
		[this, managerParent](const QPoint& pos) {handleContextMenu (pos, managerParent); });

	std::vector<unsigned char> data (20000);
	for (auto& pt : data){
		pt = rand () % 255;
	}
	
	px += unscaledBackgroundArea.right - unscaledBackgroundArea.left;
	sliderMin.initialize(loc, parent, 50, unscaledBackgroundArea.bottom - unscaledBackgroundArea.top, "MIN" );
	sliderMin.setValue ( 0 );
	parent->connect (sliderMin.slider, &QSlider::valueChanged, [this]() {redrawImage (); });
	px += 25;
	sliderMax.initialize ( loc, parent, 50, unscaledBackgroundArea.bottom - unscaledBackgroundArea.top, "MAX" );
	sliderMax.setValue ( 300 );
	parent->connect (sliderMax.slider, &QSlider::valueChanged, [this]() {redrawImage (); });
	// reset this.
	px -= unscaledBackgroundArea.right - unscaledBackgroundArea.left;
	
	py += height - 25;
	coordinatesText = new QLabel ("Coordinates:", parent);
	coordinatesText->setGeometry (px, py, 100, 20);
	coordinatesDisp = new QLabel ("", parent);
	coordinatesDisp->setGeometry (px+100, py, 100, 20);
	valueText = new QLabel ("Value", parent);
	valueText->setGeometry (px + 200, py, 100, 20);
	valueDisp = new QLabel ("", parent);
	valueDisp->setGeometry (px + 300, py, 100, 20);
	py += 25;
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
void PictureControl::setPictureArea( QPoint loc, int width, int height ){
	// this is important for the control to know where it should draw controls.
	auto& sBA = scaledBackgroundArea;
	auto& px = loc.rx (), & py = loc.ry ();
	unscaledBackgroundArea = { px, py, px + width, py + height };
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

	unsigned long picWidth = unsigned long( (sBA.right - sBA.left)*widthPicScale );
	unsigned long picHeight = (sBA.bottom - sBA.top)*heightPicScale;
	QPoint mid = { (sBA.left + sBA.right) / 2, (sBA.top + sBA.bottom) / 2 };
	pictureArea.left = mid.x() - picWidth / 2;
	pictureArea.right = mid.x() + picWidth / 2;
	pictureArea.top = mid.y() - picHeight / 2;
	pictureArea.bottom = mid.y() + picHeight / 2;
	
	if (pictureObject){
		pictureObject->setGeometry (px, py, width, height);
		pictureObject->raise ();
	}
}


/* used when transitioning between single and multiple pictures. It sets it based on the background size, so make 
 * sure to change the background size before using this.
 * ********/
void PictureControl::setSliderControlLocs (QPoint pos, int height){
	sliderMin.reposition ( pos, height);
	pos.rx() += 25;
	sliderMax.reposition ( pos, height );
}

/* used when transitioning between single and multiple pictures. It sets it based on the background size, so make
* sure to change the background size before using this.
* ********/

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
 * Recalculate the grid of pixels, which needs to be done e.g. when changing number of pictures or re-sizing the 
 * picture. Does not draw the grid.
 */
void PictureControl::recalculateGrid(imageParameters newParameters){
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
	QPoint mid = { (scaledBackgroundArea.left + scaledBackgroundArea.right) / 2,
				  (scaledBackgroundArea.top + scaledBackgroundArea.bottom) / 2 };
	pictureArea.left = mid.x() - width / 2;
	pictureArea.right = mid.x() + width / 2;
	pictureArea.top = mid.y() - height / 2;
	pictureArea.bottom = mid.y() + height / 2;
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
		drawBitmap (mostRecentImage_m, mostRecentAutoScale, mostRecentAutoMin, mostRecentAutoMax, 
			mostRecentSpecialMinSetting, mostRecentSpecialMaxSetting, mostRecentGrids, mostRecentPicNum, true);
	}
}

void PictureControl::resetStorage(){
	mostRecentImage_m = Matrix<long>(0,0);
}

void PictureControl::setSoftwareAccumulationOption ( softwareAccumulationOption opt ){
	saOption = opt;
	accumPicData = Matrix<double> ();
	accumNum = 0;
}

/* 
  Version of this from the Basler camera control Code. I will consolidate these shortly.
*/
void PictureControl::drawBitmap ( const Matrix<long>& picData, bool autoScale, int autoMin, int autoMax,
								  bool specialMin, bool specialMax, std::vector<atomGrid> grids, unsigned pictureNumber,
								  bool includingAnalysisMarkers ){
	mostRecentImage_m = picData;
	mostRecentPicNum = pictureNumber;
	mostRecentGrids = grids;

	Matrix<long> drawData;

	auto minColor = sliderMin.getValue ( );
	auto maxColor = sliderMax.getValue ( );
	mostRecentAutoScale = autoScale;
	mostRecentAutoMax = autoMax;
	mostRecentAutoMin = autoMin;
	int pixelsAreaWidth = pictureArea.right - pictureArea.left + 1;
	int pixelsAreaHeight = pictureArea.bottom - pictureArea.top + 1;
	int dataWidth = grid.size ( );
	// first element containst whether autoscaling or not.
	long colorRange;
	if ( autoScale ){
		// third element contains max, second contains min.
		colorRange = autoMax - autoMin;
		minColor = autoMin;
	}
	else{
		colorRange = sliderMax.getValue ( ) - sliderMin.getValue ( );
		minColor = sliderMin.getValue ( );
	}
	if (saOption.accumAll){
		if (accumPicData.size () == 0)	{
			accumPicData = Matrix<double> (picData.getRows (), picData.getCols ());
			accumNum = 0;
		}
		accumNum++;
		if (accumPicData.size () != picData.size ()){
			thrower ("Size mismatch between software accumulated picture and picture input!");
		}
		Matrix<long> accumPicLongData (picData.getRows (), picData.getCols());
		for (auto rowInc : range (accumPicLongData.getRows ())) {
			for (auto colInc : range (accumPicLongData.getCols ())) {
				accumPicData (rowInc, colInc) = ((accumNum - 1) * accumPicData (rowInc, colInc) + picData (rowInc, colInc)) / accumNum;
				accumPicLongData (rowInc, colInc) = long (accumPicData (rowInc, colInc));
			}
		}
		//std::vector<long> accumPicLongData (picData.size ());
		//for (auto pixelInc : range (accumPicData.size ())){
		//	// suppose 16th image. accumNum = 16. new data = 15/16 * old data + new data / 16.
		//	accumPicData[pixelInc] = ((accumNum - 1) * accumPicData[pixelInc] + picData.data[pixelInc]) / accumNum;
		//	accumPicLongData[pixelInc] = long (accumPicData[pixelInc]);
		//}
		drawData = accumPicLongData;
	}
	else if (saOption.accumNum == 1){
		drawData = picData;
	}
	else {
		drawData = picData;
	}
	// assumes non-zero size...
	if ( grid.size ( ) == 0 ){
		thrower  ( "Tried to draw bitmap without setting grid size!" );
	}
	int dataHeight = grid[ 0 ].size ( );
	int totalGridSize = dataWidth * dataHeight;
	if (drawData.size ( ) != totalGridSize ){
		thrower  ( "Picture data didn't match grid size!" );
	}
	
	float yscale = ( 256.0f ) / (float) colorRange;
	std::vector<uchar> dataArray2 ( dataWidth * dataHeight, 255 );
	int iTemp;
	double dTemp = 1;
	const int picPaletteSize = 256;
	for (int heightInc = 0; heightInc < dataHeight; heightInc++){
		for (int widthInc = 0; widthInc < dataWidth; widthInc++){
			dTemp = ceil (yscale * double(drawData (heightInc, widthInc) - minColor));
			if (dTemp <= 0)	{
				// raise value to zero which is the floor of values this parameter can take.
				iTemp = 1;
			}
			else if (dTemp >= picPaletteSize - 1)	{
				// round to maximum value.
				iTemp = picPaletteSize - 2;
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
	drawDongles (painter, grids, pictureNumber, includingAnalysisMarkers);
	painter.end ();	
	// seems like this doesn't *quite* work for some reason, hence the extra number here to adjust
	if (img.width () / img.height () > (pictureObject->width () / pictureObject->height ())-0.1)	{
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
			if (loc.x() < box.right && loc.x () > box.left && loc.y() > box.top && loc.y () < box.bottom ) {
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
void PictureControl::drawGrid(QPainter& painter){
	if (!active){
		return;
	}
	if (grid.size() != 0){
		// hard set to 5000. Could easily change this to be able to see finer grids. Tested before and 5000 seems 
		// reasonable.
		if (grid.size() * grid.front().size() > 5000){
			return;
		}
	}
	// draw rectangles indicating where the pixels are.
	for (unsigned columnInc = 0; columnInc < grid.size(); columnInc++){
		for (unsigned rowInc = 0; rowInc < grid[columnInc].size(); rowInc++){
			unsigned pixelRow = picScaleFactor * grid[columnInc][rowInc].top;
			unsigned pixelColumn = picScaleFactor * grid[columnInc][rowInc].left;
			QRect rect = QRect (QPoint (pixelColumn, pixelRow),
						 QPoint (pixelColumn + picScaleFactor - 2, pixelRow + picScaleFactor - 2));
			painter.drawRect (rect);
		}
	}
}

/*
 * draws the circle which denotes the selected pixel that the user wants to know the counts for. 
 */
void PictureControl::drawCircle(coordinate selectedLocation, QPainter& painter){
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

void PictureControl::drawAnalysisMarkers( std::vector<atomGrid> gridInfo, QPainter& painter ){
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

void PictureControl::drawDongles (QPainter& painter, std::vector<atomGrid> grids, unsigned pictureNumber, 
	bool includingAnalysisMarkers){
	drawPicNum (pictureNumber, painter);
	if (includingAnalysisMarkers) {
		drawAnalysisMarkers (  grids, painter );
	}
	painter.setPen (Qt::red);
	drawCircle (selectedLocation, painter);
}
 
 
void PictureControl::setTransformationMode (Qt::TransformationMode mode) {
	transformationMode = mode;
}

