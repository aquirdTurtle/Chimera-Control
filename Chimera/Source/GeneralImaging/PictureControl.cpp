// created by Mark O. Brown
#include "stdafx.h"
#include <GeneralImaging/PictureManager.h>
#include "PictureControl.h"
#include <algorithm>
#include <numeric>
#include <boost/lexical_cast.hpp>
#include <qmenu.h>

PictureControl::PictureControl ( bool histogramOption, Qt::TransformationMode mode) 
	: histOption(histogramOption), QWidget(), transformationMode(mode) {
	active = true;
	histOption = true;
	if ( histOption ){
		updatePlotData ( );
	}
	repaint ();
}

void PictureControl::initialize(QPoint loc, int width, int height, IChimeraQtWindow* parent,
	PictureManager* managerParent, int picScaleFactorIn) {
	picScaleFactor = picScaleFactorIn;
	if (width < 100) {
		thrower("Pictures must be greater than 100 in width because this is the size of the max/min"
			"controls.");
	}
	if (height < 100) {
		thrower("Pictures must be greater than 100 in height because this is the minimum height "
			"of the max/min controls.");
	}
	auto& px = loc.rx(), & py = loc.ry();
	maxWidth = width;
	maxHeight = height;
	if (histOption) {
		vertGraph = new QCustomPlotCtrl(1, plotStyle::PicturePlot, std::vector<int>(), true);
		vertGraph->init(loc, graphSmallSize, 860, parent, "");
		loc.rx() += graphSmallSize;
		horGraph = new QCustomPlotCtrl(1, plotStyle::PicturePlot, std::vector<int>(), true);
		horGraph->init(loc, 1565 - 50, graphSmallSize, parent, "");
	}
	pictureObject = new ImageLabel(parent);
	//pictureObject->setGeometry (px, py, width, height);
	pictureObject->setContextMenuPolicy(Qt::CustomContextMenu);
	parent->connect(pictureObject, &ImageLabel::mouseReleased, 
		[this, parent](QMouseEvent* event) {
		try {
			handleMouse(event);
		}
		catch (ChimeraError & error) {
			parent->reportErr(error.qtrace());
		}
		});
	setPictureArea(loc, maxWidth, maxHeight);
	parent->connect(pictureObject, &QLabel::customContextMenuRequested,
		[this, managerParent](const QPoint& pos) {handleContextMenu(pos, managerParent); });

	std::vector<unsigned char> data(20000);
	for (auto& pt : data) {
		pt = rand() % 255;
	}
	px += unscaledBackgroundArea.right() - unscaledBackgroundArea.left();
	sliderMin.initialize(loc, parent, 50, unscaledBackgroundArea.bottom() - unscaledBackgroundArea.top(), "MIN");
	sliderMin.setValue(0);
	parent->connect(sliderMin.slider, &QSlider::valueChanged, [this]() {redrawImage(); });
	px += 25;
	sliderMax.initialize(loc, parent, 50, unscaledBackgroundArea.bottom() - unscaledBackgroundArea.top(), "MAX");
	sliderMax.setValue(300);
	parent->connect(sliderMax.slider, &QSlider::valueChanged, [this]() {redrawImage(); });
	// reset this.
	px -= unscaledBackgroundArea.right() - unscaledBackgroundArea.left();

	py += height - 25;
	coordinatesText = new QLabel("Coordinates:", parent);
	coordinatesText->setGeometry(px, py, 100, 20);
	coordinatesDisp = new QLabel("", parent);
	coordinatesDisp->setGeometry(px + 100, py, 100, 20);
	valueText = new QLabel("Value", parent);
	valueText->setGeometry(px + 200, py, 100, 20);
	valueDisp = new QLabel("", parent);
	valueDisp->setGeometry(px + 300, py, 100, 20);
	py += 25;
}


void PictureControl::updatePlotData() {
	if (!histOption || !vertGraph || !horGraph) {
		return;
	}
	auto useAccum = saOption.accumNum != 1 || saOption.accumAll;
	auto rowNum = useAccum ? accumPicData.getRows() : mostRecentImage_m.getRows();
	auto colNum = useAccum ? accumPicData.getCols() : mostRecentImage_m.getCols();
	std::vector<plotDataVec> horData(1), vertData(1);
	for ( auto colNum : range(colNum)){
		// integrate the column
		double pt = 0.0;
		for ( auto row : range (rowNum) ){
			pt += useAccum ? accumPicData(row, colNum) : mostRecentImage_m ( row, colNum );
		}
		horData[0].push_back({ double(colNum), pt });
		horData[0].push_back({ double(colNum+1), pt });
	}
	horGraph->setData(horData);
	for ( auto rowNum : range(rowNum)){
		// integrate the row
		double pt = 0.0;
		for ( auto col : range (colNum) ){
			pt += useAccum ? accumPicData(rowNum, col) : mostRecentImage_m(rowNum, col);
		}
		vertData[0].push_back({ pt, double(rowNum) });
		vertData[0].push_back({ pt, double(rowNum+1) });
	}
	vertGraph->setData(vertData);
}


void PictureControl::handleContextMenu (const QPoint& pos, PictureManager* managerParent) {
	QMenu menu;
	menu.setStyleSheet (chimeraStyleSheets::stdStyleSheet ());

	auto* autoScale = new QAction("Autoscale Pictures", pictureObject);
	autoScale->setCheckable(true);
	autoScale->setChecked(managerParent->autoScalePictures);
	pictureObject->connect(autoScale, &QAction::triggered, [this, managerParent]() {
		managerParent->autoScalePictures = !managerParent->autoScalePictures;
		});
	menu.addAction(autoScale);

	auto* showPlots = new QAction("Show Picture Integration Plots", pictureObject);
	showPlots->setCheckable(true);
	showPlots->setChecked(histOption);
	pictureObject->connect(showPlots, &QAction::triggered, [this]() {
		histOption = !histOption;
		updatePlotVisibility();
		});
	menu.addAction(showPlots);

	menu.exec (pictureObject->mapToGlobal (pos));
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
	unscaledBackgroundArea = { px, py, width, height };
	// reserve some area for the sliders?
	unscaledBackgroundArea.setRight(unscaledBackgroundArea.right()-100);
	sBA = unscaledBackgroundArea;

	if ( histOption && horGraph && vertGraph ){
		vertGraph->setControlLocation (QRect(QPoint({ px, py }), 
											 QSize(graphSmallSize, sBA.height() - graphSmallSize)));
		horGraph->setControlLocation (QRect(QPoint({ px+graphSmallSize, sBA.bottom() - graphSmallSize }),
											QSize(sBA.width(), graphSmallSize)));
	}
	double widthPicScale;
	double heightPicScale;
	auto& uIP = unofficialImageParameters;
	double w_to_h_ratio = double (uIP.width ()) / uIP.height ();
	if (w_to_h_ratio > sBA.width() / sBA.height()){
		widthPicScale = 1;
		heightPicScale = (1.0/ w_to_h_ratio) * (sBA.width() / sBA.height());
	}
	else{
		heightPicScale = 1;
		widthPicScale = w_to_h_ratio / (sBA.width() / sBA.height());
	}

	unsigned long picWidth = unsigned long( (sBA.right() - sBA.left())*widthPicScale );
	unsigned long picHeight = (sBA.bottom() - sBA.top())*heightPicScale;
	QPoint mid = { (sBA.left() + sBA.right()) / 2, (sBA.top() + sBA.bottom()) / 2 };
	pictureArea.setLeft(mid.x() - picWidth / 2);
	pictureArea.setRight(mid.x() + picWidth / 2);
	pictureArea.setTop(mid.y() - picHeight / 2);
	pictureArea.setBottom(mid.y() + picHeight / 2);
	
	if (pictureObject) {
		if (histOption) {
			pictureObject->setGeometry(px + graphSmallSize, py, width - graphSmallSize, height - graphSmallSize);
		}
		else {
			pictureObject->setGeometry(px, py, width, height);
		}
		pictureObject->raise ();
	}
}


/* used when transitioning between single and multiple pictures. It sets it based on the background size, so make 
 * sure to change the background size before using this.
 * ********/
void PictureControl::setSliderControlLocs (QPoint pos, int height){
	if (histOption){
		height -= graphSmallSize;
	}
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
	auto& uIP = unofficialImageParameters;
	double w_to_h_ratio = double (uIP.width ()) / uIP.height ();
	auto& sBA = scaledBackgroundArea;
	double sba_w = sBA.right()- sBA.left();
	double sba_h = sBA.bottom() - sBA.top();
	if (w_to_h_ratio > sba_w / sba_h){
		widthPicScale = 1;
		heightPicScale = (1.0 / w_to_h_ratio) * (sba_w / sba_h);
	}
	else{
		heightPicScale = 1;
		widthPicScale = w_to_h_ratio / (sba_w / sba_h);
	}

	long width = long((scaledBackgroundArea.right()- scaledBackgroundArea.left())*widthPicScale);
	long height = long((scaledBackgroundArea.bottom() - scaledBackgroundArea.top())*heightPicScale);
	QPoint mid = { (scaledBackgroundArea.left()+ scaledBackgroundArea.right()) / 2,
				  (scaledBackgroundArea.top()+ scaledBackgroundArea.bottom()) / 2 };
	pictureArea.setLeft( mid.x() - width / 2);
	pictureArea.setRight(mid.x() + width / 2);
	pictureArea.setTop(mid.y() - height / 2);
	pictureArea.setBottom( mid.y() + height / 2);
	//

	grid.resize(newParameters.width());
	for (unsigned colInc = 0; colInc < grid.size(); colInc++){
		grid[colInc].resize(newParameters.height());
		for (unsigned rowInc = 0; rowInc < grid[colInc].size(); rowInc++){
			// for all 4 pictures...
			grid[colInc][rowInc].setLeft(int(pictureArea.left()
											 + (double)(colInc+1) * (pictureArea.right()- pictureArea.left())
											 / (double)grid.size( ) + 2));
			grid[colInc][rowInc].setRight( int(pictureArea.left()
				+ (double)(colInc + 2) * (pictureArea.right()- pictureArea.left()) / (double)grid.size() + 2));
			grid[colInc][rowInc].setTop( int(pictureArea.top()
				+ (double)(rowInc)* (pictureArea.bottom() - pictureArea.top()) / (double)grid[colInc].size()));
			grid[colInc][rowInc].setBottom( int(pictureArea.top()
				+ (double)(rowInc + 1)* (pictureArea.bottom() - pictureArea.top()) / (double)grid[colInc].size()));
		}
	}
}

void PictureControl::updatePlotVisibility() {
	if (histOption && active) {
		horGraph->plot->show();
		vertGraph->plot->show();
	}
	else {
		horGraph->plot->hide();
		vertGraph->plot->hide();
	}
}

/* 
 * sets the state of the picture and changes visibility of controls depending on that state.
 */
void PictureControl::setActive( bool activeState ){
	if (!coordinatesText || !coordinatesDisp)	{
		return;
	}
	active = activeState;
	if (!active){
		sliderMax.show ( false );
		sliderMin.show ( false );
		coordinatesText->hide( );
		coordinatesDisp->hide( );
		valueText->hide( );
		valueDisp->hide(  );
	}
	else{
		sliderMax.show( true );
		sliderMin.show( true );
		coordinatesText->show();
		coordinatesDisp->show();
		valueText->show( );
		valueDisp->show();
	}
	updatePlotVisibility();
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

Matrix<double> PictureControl::getAccumPicData () {
	return accumPicData;
}

softwareAccumulationOption PictureControl::getSoftwareAccumulationOption () {
	return saOption;
}

void PictureControl::drawBitmap ( const Matrix<long>& picData, bool autoScale, int autoMin, int autoMax,
								  bool specialMin, bool specialMax, std::vector<atomGrid> analysisGrids, unsigned pictureNumber,
								  bool includingAnalysisMarkers ){
	mostRecentImage_m = picData; 
	mostRecentPicNum = pictureNumber; 
	mostRecentGrids = analysisGrids; 
	
	Matrix<long> drawData; 
	auto minColor = sliderMin.getValue ( );
	auto maxColor = sliderMax.getValue ( ); 
	mostRecentAutoScale = autoScale;
	mostRecentAutoMax = autoMax;
	mostRecentAutoMin = autoMin;
	int pixelsAreaWidth = pictureArea.right()- pictureArea.left()+ 1;
	int pixelsAreaHeight = pictureArea.bottom() - pictureArea.top()+ 1;
	
	//int dataWidth = grid.size ( );
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
	
	// Looks like I'm just not handling the partial accumulation here. 	
	// new: always calculate the complete accumulation image update. Potentially unnecessary, but I don't think the
	// computational load is that large. but basically want to be able to toggle between the accumulated im and the
	// real-time im.
	if (accumPicData.size() == 0) {
		accumPicData = Matrix<double>(picData.getRows(), picData.getCols());
		accumNum = 0;
	}
	accumNum++;
	if (accumPicData.size() != picData.size()) {
		accumPicData = Matrix<double>(picData.getRows(), picData.getCols());
		qDebug() << "Size mismatch between software accumulated picture and picture input!";
	}
	Matrix<long> accumPicLongData(picData.getRows(), picData.getCols());
	for (auto rowInc : range(accumPicLongData.getRows())) {
		for (auto colInc : range(accumPicLongData.getCols())) {
			accumPicData(rowInc, colInc) = ((accumNum - 1) * accumPicData(rowInc, colInc) + picData(rowInc, colInc)) / accumNum;
			accumPicLongData(rowInc, colInc) = long(accumPicData(rowInc, colInc));
		}
	}

	if (saOption.accumAll){
		drawData = accumPicLongData;
	}
	else {
		drawData = picData;
		// reset this? Not sure why...
		accumPicData = Matrix<double>(picData.getRows(), picData.getCols());
	}
	// assumes non-zero size...
	auto dataWidth = drawData.getCols();
	auto dataHeight = drawData.getRows();
	
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
	// need to convert to an rgb format in order to draw on top. drawing on top()using qpainter isn't supported with the 
	// indexed format. 
	img = img.convertToFormat (QImage::Format_RGB888);
	QPainter painter;
	painter.begin (&img);
	drawDongles (painter, analysisGrids, pictureNumber, includingAnalysisMarkers);
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
			if (loc.x() < box.right()&& loc.x () > box.left()&& loc.y() > box.top()&& loc.y () < box.bottom() ) {
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
			unsigned pixelRow = picScaleFactor * grid[columnInc][rowInc].top();
			unsigned pixelColumn = picScaleFactor * grid[columnInc][rowInc].left();
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

void PictureControl::setPicScaleFactor (int scaleFactorIn){
	picScaleFactor = scaleFactorIn;
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

