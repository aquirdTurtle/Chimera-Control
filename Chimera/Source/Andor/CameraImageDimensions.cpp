// created by Mark O. Brown
#include "stdafx.h"
#include "CameraImageDimensions.h"
#include "PrimaryWindows/QtAndorWindow.h"
#include "PrimaryWindows/QtMainWindow.h"
#include "AndorCameraSettingsControl.h"
#include <PrimaryWindows/QtAndorWindow.h>
#include <boost/lexical_cast.hpp>

ImageDimsControl::ImageDimsControl (std::string whichCam) : camType (whichCam) {
	isReady = false;
}

void ImageDimsControl::initialize( QPoint& pos, IChimeraQtWindow* parent, int numRows, int width ) {
	auto& px = pos.rx (), & py = pos.ry ();
	auto wi = width * numRows / 6;
	leftText = new QLabel ("Left", parent);
	leftText->setGeometry (px, py, wi, 25);

	rightText = new QLabel ("R. (/512)", parent);
	rightText->setGeometry (px+ wi, py, wi, 25);

	horBinningText = new QLabel ("H. Bin", parent);
	horBinningText->setGeometry (px + 2 * wi, py, wi, 25);
	if (numRows == 1) {
		px += width/2;
	}
	else {
		py += 50;
	}
	bottomLabel = new QLabel ("Bottom", parent);
	bottomLabel->setGeometry (px, py, wi, 25);

	topLabel = new QLabel ("Top (/512)", parent);
	topLabel->setGeometry (px + wi, py, wi, 25);

	vertBinningText = new QLabel ("V. Bin", parent);
	vertBinningText->setGeometry (px + 2*wi, py, wi, 25);
	
	if (numRows == 1) {
		px -= width / 2;
	}
	else {
		py -= 50;
	}
	leftEdit = new CQLineEdit ("1", parent);
	leftEdit->setGeometry (px, py+=25, wi, 25);

	rightEdit = new CQLineEdit ("5", parent);
	rightEdit->setGeometry (px + wi, py, wi, 25);

	horBinningEdit = new CQLineEdit ("1", parent);
	horBinningEdit->setGeometry (px + 2* wi, py, wi, 25);
	if (numRows == 1) {
		px += width / 2;
	}
	else {
		py += 50;
	}
	bottomEdit = new CQLineEdit ("1", parent);
	bottomEdit->setGeometry (px, py, wi, 25);

	topEdit = new CQLineEdit ("5", parent);
	topEdit->setGeometry (px + wi, py, wi, 25);

	vertBinningEdit = new CQLineEdit ("1", parent);
	vertBinningEdit->setGeometry (px + 2* wi, py, wi, 25);
	if (numRows == 1){
		px -= width / 2;
	}
	py += 25;
}

void ImageDimsControl::saveParams (ConfigStream& saveFile, imageParameters params) {
	saveFile << "\nCAMERA_IMAGE_DIMENSIONS"
			 << "\n/*Left:*/ " << params.left
			 << "\n/*Right:*/ " << params.right
			 << "\n/*H-Bin:*/ " << params.horizontalBinning
			 << "\n/*Bottom:*/ " << params.bottom
			 << "\n/*Top:*/ " << params.top
			 << "\n/*V-Bin:*/ " << params.verticalBinning
			 << "\nEND_CAMERA_IMAGE_DIMENSIONS\n";
}

void ImageDimsControl::handleSave(ConfigStream& saveFile ){
	saveParams (saveFile, readImageParameters());
}

imageParameters ImageDimsControl::getImageDimSettingsFromConfig (ConfigStream& configFile){
	imageParameters params;
	configFile >> params.left;
	configFile >> params.right;
	configFile >> params.horizontalBinning;
	configFile >> params.bottom;
	configFile >> params.top;
	configFile >> params.verticalBinning;
	return params;
}

void ImageDimsControl::handleOpen(ConfigStream& openFile){
	ConfigSystem::checkDelimiterLine( openFile, "CAMERA_IMAGE_DIMENSIONS" );
	imageParameters params = getImageDimSettingsFromConfig ( openFile );
	setImageParametersFromInput( params );
}


imageParameters ImageDimsControl::readImageParameters(){
	// in case called before initialized
	imageParameters params;
	if (!leftEdit)	{
		return params;
	}
	// set all of the image parameters
	try	{
		params.left = boost::lexical_cast<int>( str(leftEdit->text ()) );
	}
	catch ( boost::bad_lexical_cast&) {
		isReady = false;
		throwNested ( "Left border argument not an integer!\r\n" );
	}
	try	{
		params.right = boost::lexical_cast<int>( str(rightEdit->text()) );
	}
	catch ( boost::bad_lexical_cast&) {
		isReady = false;
		throwNested ( "Right border argument not an integer!\r\n" );
	}
	//
	try	{
		params.bottom = boost::lexical_cast<int>( str(bottomEdit->text()) );
	}
	catch ( boost::bad_lexical_cast&) {
		isReady = false;
		throwNested ( "Top border argument not an integer!\r\n" );
	}
	//
	try	{
		params.top = boost::lexical_cast<int>( str(topEdit->text()) );
	}
	catch ( boost::bad_lexical_cast&) {
		isReady = false;
		throwNested ( "Bottom border argument not an integer!\r\n" );
	}
	try	{
		params.horizontalBinning = boost::lexical_cast<int>( str(horBinningEdit->text()) );
	}
	catch ( boost::bad_lexical_cast&) {
		isReady = false;
		throwNested ( "Horizontal binning argument not an integer!\r\n" );
	}
	try	{
		params.verticalBinning = boost::lexical_cast<int>( str(vertBinningEdit->text()) );
	}
	catch ( boost::bad_lexical_cast&) {
		isReady = false;
		throwNested ( "Vertical binning argument not an integer!\r\n" );
	}
	// Check Image parameters
	try	{
		params.checkConsistency ( camType );
	}
	catch ( ChimeraError& )
	{
		isReady = false;
		throw;
	}

	// made it through successfully.
	isReady = true;
	return params;
}


/*
 * I forget why I needed a second function for this.
 */
void ImageDimsControl::setImageParametersFromInput( imageParameters param ){
	// set all of the image parameters
	leftEdit->setText( qstr(param.left ) );
	rightEdit->setText (qstr (param.right ) );
	bottomEdit->setText (qstr (param.bottom ) );
	topEdit->setText (qstr (param.top ) );
	horBinningEdit->setText (qstr (param.horizontalBinning ) );
	vertBinningEdit->setText (qstr (param.verticalBinning ) );
	// Check Image parameters
	try{
		param.checkConsistency(camType);
	}
	catch ( ChimeraError ){
		isReady = false;
		throw;
	}
	// made it through successfully.
	isReady = true;
}


bool ImageDimsControl::checkReady(){
	if (isReady){
		return true;
	}
	else{
		return false;
	}
}

void ImageDimsControl::updateEnabledStatus (bool viewRunning) {
	leftEdit->setEnabled(!viewRunning);
	rightEdit->setEnabled (!viewRunning);
	horBinningEdit->setEnabled (!viewRunning);
	bottomEdit->setEnabled (!viewRunning);
	topEdit->setEnabled (!viewRunning);
	vertBinningEdit->setEnabled (!viewRunning);
}
