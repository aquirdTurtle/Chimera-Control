/// This file contains all functiosn for the CameraImageParametersControlParameters singleton class.

#include "stdafx.h"
#include "CameraImageDimensions.h"
#include "externals.h"
#include "constants.h"
#include "reorganizeControl.h"
#include "CameraWindow.h"
#include "CameraSettingsControl.h"


void CameraImageDimensionsControl::cameraIsOn( bool state )
{
	setImageDimensionsButton.EnableWindow( !state );
}


CameraImageDimensionsControl::CameraImageDimensionsControl()
{
	isReady = false;
}


void CameraImageDimensionsControl::initialize( cameraPositions& pos, CWnd* parent, bool isTriggerModeSensitive, int& id )
{
	setImageDimensionsButton.ID = id++;
	if (setImageDimensionsButton.ID != IDC_SET_IMAGE_PARAMETERS_BUTTON)
	{
		throw;
	}
	setImageDimensionsButton.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	setImageDimensionsButton.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 25 };
	setImageDimensionsButton.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y += 25 };
	setImageDimensionsButton.Create( "Set Image Dimensions", WS_TABSTOP | WS_CHILD | WS_VISIBLE,
									 setImageDimensionsButton.seriesPos, parent, setImageDimensionsButton.ID );

	setImageDimensionsButton.fontType = Normal;
	//
	leftText.ID = id++;
	leftText.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 160, pos.seriesPos.y + 25 };
	leftText.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 160, pos.amPos.y + 25 };
	leftText.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 160, pos.videoPos.y + 25 };
	leftText.Create( "Left", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, leftText.seriesPos, parent, leftText.ID );
	leftText.fontType = Normal;
	//
	rightText.ID = id++;
	rightText.seriesPos = { pos.seriesPos.x + 160, pos.seriesPos.y, pos.seriesPos.x + 320, pos.seriesPos.y + 25 };
	rightText.amPos = { pos.amPos.x + 160, pos.amPos.y, pos.amPos.x + 320, pos.amPos.y + 25 };
	rightText.videoPos = { pos.videoPos.x + 160, pos.videoPos.y, pos.videoPos.x + 320, pos.videoPos.y + 25 };
	rightText.Create( "Right", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, rightText.seriesPos, parent, rightText.ID );
	rightText.fontType = Normal;
	//
	horizontalBinningText.ID = id++;
	horizontalBinningText.seriesPos = { pos.seriesPos.x + 320, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	horizontalBinningText.amPos = { pos.amPos.x + 320, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 25 };
	horizontalBinningText.videoPos = { pos.videoPos.x + 320, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y += 25 };
	horizontalBinningText.Create( "H. Bin", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, horizontalBinningText.seriesPos, parent, 
								  horizontalBinningText.ID );
	horizontalBinningText.fontType = Normal;
	//
	leftEdit.ID = id++;
	leftEdit.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 160, pos.seriesPos.y + 25 };
	leftEdit.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 160, pos.amPos.y + 25 };
	leftEdit.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 160, pos.videoPos.y + 25 };
	leftEdit.Create( WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_CENTER, leftEdit.seriesPos, parent, leftEdit.ID );
	leftEdit.fontType = Normal;
	leftEdit.SetWindowTextA( "1" );
	//
	rightEdit.ID = id++;
	rightEdit.seriesPos = { pos.seriesPos.x + 160, pos.seriesPos.y, pos.seriesPos.x + 320, pos.seriesPos.y + 25 };
	rightEdit.amPos = { pos.amPos.x + 160, pos.amPos.y, pos.amPos.x + 320, pos.amPos.y + 25 };
	rightEdit.videoPos = { pos.videoPos.x + 160, pos.videoPos.y, pos.videoPos.x + 320, pos.videoPos.y + 25 };
	rightEdit.Create( WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_CENTER, rightEdit.seriesPos, parent, rightEdit.ID );
	rightEdit.fontType = Normal;
	rightEdit.SetWindowTextA( "50" );
	//
	horizontalBinningEdit.ID = id++;
	horizontalBinningEdit.seriesPos = { pos.seriesPos.x + 320, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	horizontalBinningEdit.amPos = { pos.amPos.x + 320, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 25 };
	horizontalBinningEdit.videoPos = { pos.videoPos.x + 320, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y += 25 };
	horizontalBinningEdit.Create( WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_CENTER, horizontalBinningEdit.seriesPos, parent, 
								  horizontalBinningEdit.ID );
	horizontalBinningEdit.fontType = Normal;
	horizontalBinningEdit.SetWindowTextA( "1" );
	//
	topText.ID = id++;
	topText.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 160, pos.seriesPos.y + 25 };
	topText.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 160, pos.amPos.y + 25 };
	topText.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 160, pos.videoPos.y + 25 };
	topText.Create( "Top", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, topText.seriesPos, parent, topText.ID );
	topText.fontType = Normal;

	//
	bottomText.ID = id++;
	bottomText.seriesPos = { pos.seriesPos.x + 160, pos.seriesPos.y, pos.seriesPos.x + 320, pos.seriesPos.y + 25 };
	bottomText.amPos = { pos.amPos.x + 160, pos.amPos.y, pos.amPos.x + 320, pos.amPos.y + 25 };
	bottomText.videoPos = { pos.videoPos.x + 160, pos.videoPos.y, pos.videoPos.x + 320, pos.videoPos.y + 25 };
	bottomText.Create( "Bottom", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, bottomText.seriesPos, parent, 
					   bottomText.ID );
	bottomText.fontType = Normal;
	//
	verticalBinningText.ID = id++;
	verticalBinningText.seriesPos = { pos.seriesPos.x + 320, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	verticalBinningText.amPos = { pos.amPos.x + 320, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 25 };
	verticalBinningText.videoPos = { pos.videoPos.x + 320, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y += 25 };
	verticalBinningText.Create( "V. Bin", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, verticalBinningText.seriesPos, parent, 
								verticalBinningText.ID );
	verticalBinningText.fontType = Normal;
	//
	topEdit.ID = id++;
	topEdit.seriesPos = { pos.seriesPos.x, pos.seriesPos.y, pos.seriesPos.x + 160, pos.seriesPos.y + 25 };
	topEdit.amPos = { pos.amPos.x, pos.amPos.y, pos.amPos.x + 160, pos.amPos.y + 25 };
	topEdit.videoPos = { pos.videoPos.x, pos.videoPos.y, pos.videoPos.x + 160, pos.videoPos.y + 25 };
	topEdit.Create( WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_CENTER, topEdit.seriesPos, parent, topEdit.ID );
	topEdit.fontType = Normal;
	topEdit.SetWindowTextA( "1" );
	//
	bottomEdit.ID = id++;
	bottomEdit.seriesPos = { pos.seriesPos.x + 160, pos.seriesPos.y, pos.seriesPos.x + 320, pos.seriesPos.y + 25 };
	bottomEdit.amPos = { pos.amPos.x + 160, pos.amPos.y, pos.amPos.x + 320, pos.amPos.y + 25 };
	bottomEdit.videoPos = { pos.videoPos.x + 160, pos.videoPos.y, pos.videoPos.x + 320, pos.videoPos.y + 25 };
	bottomEdit.Create( WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_CENTER, bottomEdit.seriesPos, parent, bottomEdit.ID );
	bottomEdit.fontType = Normal;
	bottomEdit.SetWindowTextA( "50" );
	//
	verticalBinningEdit.ID = id++;
	verticalBinningEdit.seriesPos = { pos.seriesPos.x + 320, pos.seriesPos.y, pos.seriesPos.x + 480, pos.seriesPos.y += 25 };
	verticalBinningEdit.amPos = { pos.amPos.x + 320, pos.amPos.y, pos.amPos.x + 480, pos.amPos.y += 25 };
	verticalBinningEdit.videoPos = { pos.videoPos.x + 320, pos.videoPos.y, pos.videoPos.x + 480, pos.videoPos.y += 25 };
	verticalBinningEdit.Create( WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_CENTER, verticalBinningEdit.seriesPos, parent, 
								verticalBinningEdit.ID );
	verticalBinningEdit.fontType = Normal;
	verticalBinningEdit.SetWindowTextA( "1" );
}


void CameraImageDimensionsControl::drawBackgrounds( CameraWindow* camWin )
{
	// recolor the box, clearing last run
	CDC* hDC = camWin->GetDC();
	SelectObject( *hDC, GetStockObject( DC_BRUSH ) );
	SelectObject( *hDC, GetStockObject( DC_PEN ) );
	// dark green brush
	SetDCBrushColor( *hDC, RGB( 0, 10, 0 ) );
	// Set the Pen to White
	SetDCPenColor( *hDC, RGB( 255, 255, 255 ) );
	// Drawing a rectangle with the current Device Context
	/*
	for (int imageInc = 0; imageInc < eImageBackgroundAreas.size(); imageInc++)
	{
		// slightly larger than the image zone.
		Rectangle(*hDC, eImageBackgroundAreas[imageInc].left - 5, eImageBackgroundAreas[imageInc].top - 5, eImageBackgroundAreas[imageInc].right + 5, eImageBackgroundAreas[imageInc].bottom + 5);
	}
	*/
	camWin->ReleaseDC( hDC );
}


imageParameters CameraImageDimensionsControl::readImageParameters( CameraWindow* camWin )
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
		currentImageParameters.leftBorder = std::stoi( std::string( tempStr ) );
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
		currentImageParameters.rightBorder = std::stoi( std::string( tempStr ) );
	}
	catch (std::invalid_argument&)
	{
		isReady = false;
		thrower( "Right border argument not an integer!\r\n" );
	}
	rightEdit.RedrawWindow();
	//
	topEdit.GetWindowTextA( tempStr );
	try
	{
		currentImageParameters.topBorder = std::stoi( std::string( tempStr ) );
	}
	catch (std::invalid_argument&)
	{
		isReady = false;
		thrower( "Top border argument not an integer!\r\n" );
	}
	topEdit.RedrawWindow();
	//
	bottomEdit.GetWindowTextA( tempStr );
	try
	{
		currentImageParameters.bottomBorder = std::stoi( std::string( tempStr ) );
	}
	catch (std::invalid_argument&)
	{
		isReady = false;
		thrower( "Bottom border argument not an integer!\r\n" );
	}
	bottomEdit.RedrawWindow();
	horizontalBinningEdit.GetWindowTextA( tempStr );
	try
	{
		currentImageParameters.horizontalBinning = std::stoi( std::string( tempStr ) );
	}
	catch (std::invalid_argument&)
	{
		isReady = false;
		thrower( "Horizontal binning argument not an integer!\r\n" );
	}
	horizontalBinningEdit.RedrawWindow();
	verticalBinningEdit.GetWindowTextA( tempStr );
	try
	{
		currentImageParameters.verticalBinning = std::stoi( std::string( tempStr ) );
	}
	catch (std::invalid_argument&)
	{
		isReady = false;
		thrower( "Vertical binning argument not an integer!\r\n" );
	}
	verticalBinningEdit.RedrawWindow();
	// reset this. There must be at least one pixel...
	/// TODO
	/*
	eCurrentlySelectedPixel.first = 0;
	eCurrentlySelectedPixel.second = 0;
	*/
	// Calculate the number of actual pixels in each dimension.
	currentImageParameters.width = (currentImageParameters.rightBorder - currentImageParameters.leftBorder + 1) / currentImageParameters.horizontalBinning;
	currentImageParameters.height = (currentImageParameters.bottomBorder - currentImageParameters.topBorder + 1) / currentImageParameters.verticalBinning;

	// Check Image parameters
	if (currentImageParameters.leftBorder > currentImageParameters.rightBorder || currentImageParameters.topBorder > currentImageParameters.bottomBorder)
	{
		isReady = false;
		thrower( "ERROR: Image start positions must not be greater than end positions\r\n" );
	}
	if (currentImageParameters.leftBorder < 1 || currentImageParameters.rightBorder > 512)
	{
		isReady = false;
		thrower( "ERROR: Image horizontal borders must be greater than 0 and less than the detector width\r\n" );
	}
	if (currentImageParameters.topBorder < 1 || currentImageParameters.bottomBorder > 512)
	{
		isReady = false;
		thrower( "ERROR: Image verttical borders must be greater than 0 and less than the detector height\r\n" );
	}
	if ((currentImageParameters.rightBorder - currentImageParameters.leftBorder + 1) % currentImageParameters.horizontalBinning != 0)
	{
		isReady = false;
		thrower( "ERROR: Image width must be a multiple of Horizontal Binning\r\n" );
	}
	if ((currentImageParameters.bottomBorder - currentImageParameters.topBorder + 1) % currentImageParameters.verticalBinning != 0)
	{
		isReady = false;
		thrower( "ERROR: Image height must be a multiple of Vertical Binning\r\n" );
	}
	// made it through successfully.
	/*
	for (int imageLocation = 0; imageLocation < eImageBackgroundAreas.size(); imageLocation++)
	{
		int imageBoxWidth = eImageBackgroundAreas[imageLocation].right - eImageBackgroundAreas[imageLocation].left + 1;
		int imageBoxHeight = eImageBackgroundAreas[imageLocation].bottom - eImageBackgroundAreas[imageLocation].top + 1;

		double boxWidth = imageBoxWidth / (double)currentImageParameters.width;
		double boxHeight = imageBoxHeight / (double)currentImageParameters.height;
		if (boxWidth > boxHeight)
		{
			// scale the box width down.
			eImageDrawAreas[imageLocation].left = eImageBackgroundAreas[imageLocation].left;
			eImageDrawAreas[imageLocation].right = (int)eImageBackgroundAreas[imageLocation].left
				+ (eImageBackgroundAreas[imageLocation].right - eImageBackgroundAreas[imageLocation].left) * boxHeight / boxWidth;
			double pixelsAreaWidth = eImageDrawAreas[imageLocation].right - eImageDrawAreas[imageLocation].left + 1;
			// move to center
			eImageDrawAreas[imageLocation].left += (imageBoxWidth - pixelsAreaWidth) / 2;
			eImageDrawAreas[imageLocation].right += (imageBoxWidth - pixelsAreaWidth) / 2;
			eImageDrawAreas[imageLocation].top = eImageBackgroundAreas[imageLocation].top;
			eImageDrawAreas[imageLocation].bottom = eImageBackgroundAreas[imageLocation].bottom;
			double pixelsAreaHeight = imageBoxHeight;
		}
		else
		{
			// cale the box height down.
			eImageDrawAreas[imageLocation].left = eImageBackgroundAreas[imageLocation].left;
			eImageDrawAreas[imageLocation].right = eImageBackgroundAreas[imageLocation].right;
			double pixelsAreaWidth = imageBoxWidth;
			// move to center
			eImageDrawAreas[imageLocation].top = eImageBackgroundAreas[imageLocation].top;
			eImageDrawAreas[imageLocation].bottom = (int)eImageBackgroundAreas[imageLocation].top + (eImageBackgroundAreas[imageLocation].bottom - eImageBackgroundAreas[imageLocation].top) * boxWidth / boxHeight;
			double pixelsAreaHeight = eImageDrawAreas[imageLocation].bottom - eImageDrawAreas[imageLocation].top + 1;
			eImageDrawAreas[imageLocation].top += (imageBoxWidth - pixelsAreaHeight) / 2;
			eImageDrawAreas[imageLocation].bottom += (imageBoxWidth - pixelsAreaHeight) / 2;
		}
	}
	// create rectangles for selection circle
	for (int pictureInc = 0; pictureInc < eImageDrawAreas.size(); pictureInc++)
	{
		ePixelRectangles[pictureInc].resize(currentImageParameters.width);
		for (int widthInc = 0; widthInc < currentImageParameters.width; widthInc++)
		{
			ePixelRectangles[pictureInc][widthInc].resize(currentImageParameters.height);
			for (int heightInc = 0; heightInc < currentImageParameters.height; heightInc++)
			{
				// for all 4 pictures...
				ePixelRectangles[pictureInc][widthInc][heightInc].left = (int)(eImageDrawAreas[pictureInc].left
					+ (double)widthInc * (eImageDrawAreas[pictureInc].right - eImageDrawAreas[pictureInc].left) / (double)currentImageParameters.width + 2);
				ePixelRectangles[pictureInc][widthInc][heightInc].right = (int)(eImageDrawAreas[pictureInc].left
					+ (double)(widthInc + 1) * (eImageDrawAreas[pictureInc].right - eImageDrawAreas[pictureInc].left) / (double)currentImageParameters.width + 2);
				ePixelRectangles[pictureInc][widthInc][heightInc].top = (int)(eImageDrawAreas[pictureInc].top
					+ (double)(heightInc)* (eImageDrawAreas[pictureInc].bottom - eImageDrawAreas[pictureInc].top) / (double)currentImageParameters.height);
				ePixelRectangles[pictureInc][widthInc][heightInc].bottom = (int)(eImageDrawAreas[pictureInc].top
					+ (double)(heightInc + 1)* (eImageDrawAreas[pictureInc].bottom - eImageDrawAreas[pictureInc].top) / (double)currentImageParameters.height);
			}
		}
	}
	*/
	// eCameraFileSystem.updateSaveStatus(false);
	isReady = true;
	return currentImageParameters;
}


/*
 * I forget why I needed a second function for this.
 */
void CameraImageDimensionsControl::setImageParametersFromInput( imageParameters param, CameraWindow* camWin )
{
	drawBackgrounds( camWin );
	//eDataExists = false;
	// set all of the image parameters
	currentImageParameters.leftBorder = param.leftBorder;
	leftEdit.SetWindowText( std::to_string( currentImageParameters.leftBorder ).c_str() );
	currentImageParameters.rightBorder = param.rightBorder;
	rightEdit.SetWindowText( std::to_string( currentImageParameters.rightBorder ).c_str() );
	currentImageParameters.topBorder = param.topBorder;
	topEdit.SetWindowText( std::to_string( currentImageParameters.topBorder ).c_str() );
	currentImageParameters.bottomBorder = param.bottomBorder;
	bottomEdit.SetWindowText( std::to_string( currentImageParameters.bottomBorder ).c_str() );
	currentImageParameters.horizontalBinning = param.horizontalBinning;
	horizontalBinningEdit.SetWindowText( std::to_string( currentImageParameters.horizontalBinning ).c_str() );
	currentImageParameters.verticalBinning = param.verticalBinning;
	verticalBinningEdit.SetWindowText( std::to_string( currentImageParameters.verticalBinning ).c_str() );
	// reset this. There must be at least one pixel...
	/*
	eCurrentlySelectedPixel.first = 0;
	eCurrentlySelectedPixel.second = 0;
	*/
	// Calculate the number of actual pixels in each dimension.
	currentImageParameters.width = (currentImageParameters.rightBorder - currentImageParameters.leftBorder + 1) / currentImageParameters.horizontalBinning;
	currentImageParameters.height = (currentImageParameters.bottomBorder - currentImageParameters.topBorder + 1) / currentImageParameters.verticalBinning;

	// Check Image parameters
	if (currentImageParameters.leftBorder > currentImageParameters.rightBorder || currentImageParameters.topBorder > currentImageParameters.bottomBorder)
	{
		isReady = false;
		thrower( "ERROR: Image start positions must not be greater than end positions\r\n" );
	}
	if (currentImageParameters.leftBorder < 1 || currentImageParameters.rightBorder > 512)
	{
		isReady = false;
		thrower( "ERROR: Image horizontal borders must be greater than 0 and less than the detector width\r\n" );
	}
	if (currentImageParameters.topBorder < 1 || currentImageParameters.bottomBorder > 512)
	{
		isReady = false;
		thrower( "ERROR: Image verttical borders must be greater than 0 and less than the detector height\r\n" );
	}
	if ((currentImageParameters.rightBorder - currentImageParameters.leftBorder + 1) % currentImageParameters.horizontalBinning != 0)
	{
		isReady = false;
		thrower( "ERROR: Image width must be a multiple of Horizontal Binning\r\n" );
	}
	if ((currentImageParameters.bottomBorder - currentImageParameters.topBorder + 1) % currentImageParameters.verticalBinning != 0)
	{
		isReady = false;
		thrower( "ERROR: Image height must be a multiple of Vertical Binning\r\n" );
	}
	// made it through successfully.
	/*
	for (int imageLocation = 0; imageLocation < eImageBackgroundAreas.size(); imageLocation++)
	{
		int imageBoxWidth = eImageBackgroundAreas[imageLocation].right - eImageBackgroundAreas[imageLocation].left + 1;
		int imageBoxHeight = eImageBackgroundAreas[imageLocation].bottom - eImageBackgroundAreas[imageLocation].top + 1;

		double boxWidth = imageBoxWidth / (double)currentImageParameters.width;
		double boxHeight = imageBoxHeight / (double)currentImageParameters.height;
		if (boxWidth > boxHeight)
		{
			// scale the box width down.
			eImageDrawAreas[imageLocation].left = eImageBackgroundAreas[imageLocation].left;
			eImageDrawAreas[imageLocation].right = (int)eImageBackgroundAreas[imageLocation].left
				+ (eImageBackgroundAreas[imageLocation].right - eImageBackgroundAreas[imageLocation].left) * boxHeight / boxWidth;
			double pixelsAreaWidth = eImageDrawAreas[imageLocation].right - eImageDrawAreas[imageLocation].left + 1;
			// move to center
			eImageDrawAreas[imageLocation].left += (imageBoxWidth - pixelsAreaWidth) / 2;
			eImageDrawAreas[imageLocation].right += (imageBoxWidth - pixelsAreaWidth) / 2;
			eImageDrawAreas[imageLocation].top = eImageBackgroundAreas[imageLocation].top;
			eImageDrawAreas[imageLocation].bottom = eImageBackgroundAreas[imageLocation].bottom;
			double pixelsAreaHeight = imageBoxHeight;
		}
		else
		{
			// cale the box height down.
			eImageDrawAreas[imageLocation].left = eImageBackgroundAreas[imageLocation].left;
			eImageDrawAreas[imageLocation].right = eImageBackgroundAreas[imageLocation].right;
			double pixelsAreaWidth = imageBoxWidth;
			// move to center
			eImageDrawAreas[imageLocation].top = eImageBackgroundAreas[imageLocation].top;
			eImageDrawAreas[imageLocation].bottom = (int)eImageBackgroundAreas[imageLocation].top + (eImageBackgroundAreas[imageLocation].bottom - eImageBackgroundAreas[imageLocation].top) * boxWidth / boxHeight;
			double pixelsAreaHeight = eImageDrawAreas[imageLocation].bottom - eImageDrawAreas[imageLocation].top + 1;
			eImageDrawAreas[imageLocation].top += (imageBoxWidth - pixelsAreaHeight) / 2;
			eImageDrawAreas[imageLocation].bottom += (imageBoxWidth - pixelsAreaHeight) / 2;
		}
	}
	// create rectangles for selection circle
	for (int pictureInc = 0; pictureInc < eImageDrawAreas.size(); pictureInc++)
	{
		ePixelRectangles[pictureInc].resize(currentImageParameters.width);
		for (int widthInc = 0; widthInc < currentImageParameters.width; widthInc++)
		{
			ePixelRectangles[pictureInc][widthInc].resize(currentImageParameters.height);
			for (int heightInc = 0; heightInc < currentImageParameters.height; heightInc++)
			{
				// for all 4 pictures...
				ePixelRectangles[pictureInc][widthInc][heightInc].left = (int)(eImageDrawAreas[pictureInc].left
					+ (double)widthInc * (eImageDrawAreas[pictureInc].right - eImageDrawAreas[pictureInc].left)
					/ (double)currentImageParameters.width + 2);
				ePixelRectangles[pictureInc][widthInc][heightInc].right = (int)(eImageDrawAreas[pictureInc].left
					+ (double)(widthInc + 1) * (eImageDrawAreas[pictureInc].right - eImageDrawAreas[pictureInc].left)
					/ (double)currentImageParameters.width + 2);
				ePixelRectangles[pictureInc][widthInc][heightInc].top = (int)(eImageDrawAreas[pictureInc].top
					+ (double)(heightInc)* (eImageDrawAreas[pictureInc].bottom - eImageDrawAreas[pictureInc].top)
					/ (double)currentImageParameters.height);
				ePixelRectangles[pictureInc][widthInc][heightInc].bottom = (int)(eImageDrawAreas[pictureInc].top
					+ (double)(heightInc + 1)* (eImageDrawAreas[pictureInc].bottom - eImageDrawAreas[pictureInc].top)
					/ (double)currentImageParameters.height);
			}
		}
	}
	this->drawRectangleFrame();
	*/
	// eCameraFileSystem.updateSaveStatus(false);
	isReady = true;
}


bool CameraImageDimensionsControl::checkReady()
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


imageParameters CameraImageDimensionsControl::getImageParameters()
{
	return currentImageParameters;
}


HBRUSH CameraImageDimensionsControl::colorEdits( HWND window, UINT message, WPARAM wParam, LPARAM lParam, MainWindow* mainWin )
{
	brushMap brushes = mainWin->getBrushes();

	rgbMap rgbs = mainWin->getRGB();
	DWORD controlID = GetDlgCtrlID( (HWND)lParam );
	HDC hdcStatic = (HDC)wParam;
	imageParameters currentImageParameters = { 0,0,0,0,0,0 };

	if (controlID == bottomEdit.ID)
	{
		//imageParameters currentImageParameters = eImageControl.getImageParameters();
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		CString textEdit;
		bottomEdit.GetWindowTextA( textEdit );
		int bottom;
		try
		{
			bottom = std::stoi( std::string( textEdit ) );
			if (bottom == currentImageParameters.bottomBorder)
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
	else if (controlID == topEdit.ID)
	{
		//imageParameters currentImageParameters = eImageControl.getImageParameters();
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		CString textEdit;
		topEdit.GetWindowTextA( textEdit );
		int top;
		try
		{
			top = std::stoi( std::string( textEdit ) );
			if (top == currentImageParameters.topBorder)
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
	else if (controlID == verticalBinningEdit.ID)
	{
		//imageParameters currentImageParameters = eImageControl.getImageParameters();
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		CString textEdit;
		verticalBinningEdit.GetWindowTextA( textEdit );
		int verticalBin;
		try
		{
			verticalBin = std::stoi( std::string( textEdit ) );
			if (verticalBin == currentImageParameters.verticalBinning)
			{
				// good.
				SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
				SetBkColor( hdcStatic, RGB( 100, 110, 100 ) );
				// catch change of color and redraw window.
				if (verticalBinningEdit.colorState != 0)
				{
					verticalBinningEdit.colorState = 0;
					verticalBinningEdit.RedrawWindow();
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
		if (verticalBinningEdit.colorState != 1)
		{
			verticalBinningEdit.colorState = 1;
			verticalBinningEdit.RedrawWindow();
		}
		return *brushes["Grey Red"];
	}
	else if (controlID == leftEdit.ID)
	{
		//imageParameters currentImageParameters = eImageControl.getImageParameters();
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		CString textEdit;
		leftEdit.GetWindowTextA( textEdit );
		int leftSide;
		try
		{
			leftSide = std::stoi( std::string( textEdit ) );
			if (leftSide == currentImageParameters.leftBorder)
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
			else
			{
				//appendText(std::to_string(leftSide) + ", " + std::to_string(currentImageParameters.leftBorder), IDC_ERROR_EDIT);
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
	else if (controlID == rightEdit.ID)
	{
		//imageParameters currentImageParameters = eImageControl.getImageParameters();
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		CString textEdit;
		rightEdit.GetWindowTextA( textEdit );
		int rightSide;
		try
		{
			rightSide = std::stoi( std::string( textEdit ) );
			if (rightSide == currentImageParameters.rightBorder)
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
	else if (controlID == horizontalBinningEdit.ID)
	{
		//imageParameters currentImageParameters = eImageControl.getImageParameters();
		SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
		CString textEdit;
		horizontalBinningEdit.GetWindowTextA( textEdit );
		int horizontalBin;
		try
		{
			horizontalBin = std::stoi( std::string( textEdit ) );
			if (horizontalBin == currentImageParameters.horizontalBinning)
			{
				// good.
				SetTextColor( hdcStatic, RGB( 255, 255, 255 ) );
				SetBkColor( hdcStatic, RGB( 100, 110, 100 ) );
				// catch change of color and redraw window.
				if (horizontalBinningEdit.colorState != 0)
				{
					horizontalBinningEdit.colorState = 0;
					horizontalBinningEdit.RedrawWindow();
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
		if (horizontalBinningEdit.colorState != 1)
		{
			horizontalBinningEdit.colorState = 1;
			horizontalBinningEdit.RedrawWindow();
		}
		return *brushes["Grey Red"];
	}
	return FALSE;
}


void CameraImageDimensionsControl::rearrange( std::string cameraMode, std::string triggerMode, int width, int height, 
											  fontMap fonts )
{
	leftText.rearrange( cameraMode, triggerMode, width, height, fonts );
	rightText.rearrange( cameraMode, triggerMode, width, height, fonts );
	horizontalBinningText.rearrange( cameraMode, triggerMode, width, height, fonts );
	topText.rearrange( cameraMode, triggerMode, width, height, fonts );
	bottomText.rearrange( cameraMode, triggerMode, width, height, fonts );
	verticalBinningText.rearrange( cameraMode, triggerMode, width, height, fonts );
	leftEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
	rightEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
	horizontalBinningEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
	topEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
	bottomEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
	verticalBinningEdit.rearrange( cameraMode, triggerMode, width, height, fonts );
	setImageDimensionsButton.rearrange( cameraMode, triggerMode, width, height, fonts );
}

