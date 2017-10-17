/// This file contains all functiosn for the CameraImageParametersControlParameters singleton class.

#include "stdafx.h"
#include "CameraImageDimensions.h"
#include "CameraWindow.h"
#include "CameraSettingsControl.h"


ImageDimsControl::ImageDimsControl()
{
	isReady = false;
}


void ImageDimsControl::initialize( cameraPositions& pos, CWnd* parent, bool isTriggerModeSensitive, int& id )
{
	//
	leftText.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 160, pos.seriesPos.y + 25 };
	leftText.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 160, pos.amPos.y + 25 };
	leftText.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 160, pos.videoPos.y + 25 };
	leftText.Create( "Left", NORM_STATIC_OPTIONS, leftText.seriesPos, parent, id++);
	//
	rightText.seriesPos = { pos.seriesPos.x + 160, pos.seriesPos.y, pos.seriesPos.x + 320, pos.seriesPos.y + 25 };
	rightText.amPos = { pos.amPos.x + 160, pos.amPos.y, pos.amPos.x + 320, pos.amPos.y + 25 };
	rightText.videoPos = { pos.videoPos.x + 160, pos.videoPos.y, pos.videoPos.x + 320, pos.videoPos.y + 25 };
	rightText.Create( "Right", NORM_STATIC_OPTIONS, rightText.seriesPos,
					  parent, id++ );
	//
	horBinningText.seriesPos = { pos.seriesPos.x + 320, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	horBinningText.amPos = { pos.amPos.x + 320, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 25 };
	horBinningText.videoPos = { pos.videoPos.x + 320, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y += 25 };
	horBinningText.Create( "H. Bin", NORM_STATIC_OPTIONS, horBinningText.seriesPos, parent, id++);
	//
	leftEdit.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 160, pos.seriesPos.y + 25 };
	leftEdit.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 160, pos.amPos.y + 25 };
	leftEdit.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 160, pos.videoPos.y + 25 };
	leftEdit.Create( NORM_EDIT_OPTIONS | ES_CENTER, leftEdit.seriesPos, parent, id++);
	leftEdit.SetWindowTextA( "1" );
	//
	rightEdit.seriesPos = { pos.seriesPos.x + 160, pos.seriesPos.y, pos.seriesPos.x + 320, pos.seriesPos.y + 25 };
	rightEdit.amPos = { pos.amPos.x + 160, pos.amPos.y, pos.amPos.x + 320, pos.amPos.y + 25 };
	rightEdit.videoPos = { pos.videoPos.x + 160, pos.videoPos.y, pos.videoPos.x + 320, pos.videoPos.y + 25 };
	rightEdit.Create( NORM_EDIT_OPTIONS | ES_CENTER, rightEdit.seriesPos, parent, id++);
	rightEdit.SetWindowTextA( "50" );
	//
	horBinningEdit.seriesPos = { pos.seriesPos.x + 320, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	horBinningEdit.amPos = { pos.amPos.x + 320, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 25 };
	horBinningEdit.videoPos = { pos.videoPos.x + 320, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y += 25 };
	horBinningEdit.Create( NORM_EDIT_OPTIONS | ES_CENTER, horBinningEdit.seriesPos, parent, id++ );
	horBinningEdit.SetWindowTextA( "1" );
	//
	bottomLabel.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 160, pos.seriesPos.y + 25 };
	bottomLabel.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 160, pos.amPos.y + 25 };
	bottomLabel.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 160, pos.videoPos.y + 25 };
	bottomLabel.Create( "Bottom", NORM_STATIC_OPTIONS, bottomLabel.seriesPos, parent, id++);

	//
	topLabel.seriesPos = { pos.seriesPos.x + 160, pos.seriesPos.y, pos.seriesPos.x + 320, pos.seriesPos.y + 25 };
	topLabel.amPos = { pos.amPos.x + 160, pos.amPos.y, pos.amPos.x + 320, pos.amPos.y + 25 };
	topLabel.videoPos = { pos.videoPos.x + 160, pos.videoPos.y, pos.videoPos.x + 320, pos.videoPos.y + 25 };
	topLabel.Create( "Top", NORM_STATIC_OPTIONS, topLabel.seriesPos, parent, id++);
	//
	vertBinningText.seriesPos = { pos.seriesPos.x + 320, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	vertBinningText.amPos = { pos.amPos.x + 320, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 25 };
	vertBinningText.videoPos = { pos.videoPos.x + 320, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y += 25 };
	vertBinningText.Create( "V. Bin", NORM_STATIC_OPTIONS, vertBinningText.seriesPos, parent, id++);
	//
	bottomEdit.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 160, pos.seriesPos.y + 25 };
	bottomEdit.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 160, pos.amPos.y + 25 };
	bottomEdit.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 160, pos.videoPos.y + 25 };
	bottomEdit.Create( NORM_EDIT_OPTIONS | ES_CENTER, bottomEdit.seriesPos, parent, id++);
	bottomEdit.SetWindowTextA( "1" );
	//
	topEdit.seriesPos = { pos.seriesPos.x + 160, pos.seriesPos.y, pos.seriesPos.x + 320, pos.seriesPos.y + 25 };
	topEdit.amPos = { pos.amPos.x + 160, pos.amPos.y, pos.amPos.x + 320, pos.amPos.y + 25 };
	topEdit.videoPos = { pos.videoPos.x + 160, pos.videoPos.y, pos.videoPos.x + 320, pos.videoPos.y + 25 };
	topEdit.Create( NORM_EDIT_OPTIONS | ES_CENTER, topEdit.seriesPos, parent, id++);
	topEdit.SetWindowTextA( "50" );
	//
	vertBinningEdit.seriesPos = { pos.seriesPos.x + 320, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	vertBinningEdit.amPos = { pos.amPos.x + 320, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 25 };
	vertBinningEdit.videoPos = { pos.videoPos.x + 320, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y += 25 };
	vertBinningEdit.Create( NORM_EDIT_OPTIONS | ES_CENTER, vertBinningEdit.seriesPos, parent, id++ );
	vertBinningEdit.SetWindowTextA( "1" );
}


void ImageDimsControl::drawBackgrounds( CameraWindow* camWin )
{
	// recolor the box, clearing last run
	CDC* dc = camWin->GetDC();
	dc->SelectObject( GetStockObject( DC_BRUSH ) );
	dc->SelectObject( GetStockObject( DC_PEN ) );
	// dark green brush
	dc->SetDCBrushColor( RGB( 0, 10, 0 ) );
	dc->SetDCPenColor( RGB( 255, 255, 255 ) );
	// Drawing a rectangle with the current Device Context
	// ??? Is there something missing here??? There's no draw...
	camWin->ReleaseDC( dc );
}


void ImageDimsControl::handleSave( std::ofstream& saveFile )
{
	saveFile << "CAMERA_IMAGE_DIMENSIONS\n";
	saveFile << currentImageParameters.left << " " << currentImageParameters.right << " "
		<< currentImageParameters.horizontalBinning << "\n";
	saveFile << currentImageParameters.bottom << " " << currentImageParameters.top << " "
		<< currentImageParameters.verticalBinning << "\n";
	saveFile << "END_CAMERA_IMAGE_DIMENSIONS\n";
}


void ImageDimsControl::handleNew( std::ofstream& newfile )
{
	newfile << "CAMERA_IMAGE_DIMENSIONS\n";
	newfile << "1 512 1\n";
	newfile << "1 512 1\n";
	newfile << "END_CAMERA_IMAGE_DIMENSIONS\n";
}


void ImageDimsControl::handleOpen( std::ifstream& openFile, int versionMajor, int versionMinor )
{
	ProfileSystem::checkDelimiterLine( openFile, "CAMERA_IMAGE_DIMENSIONS" );
	imageParameters params;
	openFile >> params.left;
	openFile >> params.right;
	openFile >> params.horizontalBinning;
	openFile >> params.bottom;
	openFile >> params.top;
	openFile >> params.verticalBinning;
	setImageParametersFromInput( params, NULL );
	ProfileSystem::checkDelimiterLine( openFile, "END_CAMERA_IMAGE_DIMENSIONS" );
	
}


imageParameters ImageDimsControl::readImageParameters( CameraWindow* camWin )
{
	// in case called before initialized
	if (!leftEdit)
	{
		return currentImageParameters;
	}
	drawBackgrounds( camWin );
	// If new dimensions are set, we don't have data for the new dimensions.
	// eDataExists = false;
	// set all of the image parameters
	CString tempStr;
	leftEdit.GetWindowTextA( tempStr );
	try
	{
		currentImageParameters.left = std::stoi( str( tempStr ) );
	}
	catch (std::invalid_argument&)
	{
		isReady = false;
		thrower( "Left border argument not an integer!\r\n" );
	}
	leftEdit.RedrawWindow();
	rightEdit.GetWindowTextA( tempStr );
	try
	{
		currentImageParameters.right = std::stoi( str( tempStr ) );
	}
	catch (std::invalid_argument&)
	{
		isReady = false;
		thrower( "Right border argument not an integer!\r\n" );
	}
	rightEdit.RedrawWindow();
	//
	bottomEdit.GetWindowTextA( tempStr );
	try
	{
		currentImageParameters.bottom = std::stoi( str( tempStr ) );
	}
	catch (std::invalid_argument&)
	{
		isReady = false;
		thrower( "Top border argument not an integer!\r\n" );
	}
	bottomEdit.RedrawWindow();
	//
	topEdit.GetWindowTextA( tempStr );
	try
	{
		currentImageParameters.top = std::stoi( str( tempStr ) );
	}
	catch (std::invalid_argument&)
	{
		isReady = false;
		thrower( "Bottom border argument not an integer!\r\n" );
	}
	topEdit.RedrawWindow();
	horBinningEdit.GetWindowTextA( tempStr );
	try
	{
		currentImageParameters.horizontalBinning = std::stoi( str( tempStr ) );
	}
	catch (std::invalid_argument&)
	{
		isReady = false;
		thrower( "Horizontal binning argument not an integer!\r\n" );
	}
	horBinningEdit.RedrawWindow();
	vertBinningEdit.GetWindowTextA( tempStr );
	try
	{
		currentImageParameters.verticalBinning = std::stoi( str( tempStr ) );
	}
	catch (std::invalid_argument&)
	{
		isReady = false;
		thrower( "Vertical binning argument not an integer!\r\n" );
	}
	vertBinningEdit.RedrawWindow();
	updateWidthHeight( );

	// Check Image parameters
	if (currentImageParameters.left > currentImageParameters.right || currentImageParameters.bottom > currentImageParameters.top)
	{
		isReady = false;
		thrower( "ERROR: Image start positions must not be greater than end positions\r\n" );
	}
	if (currentImageParameters.left < 1 || currentImageParameters.right > 512)
	{
		isReady = false;
		thrower( "ERROR: Image horizontal borders must be greater than 0 and less than the detector width\r\n" );
	}
	if (currentImageParameters.bottom < 1 || currentImageParameters.top > 512)
	{
		isReady = false;
		thrower( "ERROR: Image verttical borders must be greater than 0 and less than the detector height\r\n" );
	}
	if ((currentImageParameters.right - currentImageParameters.left + 1) % currentImageParameters.horizontalBinning != 0)
	{
		isReady = false;
		thrower( "ERROR: Image width must be a multiple of Horizontal Binning\r\n" );
	}
	if ((currentImageParameters.top - currentImageParameters.bottom + 1) % currentImageParameters.verticalBinning != 0)
	{
		isReady = false;
		thrower( "ERROR: Image height must be a multiple of Vertical Binning\r\n" );
	}
	// made it through successfully.
	isReady = true;
	return currentImageParameters;
}


// Calculate the number of actual pixels in each dimension.
void ImageDimsControl::updateWidthHeight( )
{
	currentImageParameters.width = (currentImageParameters.right - currentImageParameters.left + 1) / currentImageParameters.horizontalBinning;
	currentImageParameters.height = (currentImageParameters.top - currentImageParameters.bottom + 1) / currentImageParameters.verticalBinning;
}

/*
 * I forget why I needed a second function for this.
 */
void ImageDimsControl::setImageParametersFromInput( imageParameters param, CameraWindow* camWin )
{
	if ( camWin != NULL )
	{
		drawBackgrounds( camWin );
	}
	// set all of the image parameters
	currentImageParameters.left = param.left;
	leftEdit.SetWindowText( cstr( currentImageParameters.left ) );
	currentImageParameters.right = param.right;
	rightEdit.SetWindowText( cstr( currentImageParameters.right ) );
	currentImageParameters.bottom = param.bottom;
	bottomEdit.SetWindowText( cstr( currentImageParameters.bottom ) );
	currentImageParameters.top = param.top;
	topEdit.SetWindowText( cstr( currentImageParameters.top ) );
	currentImageParameters.horizontalBinning = param.horizontalBinning;
	horBinningEdit.SetWindowText( cstr( currentImageParameters.horizontalBinning ) );
	currentImageParameters.verticalBinning = param.verticalBinning;
	vertBinningEdit.SetWindowText( cstr( currentImageParameters.verticalBinning ) );
	// Calculate the number of actual pixels in each dimension.
	updateWidthHeight( );

	// Check Image parameters
	if (currentImageParameters.left > currentImageParameters.right || currentImageParameters.bottom > currentImageParameters.top)
	{
		isReady = false;
		thrower( "ERROR: Image start positions must not be greater than end positions\r\n" );
	}
	if (currentImageParameters.left < 1 || currentImageParameters.right > 512)
	{
		isReady = false;
		thrower( "ERROR: Image horizontal borders must be greater than 0 and less than the detector width\r\n" );
	}
	if (currentImageParameters.bottom < 1 || currentImageParameters.top > 512)
	{
		isReady = false;
		thrower( "ERROR: Image verttical borders must be greater than 0 and less than the detector height\r\n" );
	}
	if ((currentImageParameters.right - currentImageParameters.left + 1) % currentImageParameters.horizontalBinning != 0)
	{
		isReady = false;
		thrower( "ERROR: Image width must be a multiple of Horizontal Binning\r\n" );
	}
	if ((currentImageParameters.top - currentImageParameters.bottom + 1) % currentImageParameters.verticalBinning != 0)
	{
		isReady = false;
		thrower( "ERROR: Image height must be a multiple of Vertical Binning\r\n" );
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
	brushMap brushes = mainWin->getBrushes();

	rgbMap rgbs = mainWin->getRgbs();
	DWORD controlID = GetDlgCtrlID( (HWND)lParam );
	HDC hdcStatic = (HDC)wParam;
	imageParameters currentImageParameters = { 0,0,0,0,0,0 };

	if (controlID == topEdit.GetDlgCtrlID())
	{
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		CString textEdit;
		topEdit.GetWindowTextA( textEdit );
		int bottom;
		try
		{
			bottom = std::stoi( str( textEdit ) );
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
				return *brushes["Grey Green"];
			}
		}
		catch (std::exception&)
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
		return *brushes["Grey Red"];
	}
	else if (controlID == bottomEdit.GetDlgCtrlID())
	{
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		CString textEdit;
		bottomEdit.GetWindowTextA( textEdit );
		int top;
		try
		{
			top = std::stoi( str( textEdit ) );
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
				return *brushes["Grey Green"];
			}
		}
		catch (std::exception&)
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
		return *brushes["Grey Red"];
	}
	else if (controlID == vertBinningEdit.GetDlgCtrlID())
	{
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		CString textEdit;
		vertBinningEdit.GetWindowTextA( textEdit );
		int verticalBin;
		try
		{
			verticalBin = std::stoi( str( textEdit ) );
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
				return *brushes["Grey Green"];
			}
		}
		catch (std::exception&)
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
		return *brushes["Grey Red"];
	}
	else if (controlID == leftEdit.GetDlgCtrlID())
	{
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		CString textEdit;
		leftEdit.GetWindowTextA( textEdit );
		int leftSide;
		try
		{
			leftSide = std::stoi( str( textEdit ) );
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
				return *brushes["Grey Green"];
			}
		}
		catch (std::exception&)
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
		return *brushes["Grey Red"];
	}
	else if (controlID == rightEdit.GetDlgCtrlID())
	{
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		CString textEdit;
		rightEdit.GetWindowTextA( textEdit );
		int rightSide;
		try
		{
			rightSide = std::stoi( str( textEdit ) );
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
				return *brushes["Grey Green"];
			}
		}
		catch (std::exception&)
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
		return *brushes["Grey Red"];
	}
	else if (controlID == horBinningEdit.GetDlgCtrlID())
	{
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		CString textEdit;
		horBinningEdit.GetWindowTextA( textEdit );
		int horizontalBin;
		try
		{
			horizontalBin = std::stoi( str( textEdit ) );
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
				return *brushes["Grey Green"];
			}
		}
		catch (std::exception&)
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
		return *brushes["Grey Red"];
	}
	return FALSE;
}


void ImageDimsControl::rearrange( std::string cameraMode, std::string triggerMode, int width, int height, 
											  fontMap fonts )
{
	leftText.rearrange( cameraMode, triggerMode, width, height, fonts );
	rightText.rearrange( cameraMode, triggerMode, width, height, fonts );
	horBinningText.rearrange( cameraMode, triggerMode, width, height, fonts );
	bottomLabel.rearrange( cameraMode, triggerMode, width, height, fonts );
	topLabel.rearrange( cameraMode, triggerMode, width, height, fonts );
	vertBinningText.rearrange( cameraMode, triggerMode, width, height, fonts );
	leftEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
	rightEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
	horBinningEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
	bottomEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
	topEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
	vertBinningEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
}

