// created by Mark O. Brown
#include "stdafx.h"
#include "CameraImageDimensions.h"
#include "PrimaryWindows/AndorWindow.h"
#include "PrimaryWindows/MainWindow.h"
#include "CameraSettingsControl.h"
#include <PrimaryWindows/QtAndorWindow.h>
#include <boost/lexical_cast.hpp>

ImageDimsControl::ImageDimsControl (std::string whichCam) : camType (whichCam) {
	isReady = false;
}


void ImageDimsControl::initialize( POINT& pos, IChimeraWindowWidget* parent, int numRows, int width ) {
	auto wi = width * numRows / 6;
	leftText = new QLabel ("Left", parent);
	leftText->setGeometry (pos.x, pos.y, wi, 25);

	rightText = new QLabel ("Right (/512)", parent);
	rightText->setGeometry (pos.x+ wi, pos.y, wi, 25);

	horBinningText = new QLabel ("H. Bin", parent);
	horBinningText->setGeometry (pos.x + 2 * wi, pos.y, wi, 25);
	if (numRows == 1) {
		pos.x += width/2;
	}
	else {
		pos.y += 50;
	}
	bottomLabel = new QLabel ("Bottom (/512)", parent);
	bottomLabel->setGeometry (pos.x, pos.y, wi, 25);

	topLabel = new QLabel ("Top", parent);
	topLabel->setGeometry (pos.x + wi, pos.y, wi, 25);

	vertBinningText = new QLabel ("V. Bin", parent);
	vertBinningText->setGeometry (pos.x + 2*wi, pos.y, wi, 25);
	
	if (numRows == 1) {
		pos.x -= width / 2;
	}
	else {
		pos.y -= 50;
	}
	leftEdit = new CQLineEdit ("1", parent);
	leftEdit->setGeometry (pos.x, pos.y+=25, wi, 25);

	rightEdit = new CQLineEdit ("5", parent);
	rightEdit->setGeometry (pos.x + wi, pos.y, wi, 25);

	horBinningEdit = new CQLineEdit ("1", parent);
	horBinningEdit->setGeometry (pos.x + 2* wi, pos.y, wi, 25);
	if (numRows == 1) {
		pos.x += width / 2;
	}
	else {
		pos.y += 50;
	}
	bottomEdit = new CQLineEdit ("1", parent);
	bottomEdit->setGeometry (pos.x, pos.y, wi, 25);

	topEdit = new CQLineEdit ("5", parent);
	topEdit->setGeometry (pos.x + wi, pos.y, wi, 25);

	vertBinningEdit = new CQLineEdit ("1", parent);
	vertBinningEdit->setGeometry (pos.x + 2* wi, pos.y, wi, 25);
	if (numRows == 1)
	{
		pos.x -= width / 2;
	}
	pos.y += 25;
}


void ImageDimsControl::handleSave(ConfigStream& saveFile )
{
	saveFile << "\nCAMERA_IMAGE_DIMENSIONS"
			 << "\n/*Left:*/ " << currentImageParameters.left
			 << "\n/*Right:*/ " << currentImageParameters.right
			 << "\n/*H-Bin:*/ " << currentImageParameters.horizontalBinning
			 << "\n/*Bottom:*/ " << currentImageParameters.bottom
			 << "\n/*Top:*/ " << currentImageParameters.top
			 << "\n/*V-Bin:*/ " << currentImageParameters.verticalBinning
			 << "\nEND_CAMERA_IMAGE_DIMENSIONS\n";
}


imageParameters ImageDimsControl::getImageDimSettingsFromConfig (ConfigStream& configFile)
{
	imageParameters params;
	configFile >> params.left;
	configFile >> params.right;
	configFile >> params.horizontalBinning;
	configFile >> params.bottom;
	configFile >> params.top;
	configFile >> params.verticalBinning;
	return params;
}

void ImageDimsControl::handleOpen(ConfigStream& openFile)
{
	ProfileSystem::checkDelimiterLine( openFile, "CAMERA_IMAGE_DIMENSIONS" );
	imageParameters params = getImageDimSettingsFromConfig ( openFile );
	setImageParametersFromInput( params );
}


imageParameters ImageDimsControl::readImageParameters()
{
	// in case called before initialized
	if (!leftEdit)	{
		return currentImageParameters;
	}
	// set all of the image parameters
	try	{
		currentImageParameters.left = boost::lexical_cast<int>( str(leftEdit->text ()) );
	}
	catch ( boost::bad_lexical_cast&) {
		isReady = false;
		throwNested ( "Left border argument not an integer!\r\n" );
	}
	try	{
		currentImageParameters.right = boost::lexical_cast<int>( str(rightEdit->text()) );
	}
	catch ( boost::bad_lexical_cast&) {
		isReady = false;
		throwNested ( "Right border argument not an integer!\r\n" );
	}
	//
	try	{
		currentImageParameters.bottom = boost::lexical_cast<int>( str(bottomEdit->text()) );
	}
	catch ( boost::bad_lexical_cast&) {
		isReady = false;
		throwNested ( "Top border argument not an integer!\r\n" );
	}
	//
	try	{
		currentImageParameters.top = boost::lexical_cast<int>( str(topEdit->text()) );
	}
	catch ( boost::bad_lexical_cast&) {
		isReady = false;
		throwNested ( "Bottom border argument not an integer!\r\n" );
	}
	try	{
		currentImageParameters.horizontalBinning = boost::lexical_cast<int>( str(horBinningEdit->text()) );
	}
	catch ( boost::bad_lexical_cast&) {
		isReady = false;
		throwNested ( "Horizontal binning argument not an integer!\r\n" );
	}
	try	{
		currentImageParameters.verticalBinning = boost::lexical_cast<int>( str(vertBinningEdit->text()) );
	}
	catch ( boost::bad_lexical_cast&) {
		isReady = false;
		throwNested ( "Vertical binning argument not an integer!\r\n" );
	}
	// Check Image parameters
	try	{
		currentImageParameters.checkConsistency ( camType );
	}
	catch ( Error& )
	{
		isReady = false;
		throw;
	}

	// made it through successfully.
	isReady = true;
	return currentImageParameters;
}


/*
 * I forget why I needed a second function for this.
 */
void ImageDimsControl::setImageParametersFromInput( imageParameters param )
{
	// set all of the image parameters
	currentImageParameters.left = param.left;
	leftEdit->setText( cstr( currentImageParameters.left ) );
	currentImageParameters.right = param.right;
	rightEdit->setText ( cstr( currentImageParameters.right ) );
	currentImageParameters.bottom = param.bottom;
	bottomEdit->setText ( cstr( currentImageParameters.bottom ) );
	currentImageParameters.top = param.top;
	topEdit->setText ( cstr( currentImageParameters.top ) );
	currentImageParameters.horizontalBinning = param.horizontalBinning;
	horBinningEdit->setText ( cstr( currentImageParameters.horizontalBinning ) );
	currentImageParameters.verticalBinning = param.verticalBinning;
	vertBinningEdit->setText ( cstr( currentImageParameters.verticalBinning ) );
	// Check Image parameters
	try
	{
		currentImageParameters.checkConsistency(camType);
	}
	catch ( Error )
	{
		isReady = false;
		throw;
	}
	// made it through successfully.
	isReady = true;
}


bool ImageDimsControl::checkReady()
{
	if (isReady)
	{
		return true;
	}
	else
	{
		return false;
	}
}


imageParameters ImageDimsControl::getImageParameters()
{
	return currentImageParameters;
}


HBRUSH ImageDimsControl::colorEdits( HWND window, UINT message, WPARAM wParam, LPARAM lParam, MainWindow* mainWin )
{
	DWORD controlID = GetDlgCtrlID( (HWND)lParam );
	HDC hdcStatic = (HDC)wParam;
	imageParameters currentImageParameters = { 0,0,0,0,0,0 };
	/*
	if (controlID == topEdit.GetDlgCtrlID())
	{
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		CString textEdit;
		topEdit.GetWindowTextA( textEdit );
		int bottom;
		try
		{
			bottom = boost::lexical_cast<int>( str( textEdit ) );
			if (bottom == currentImageParameters.top)
			{
				// good.
				SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
				SetBkColor( hdcStatic, RGB( 100, 110, 100 ) );
				// catch change of color and redraw window.
				if (topEdit.colorState != 0)
				{
					topEdit.colorState = 0;
					topEdit.RedrawWindow();
				}
				return *_myBrushes["Grey Green"];
			}
		}
		catch ( boost::bad_lexical_cast&)
		{
			// don't do anything with it.
		}
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		SetBkColor( hdcStatic, RGB( 150, 100, 100 ) );
		// catch change of color and redraw window.
		if (topEdit.colorState != 1)
		{
			topEdit.colorState = 1;
			topEdit.RedrawWindow();
		}
		return *_myBrushes["Grey Red"];
	}
	else if (controlID == bottomEdit.GetDlgCtrlID())
	{
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		CString textEdit;
		bottomEdit.GetWindowTextA( textEdit );
		int top;
		try
		{
			top = boost::lexical_cast<int>( str( textEdit ) );
			if (top == currentImageParameters.bottom)
			{
				// good.
				SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
				SetBkColor( hdcStatic, RGB( 100, 110, 100 ) );
				// catch change of color and redraw window.
				if (bottomEdit.colorState != 0)
				{
					bottomEdit.colorState = 0;
					bottomEdit.RedrawWindow();
				}
				return *_myBrushes["Grey Green"];
			}
		}
		catch ( boost::bad_lexical_cast&)
		{
			// don't do anything with it.
		}
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		SetBkColor( hdcStatic, RGB( 150, 100, 100 ) );
		// catch change of color and redraw window.
		if (bottomEdit.colorState != 1)
		{
			bottomEdit.colorState = 1;
			bottomEdit.RedrawWindow();
		}
		return *_myBrushes["Grey Red"];
	}
	else if (controlID == vertBinningEdit.GetDlgCtrlID())
	{
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		CString textEdit;
		vertBinningEdit.GetWindowTextA( textEdit );
		int verticalBin;
		try
		{
			verticalBin = boost::lexical_cast<int>( str( textEdit ) );
			if (verticalBin == currentImageParameters.verticalBinning)
			{
				// good.
				SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
				SetBkColor( hdcStatic, RGB( 100, 110, 100 ) );
				// catch change of color and redraw window.
				if (vertBinningEdit.colorState != 0)
				{
					vertBinningEdit.colorState = 0;
					vertBinningEdit.RedrawWindow();
				}
				return *_myBrushes["Grey Green"];
			}
		}
		catch ( boost::bad_lexical_cast&)
		{
			// don't do anything with it.
		}
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		SetBkColor( hdcStatic, RGB( 150, 100, 100 ) );
		// catch change of color and redraw window.
		if (vertBinningEdit.colorState != 1)
		{
			vertBinningEdit.colorState = 1;
			vertBinningEdit.RedrawWindow();
		}
		return *_myBrushes["Grey Red"];
	}
	else if (controlID == leftEdit.GetDlgCtrlID())
	{
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		CString textEdit;
		leftEdit.GetWindowTextA( textEdit );
		int leftSide;
		try
		{
			leftSide = boost::lexical_cast<int>( str( textEdit ) );
			if (leftSide == currentImageParameters.left)
			{
				// good.
				SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
				SetBkColor( hdcStatic, RGB( 100, 110, 100 ) );
				// catch change of color and redraw window.
				if (leftEdit.colorState != 0)
				{
					leftEdit.colorState = 0;
					leftEdit.RedrawWindow();
				}
				return *_myBrushes["Grey Green"];
			}
		}
		catch ( boost::bad_lexical_cast&)
		{
			// don't do anything with it.
		}
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		SetBkColor( hdcStatic, RGB( 150, 100, 100 ) );
		// catch change of color and redraw window.
		if (leftEdit.colorState != 1)
		{
			leftEdit.colorState = 1;
			leftEdit.RedrawWindow();
		}
		return *_myBrushes["Grey Red"];
	}
	else if (controlID == rightEdit.GetDlgCtrlID())
	{
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		CString textEdit;
		rightEdit.GetWindowTextA( textEdit );
		int rightSide;
		try
		{
			rightSide = boost::lexical_cast<int>( str( textEdit ) );
			if (rightSide == currentImageParameters.right)
			{
				// good.
				SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
				SetBkColor( hdcStatic, RGB( 100, 110, 100 ) );
				// catch change of color and redraw window.
				if (rightEdit.colorState != 0)
				{
					rightEdit.colorState = 0;
					rightEdit.RedrawWindow();
				}
				return *_myBrushes["Grey Green"];
			}
		}
		catch ( boost::bad_lexical_cast&)
		{
			// don't do anything with it.
		}
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		SetBkColor( hdcStatic, RGB( 150, 100, 100 ) );
		// catch change of color and redraw window.
		if (rightEdit.colorState != 1)
		{
			rightEdit.colorState = 1;
			rightEdit.RedrawWindow();
		}
		return *_myBrushes["Grey Red"];
	}
	else if (controlID == horBinningEdit.GetDlgCtrlID())
	{
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		CString textEdit;
		horBinningEdit.GetWindowTextA( textEdit );
		int horizontalBin;
		try
		{
			horizontalBin = boost::lexical_cast<int>( str( textEdit ) );
			if (horizontalBin == currentImageParameters.horizontalBinning)
			{
				// good.
				SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
				SetBkColor( hdcStatic, RGB( 100, 110, 100 ) );
				// catch change of color and redraw window.
				if (horBinningEdit.colorState != 0)
				{
					horBinningEdit.colorState = 0;
					horBinningEdit.RedrawWindow();
				}
				return *_myBrushes["Grey Green"];
			}
		}
		catch ( boost::bad_lexical_cast&)
		{
			// don't do anything with it.
		}
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		SetBkColor( hdcStatic, RGB( 150, 100, 100 ) );
		// catch change of color and redraw window.
		if (horBinningEdit.colorState != 1)
		{
			horBinningEdit.colorState = 1;
			horBinningEdit.RedrawWindow();
		}
		return *_myBrushes["Grey Red"];
	}*/
	return FALSE;
}


void ImageDimsControl::rearrange( int width, int height, fontMap fonts ) {}

