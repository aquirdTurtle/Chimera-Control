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
	this->setImageDimensionsButton.EnableWindow( !state );
	return;
}


CameraImageDimensionsControl::CameraImageDimensionsControl()
{
	isReady = false;
}

bool CameraImageDimensionsControl::initialize( POINT& topLeftPositionKinetic, POINT& topLeftPositionAccumulate, POINT& topLeftPositionContinuous,
											   CWnd* parent, bool isTriggerModeSensitive, int& id )
{
	setImageDimensionsButton.ID = id++;
	if (setImageDimensionsButton.ID != IDC_SET_IMAGE_PARAMETERS_BUTTON)
	{
		throw;
	}
	setImageDimensionsButton.ksmPos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 480, topLeftPositionKinetic.y + 25 };
	setImageDimensionsButton.amPos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 480, topLeftPositionAccumulate.y + 25 };
	setImageDimensionsButton.cssmPos = { topLeftPositionContinuous.x, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 480, topLeftPositionContinuous.y + 25 };
	setImageDimensionsButton.Create( "Set Image Dimensions", WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
									 setImageDimensionsButton.ksmPos, parent, setImageDimensionsButton.ID );
	setImageDimensionsButton.fontType = "Normal";
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;
	topLeftPositionContinuous.y += 25;
	//
	leftText.ID = id++;
	leftText.ksmPos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 160, topLeftPositionKinetic.y + 25 };
	leftText.amPos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 160, topLeftPositionAccumulate.y + 25 };
	leftText.cssmPos = { topLeftPositionContinuous.x, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 160, topLeftPositionContinuous.y + 25 };
	leftText.Create( "Left", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, leftText.ksmPos, parent, leftText.ID );
	leftText.fontType = "Normal";
	//
	rightText.ID = id++;
	rightText.ksmPos = { topLeftPositionKinetic.x + 160, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 320, topLeftPositionKinetic.y + 25 };
	rightText.amPos = { topLeftPositionAccumulate.x + 160, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 320, topLeftPositionAccumulate.y + 25 };
	rightText.cssmPos = { topLeftPositionContinuous.x + 160, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 320, topLeftPositionContinuous.y + 25 };
	rightText.Create( "Right", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, rightText.ksmPos, parent, rightText.ID );
	rightText.fontType = "Normal";
	//
	horizontalBinningText.ID = id++;
	horizontalBinningText.ksmPos = { topLeftPositionKinetic.x + 320, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 480, topLeftPositionKinetic.y + 25 };
	horizontalBinningText.amPos = { topLeftPositionAccumulate.x + 320, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 480, topLeftPositionAccumulate.y + 25 };
	horizontalBinningText.cssmPos = { topLeftPositionContinuous.x + 320, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 480, topLeftPositionContinuous.y + 25 };
	horizontalBinningText.Create( "H. Bin", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, horizontalBinningText.ksmPos, parent, horizontalBinningText.ID );
	horizontalBinningText.fontType = "Normal";
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;
	topLeftPositionContinuous.y += 25;
	//
	leftEdit.ID = id++;
	leftEdit.ksmPos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 160, topLeftPositionKinetic.y + 25 };
	leftEdit.amPos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 160, topLeftPositionAccumulate.y + 25 };
	leftEdit.cssmPos = { topLeftPositionContinuous.x, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 160, topLeftPositionContinuous.y + 25 };
	leftEdit.Create( WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_CENTER, leftEdit.ksmPos, parent, leftEdit.ID );
	leftEdit.fontType = "Normal";
	leftEdit.SetWindowTextA( "1" );
	//
	rightEdit.ID = id++;
	rightEdit.ksmPos = { topLeftPositionKinetic.x + 160, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 320, topLeftPositionKinetic.y + 25 };
	rightEdit.amPos = { topLeftPositionAccumulate.x + 160, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 320, topLeftPositionAccumulate.y + 25 };
	rightEdit.cssmPos = { topLeftPositionContinuous.x + 160, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 320, topLeftPositionContinuous.y + 25 };
	rightEdit.Create( WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_CENTER, rightEdit.ksmPos, parent, rightEdit.ID );
	rightEdit.fontType = "Normal";
	rightEdit.SetWindowTextA( "10" );
	//
	horizontalBinningEdit.ID = id++;
	horizontalBinningEdit.ksmPos = { topLeftPositionKinetic.x + 320, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 480, topLeftPositionKinetic.y + 25 };
	horizontalBinningEdit.amPos = { topLeftPositionAccumulate.x + 320, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 480, topLeftPositionAccumulate.y + 25 };
	horizontalBinningEdit.cssmPos = { topLeftPositionContinuous.x + 320, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 480, topLeftPositionContinuous.y + 25 };
	horizontalBinningEdit.Create( WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_CENTER, horizontalBinningEdit.ksmPos, parent, horizontalBinningEdit.ID );
	horizontalBinningEdit.fontType = "Normal";
	horizontalBinningEdit.SetWindowTextA( "1" );
	//
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;
	topLeftPositionContinuous.y += 25;
	topText.ID = id++;
	topText.ksmPos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 160, topLeftPositionKinetic.y + 25 };
	topText.amPos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 160, topLeftPositionAccumulate.y + 25 };
	topText.cssmPos = { topLeftPositionContinuous.x, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 160, topLeftPositionContinuous.y + 25 };
	topText.Create( "Top", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, topText.ksmPos, parent, topText.ID );
	topText.fontType = "Normal";

	//
	bottomText.ID = id++;
	bottomText.ksmPos = { topLeftPositionKinetic.x + 160, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 320, topLeftPositionKinetic.y + 25 };
	bottomText.amPos = { topLeftPositionAccumulate.x + 160, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 320, topLeftPositionAccumulate.y + 25 };
	bottomText.cssmPos = { topLeftPositionContinuous.x + 160, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 320, topLeftPositionContinuous.y + 25 };
	bottomText.Create( "Bottom", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, bottomText.ksmPos, parent, bottomText.ID );
	bottomText.fontType = "Normal";
	//
	verticalBinningText.ID = id++;
	verticalBinningText.ksmPos = { topLeftPositionKinetic.x + 320, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 480, topLeftPositionKinetic.y + 25 };
	verticalBinningText.amPos = { topLeftPositionAccumulate.x + 320, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 480, topLeftPositionAccumulate.y + 25 };
	verticalBinningText.cssmPos = { topLeftPositionContinuous.x + 320, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 480, topLeftPositionContinuous.y + 25 };
	verticalBinningText.Create( "V. Bin", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, verticalBinningText.ksmPos, parent, verticalBinningText.ID );
	verticalBinningText.fontType = "Normal";
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;
	topLeftPositionContinuous.y += 25;
	//
	topEdit.ID = id++;
	topEdit.ksmPos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 160, topLeftPositionKinetic.y + 25 };
	topEdit.amPos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 160, topLeftPositionAccumulate.y + 25 };
	topEdit.cssmPos = { topLeftPositionContinuous.x, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 160, topLeftPositionContinuous.y + 25 };
	topEdit.Create( WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_CENTER, topEdit.ksmPos, parent, topEdit.ID );
	topEdit.fontType = "Normal";
	topEdit.SetWindowTextA( "1" );
	//
	bottomEdit.ID = id++;
	bottomEdit.ksmPos = { topLeftPositionKinetic.x + 160, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 320, topLeftPositionKinetic.y + 25 };
	bottomEdit.amPos = { topLeftPositionAccumulate.x + 160, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 320, topLeftPositionAccumulate.y + 25 };
	bottomEdit.cssmPos = { topLeftPositionContinuous.x + 160, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 320, topLeftPositionContinuous.y + 25 };
	bottomEdit.Create( WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_CENTER, bottomEdit.ksmPos, parent, bottomEdit.ID );
	bottomEdit.fontType = "Normal";
	bottomEdit.SetWindowTextA( "10" );
	//
	verticalBinningEdit.ID = id++;
	verticalBinningEdit.ksmPos = { topLeftPositionKinetic.x + 320, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 480, topLeftPositionKinetic.y + 25 };
	verticalBinningEdit.amPos = { topLeftPositionAccumulate.x + 320, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 480, topLeftPositionAccumulate.y + 25 };
	verticalBinningEdit.cssmPos = { topLeftPositionContinuous.x + 320, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 480, topLeftPositionContinuous.y + 25 };
	verticalBinningEdit.Create( WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_CENTER, verticalBinningEdit.ksmPos, parent, verticalBinningEdit.ID );
	verticalBinningEdit.fontType = "Normal";
	verticalBinningEdit.SetWindowTextA( "1" );
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;
	topLeftPositionContinuous.y += 25;
	return false;
}

bool CameraImageDimensionsControl::drawBackgrounds(CameraWindow* camWin)
{
	// recolor the box, clearing last run
	CDC* hDC = camWin->GetDC();
	SelectObject(*hDC, GetStockObject(DC_BRUSH));
	SelectObject(*hDC, GetStockObject(DC_PEN));
	// dark green brush
	SetDCBrushColor(*hDC, RGB(0, 10, 0));
	// Set the Pen to White
	SetDCPenColor(*hDC, RGB(255, 255, 255));
	// Drawing a rectangle with the current Device Context
	/*
	for (int imageInc = 0; imageInc < eImageBackgroundAreas.size(); imageInc++)
	{
		// slightly larger than the image zone.
		Rectangle(*hDC, eImageBackgroundAreas[imageInc].left - 5, eImageBackgroundAreas[imageInc].top - 5, eImageBackgroundAreas[imageInc].right + 5, eImageBackgroundAreas[imageInc].bottom + 5);
	}
	*/
	camWin->ReleaseDC(hDC);
	return true;
}


imageParameters CameraImageDimensionsControl::readImageParameters(CameraWindow* camWin)
{
	this->drawBackgrounds(camWin);
	// If new dimensions are set, we don't have data for the new dimensions.
	// eDataExists = false;
	// set all of the image parameters
	CString tempStr;
	leftEdit.GetWindowTextA(tempStr);
	try
	{
		currentImageParameters.leftBorder = std::stoi(std::string(tempStr));
	}
	catch (std::invalid_argument &)
	{
		isReady = false;
		thrower( "Left border argument not an integer!\r\n" );
		return currentImageParameters;
	}
	leftEdit.RedrawWindow();
	rightEdit.GetWindowTextA(tempStr);
	try
	{
		currentImageParameters.rightBorder = std::stoi(std::string(tempStr));
	}
	catch (std::invalid_argument &)
	{
		isReady = false;
		thrower("Right border argument not an integer!\r\n");
		return currentImageParameters;
	}
	rightEdit.RedrawWindow();
	//
	topEdit.GetWindowTextA(tempStr);
	try
	{
		currentImageParameters.topBorder = std::stoi(std::string(tempStr));
	}
	catch (std::invalid_argument &)
	{
		isReady = false;
		thrower("Top border argument not an integer!\r\n");
		return currentImageParameters;
	}
	topEdit.RedrawWindow();
	//
	bottomEdit.GetWindowTextA(tempStr);
	try
	{
		currentImageParameters.bottomBorder = std::stoi(std::string(tempStr));
	}
	catch (std::invalid_argument &)
	{
		isReady = false;
		thrower("Bottom border argument not an integer!\r\n");
		return currentImageParameters;
	}
	bottomEdit.RedrawWindow();
	horizontalBinningEdit.GetWindowTextA(tempStr);
	try
	{
		currentImageParameters.horizontalBinning = std::stoi(std::string(tempStr));
	}
	catch (std::invalid_argument &)
	{
		isReady = false;
		thrower("Horizontal binning argument not an integer!\r\n");
		return currentImageParameters;
	}
	horizontalBinningEdit.RedrawWindow();
	verticalBinningEdit.GetWindowTextA(tempStr);
	try
	{
		currentImageParameters.verticalBinning = std::stoi(std::string(tempStr));
	}
	catch (std::invalid_argument &)
	{
		isReady = false;
		thrower("Vertical binning argument not an integer!\r\n");
		return currentImageParameters;
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
		thrower("ERROR: Image start positions must not be greater than end positions\r\n");
		return currentImageParameters;
	}
	if (currentImageParameters.leftBorder < 1 || currentImageParameters.rightBorder > 512)
	{
		isReady = false;
		thrower("ERROR: Image horizontal borders must be greater than 0 and less than the detector width\r\n");
		return currentImageParameters;
	}
	if (currentImageParameters.topBorder < 1 || currentImageParameters.bottomBorder > 512)
	{
		isReady = false;
		thrower("ERROR: Image verttical borders must be greater than 0 and less than the detector height\r\n");
		return currentImageParameters;
	}
	if ((currentImageParameters.rightBorder - currentImageParameters.leftBorder + 1) % currentImageParameters.horizontalBinning != 0)
	{
		isReady = false;
		thrower("ERROR: Image width must be a multiple of Horizontal Binning\r\n");
		return currentImageParameters;
	}
	if ((currentImageParameters.bottomBorder - currentImageParameters.topBorder + 1) % currentImageParameters.verticalBinning != 0)
	{
		isReady = false;
		thrower("ERROR: Image height must be a multiple of Vertical Binning\r\n");
		return currentImageParameters;
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
bool CameraImageDimensionsControl::setImageParametersFromInput(imageParameters param, CameraWindow* camWin)
{

	this->drawBackgrounds(camWin);
	//eDataExists = false;
	// set all of the image parameters
	currentImageParameters.leftBorder = param.leftBorder;
	leftEdit.SetWindowText(std::to_string(currentImageParameters.leftBorder).c_str());
	currentImageParameters.rightBorder = param.rightBorder;
	rightEdit.SetWindowText(std::to_string(currentImageParameters.rightBorder).c_str());
	currentImageParameters.topBorder = param.topBorder;
	topEdit.SetWindowText(std::to_string(currentImageParameters.topBorder).c_str());	
	currentImageParameters.bottomBorder = param.bottomBorder;
	bottomEdit.SetWindowText(std::to_string(currentImageParameters.bottomBorder).c_str());
	currentImageParameters.horizontalBinning = param.horizontalBinning;
	horizontalBinningEdit.SetWindowText(std::to_string(currentImageParameters.horizontalBinning).c_str());
	currentImageParameters.verticalBinning = param.verticalBinning;
	verticalBinningEdit.SetWindowText(std::to_string(currentImageParameters.verticalBinning).c_str());
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
		thrower("ERROR: Image start positions must not be greater than end positions\r\n");
		return true;
	}
	if (currentImageParameters.leftBorder < 1 || currentImageParameters.rightBorder > 512)
	{
		isReady = false;
		thrower("ERROR: Image horizontal borders must be greater than 0 and less than the detector width\r\n");
		return true;
	}
	if (currentImageParameters.topBorder < 1 || currentImageParameters.bottomBorder > 512)
	{
		isReady = false;
		thrower("ERROR: Image verttical borders must be greater than 0 and less than the detector height\r\n");
		return true;
	}
	if ((currentImageParameters.rightBorder - currentImageParameters.leftBorder + 1) % currentImageParameters.horizontalBinning != 0)
	{
		isReady = false;
		thrower("ERROR: Image width must be a multiple of Horizontal Binning\r\n");
		return true;
	}
	if ((currentImageParameters.bottomBorder - currentImageParameters.topBorder + 1) % currentImageParameters.verticalBinning != 0)
	{
		isReady = false;
		thrower("ERROR: Image height must be a multiple of Vertical Binning\r\n");
		return true;
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
	return false;
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

CBrush* CameraImageDimensionsControl::colorEdits( int id, CDC* colorer, std::unordered_map<std::string, CBrush*> brushes,
												 std::unordered_map<std::string, COLORREF> rgbs )
{
	//imageParameters currentImageParameters = { 0,0,0,0,0,0 };
	if (id == bottomEdit.ID)
	{
		// imageParameters currentImageParameters = eImageControl.getImageParameters();
		colorer->SetTextColor( rgbs["White"] );
		CString textEdit;
		bottomEdit.GetWindowTextA(textEdit);
		int bottom;
		try
		{
			bottom = std::stoi(std::string(textEdit));
			if (bottom == currentImageParameters.bottomBorder)
			{
				// good.
				colorer->SetBkColor( rgbs["Grey Green"] );
				// catch change of color and redraw window.
				if (bottomEdit.colorState != 0)
				{
					bottomEdit.colorState = 0;
					bottomEdit.RedrawWindow();
				}
				return brushes["Grey Green"];
			}
		}
		catch (std::exception&)
		{
			// don't do anything with it.
		}
		colorer->SetBkColor( rgbs["Red"] );
		// catch change of color and redraw window.
		if (bottomEdit.colorState != 1)
		{
			bottomEdit.colorState = 1; 
			bottomEdit.RedrawWindow();
		}
		return brushes["Red"];
	}
	else if (id == topEdit.ID)
	{
		//imageParameters currentImageParameters = eImageControl.getImageParameters();
		colorer->SetTextColor( rgbs["White"] );
		CString textEdit;
		topEdit.GetWindowTextA(textEdit);
		int top;
		try
		{
			top = std::stoi(std::string(textEdit));
			if (top == currentImageParameters.topBorder)
			{
				// good.
				colorer->SetBkColor( rgbs["Grey Green"] );
				// catch change of color and redraw window.
				if (topEdit.colorState != 0)
				{
					topEdit.colorState = 0;
					topEdit.RedrawWindow();
				}
				return brushes["Grey Green"];
			}
		}
		catch (std::exception&)
		{
			// don't do anything with it.
		}
		colorer->SetBkColor( rgbs["Red"] );
		// catch change of color and redraw window.
		if (topEdit.colorState != 1)
		{
			topEdit.colorState = 1;
			topEdit.RedrawWindow();
		}
		return brushes["Red"];
	}
	else if (id == verticalBinningEdit.ID)
	{
		//imageParameters currentImageParameters = eImageControl.getImageParameters();
		colorer->SetTextColor( rgbs["White"] );
		CString textEdit;
		verticalBinningEdit.GetWindowTextA(textEdit);
		int verticalBin;
		try
		{
			verticalBin = std::stoi(std::string(textEdit));
			if (verticalBin == currentImageParameters.verticalBinning)
			{
				// good.
				colorer->SetBkColor( rgbs["Grey Green"] );
				// catch change of color and redraw window.
				if (verticalBinningEdit.colorState != 0)
				{
					verticalBinningEdit.colorState = 0;
					verticalBinningEdit.RedrawWindow();
				}
				return brushes["Grey Green"];
			}
		}
		catch (std::exception&)
		{
			// don't do anything with it.
		}
		colorer->SetBkColor( rgbs["Red"] );
		// catch change of color and redraw window.
		if (verticalBinningEdit.colorState != 1)
		{
			verticalBinningEdit.colorState = 1;
			verticalBinningEdit.RedrawWindow();
		}
		return brushes["Red"];
	}
	else if (id == leftEdit.ID)
	{
		//imageParameters currentImageParameters = eImageControl.getImageParameters();
		colorer->SetTextColor( rgbs["White"] );
		CString textEdit;
		leftEdit.GetWindowTextA(textEdit);
		int leftSide;
		try
		{
			leftSide = std::stoi(std::string(textEdit));
			if (leftSide == currentImageParameters.leftBorder)
			{
				// good.
				colorer->SetBkColor( rgbs["Grey Green"] );
				// catch change of color and redraw window.
				if (leftEdit.colorState != 0)
				{
					leftEdit.colorState = 0;
					leftEdit.RedrawWindow();
				}
				return brushes["Grey Green"];
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
		colorer->SetBkColor( rgbs["Red"] );
		// catch change of color and redraw window.
		if (leftEdit.colorState != 1)
		{
			leftEdit.colorState = 1;
			leftEdit.RedrawWindow();
		}
		return brushes["Red"];
	}
	else if (id == rightEdit.ID)
	{
		//imageParameters currentImageParameters = eImageControl.getImageParameters();
		colorer->SetTextColor( rgbs["White"] );
		CString textEdit;
		rightEdit.GetWindowTextA(textEdit);
		int rightSide;
		try
		{
			rightSide = std::stoi(std::string(textEdit));
			if (rightSide == currentImageParameters.rightBorder)
			{
				// good.
				colorer->SetBkColor( rgbs["Grey Green"] );
				// catch change of color and redraw window.
				if (rightEdit.colorState != 0)
				{
					rightEdit.colorState = 0;
					rightEdit.RedrawWindow();
				}
				return brushes["Grey Green"];
			}
		}
		catch (std::exception&)
		{
			// don't do anything with it.
		}
		colorer->SetBkColor( rgbs["Red"] );
		// catch change of color and redraw window.
		if (rightEdit.colorState != 1)
		{
			rightEdit.colorState = 1;
			rightEdit.RedrawWindow();
		}
		return brushes["Red"];
	}
	else if (id == horizontalBinningEdit.ID)
	{
		//imageParameters currentImageParameters = eImageControl.getImageParameters();
		colorer->SetTextColor( rgbs["White"] );
		CString textEdit;
		horizontalBinningEdit.GetWindowTextA(textEdit);
		int horizontalBin;
		try
		{
			horizontalBin = std::stoi(std::string(textEdit));
			if (horizontalBin == currentImageParameters.horizontalBinning)
			{
				// good.
				colorer->SetBkColor( rgbs["Grey Green"] );
				// catch change of color and redraw window.
				if (horizontalBinningEdit.colorState != 0)
				{
					horizontalBinningEdit.colorState = 0;
					horizontalBinningEdit.RedrawWindow();
				}
				return brushes["Grey Green"];
			}
		}
		catch (std::exception&)
		{
			// don't do anything with it.
		}
		colorer->SetBkColor( rgbs["Red"] );
		// catch change of color and redraw window.
		if (horizontalBinningEdit.colorState != 1)
		{
			horizontalBinningEdit.colorState = 1;
			horizontalBinningEdit.RedrawWindow();
		}
		return brushes["Red"];
	}
	// control not in this obj
	return NULL;
}

bool CameraImageDimensionsControl::rearrange(std::string cameraMode, std::string triggerMode, int width, 
	int height, std::unordered_map<std::string, CFont*> fonts)
{
	leftText.rearrange(cameraMode, triggerMode, width, height, fonts);
	rightText.rearrange(cameraMode, triggerMode, width, height, fonts);
	horizontalBinningText.rearrange(cameraMode, triggerMode, width, height, fonts);
	topText.rearrange(cameraMode, triggerMode, width, height, fonts);
	bottomText.rearrange(cameraMode, triggerMode, width, height, fonts);
	verticalBinningText.rearrange(cameraMode, triggerMode, width, height, fonts);
	leftEdit.rearrange(cameraMode, triggerMode, width, height, fonts);
	rightEdit.rearrange(cameraMode, triggerMode, width, height, fonts);
	horizontalBinningEdit.rearrange(cameraMode, triggerMode, width, height, fonts);
	topEdit.rearrange(cameraMode, triggerMode, width, height, fonts);
	bottomEdit.rearrange(cameraMode, triggerMode, width, height, fonts);
	verticalBinningEdit.rearrange(cameraMode, triggerMode, width, height, fonts);
	setImageDimensionsButton.rearrange(cameraMode, triggerMode, width, height, fonts);
	return false;
}

