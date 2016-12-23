/// This file contains all functiosn for the CameraImageParameters singleton class.

#include "stdafx.h"
#include "CameraImageParameters.h"
#include "externals.h"
#include "constants.h"
#include "appendText.h"
#include "reorganizeControl.h"

CameraImageParametersControl::CameraImageParametersControl()
{
	isReady = false;
}

bool CameraImageParametersControl::initiateControls(POINT& topLeftPositionKinetic, POINT& topLeftPositionAccumulate, POINT& topLeftPositionContinuous,
	HWND parentWindow, bool isTriggerModeSensitive)
{
	setImageParametersButton.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 272, topLeftPositionKinetic.y + 25 };
	setImageParametersButton.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 272, topLeftPositionAccumulate.y + 25 };
	setImageParametersButton.continuousSingleScansModePos = { topLeftPositionContinuous.x, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 272, topLeftPositionContinuous.y + 25 };
	RECT initPos = setImageParametersButton.kineticSeriesModePos;
	setImageParametersButton.hwnd = CreateWindowEx(0, "BUTTON", "Set Image Parameters", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_SET_IMAGE_PARAMS_BUTTON, eHInst, NULL);
	setImageParametersButton.fontType = "Normal";
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;
	topLeftPositionContinuous.y += 25;
	//
	leftText.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 91, topLeftPositionKinetic.y + 25 };
	leftText.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 91, topLeftPositionAccumulate.y + 25 };
	leftText.continuousSingleScansModePos = { topLeftPositionContinuous.x, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 91, topLeftPositionContinuous.y + 25 };
	initPos = leftText.kineticSeriesModePos;
	leftText.hwnd = CreateWindowEx(0, "STATIC", "Left", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
	leftText.fontType = "Normal";
	//
	rightText.kineticSeriesModePos = { topLeftPositionKinetic.x + 91, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 182, topLeftPositionKinetic.y + 25 };
	rightText.accumulateModePos = { topLeftPositionAccumulate.x + 91, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 182, topLeftPositionAccumulate.y + 25 };
	rightText.continuousSingleScansModePos = { topLeftPositionContinuous.x + 91, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 182, topLeftPositionContinuous.y + 25 };
	initPos = rightText.kineticSeriesModePos;
	rightText.hwnd = CreateWindowEx(0, "STATIC", "Right", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
	rightText.fontType = "Normal";
	//
	horizontalBinningText.kineticSeriesModePos = { topLeftPositionKinetic.x + 182, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 270, topLeftPositionKinetic.y + 25 };
	horizontalBinningText.accumulateModePos = { topLeftPositionAccumulate.x + 182, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 270, topLeftPositionAccumulate.y + 25 };
	horizontalBinningText.continuousSingleScansModePos = { topLeftPositionContinuous.x + 182, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 270, topLeftPositionContinuous.y + 25 };
	initPos = horizontalBinningText.kineticSeriesModePos;
	horizontalBinningText.hwnd = CreateWindowEx(0, "STATIC", "H. Bin", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
	horizontalBinningText.fontType = "Normal";
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;
	topLeftPositionContinuous.y += 25;
	//
	leftEdit.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 91, topLeftPositionKinetic.y + 25 };
	leftEdit.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 91, topLeftPositionAccumulate.y + 25 };
	leftEdit.continuousSingleScansModePos = { topLeftPositionContinuous.x, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 91, topLeftPositionContinuous.y + 25 };
	initPos = leftEdit.kineticSeriesModePos;
	leftEdit.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_CENTER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_IMG_LEFT_EDIT, eHInst, NULL);
	leftEdit.fontType = "Normal";
	//
	rightEdit.kineticSeriesModePos = { topLeftPositionKinetic.x + 91, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 182, topLeftPositionKinetic.y + 25 };
	rightEdit.accumulateModePos = { topLeftPositionAccumulate.x + 91, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 182, topLeftPositionAccumulate.y + 25 };
	rightEdit.continuousSingleScansModePos = { topLeftPositionContinuous.x + 91, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 182, topLeftPositionContinuous.y + 25 };
	initPos = rightEdit.kineticSeriesModePos;
	rightEdit.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_CENTER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_IMG_RIGHT_EDIT, eHInst, NULL);
	rightEdit.fontType = "Normal";
	//
	horizontalBinningEdit.kineticSeriesModePos = { topLeftPositionKinetic.x + 182, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 270, topLeftPositionKinetic.y + 25 };
	horizontalBinningEdit.accumulateModePos = { topLeftPositionAccumulate.x + 182, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 270, topLeftPositionAccumulate.y + 25 };
	horizontalBinningEdit.continuousSingleScansModePos = { topLeftPositionContinuous.x + 182, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 270, topLeftPositionContinuous.y + 25 };
	initPos = horizontalBinningEdit.kineticSeriesModePos;
	horizontalBinningEdit.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_CENTER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_HOR_BIN_EDIT, eHInst, NULL);
	horizontalBinningEdit.fontType = "Normal";
	//
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;
	topLeftPositionContinuous.y += 25;
	leftDisp.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 91, topLeftPositionKinetic.y + 25 };
	leftDisp.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 91, topLeftPositionAccumulate.y + 25 };
	leftDisp.continuousSingleScansModePos = { topLeftPositionContinuous.x, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 91, topLeftPositionContinuous.y + 25 };
	initPos = leftDisp.kineticSeriesModePos;
	leftDisp.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_IMG_LEFT_DISP, eHInst, NULL);
	leftDisp.fontType = "Normal";
	//
	rightDisp.kineticSeriesModePos = { topLeftPositionKinetic.x + 91, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 182, topLeftPositionKinetic.y + 25 };
	rightDisp.accumulateModePos = { topLeftPositionAccumulate.x + 91, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 182, topLeftPositionAccumulate.y + 25 };
	rightDisp.continuousSingleScansModePos = { topLeftPositionContinuous.x + 91, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 182, topLeftPositionContinuous.y + 25 };
	initPos = rightDisp.kineticSeriesModePos;
	rightDisp.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_IMG_RIGHT_DISP, eHInst, NULL);
	rightDisp.fontType = "Normal";
	//
	horizontalBinningDisp.kineticSeriesModePos = { topLeftPositionKinetic.x + 182, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 270, topLeftPositionKinetic.y + 25 };
	horizontalBinningDisp.accumulateModePos = { topLeftPositionAccumulate.x + 182, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 270, topLeftPositionAccumulate.y + 25 };
	horizontalBinningDisp.continuousSingleScansModePos = { topLeftPositionContinuous.x + 182, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 270, topLeftPositionContinuous.y + 25 };
	initPos = horizontalBinningDisp.kineticSeriesModePos;
	horizontalBinningDisp.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_HOR_BIN_DISP, eHInst, NULL);
	horizontalBinningDisp.fontType = "Normal";
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;
	topLeftPositionContinuous.y += 25;
	//
	topText.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 91, topLeftPositionKinetic.y + 25 };
	topText.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 91, topLeftPositionAccumulate.y + 25 };
	topText.continuousSingleScansModePos = { topLeftPositionContinuous.x, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 91, topLeftPositionContinuous.y + 25 };
	initPos = topText.kineticSeriesModePos;
	topText.hwnd = CreateWindowEx(0, "STATIC", "Top", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
	topText.fontType = "Normal";
	//
	bottomText.kineticSeriesModePos = { topLeftPositionKinetic.x + 91, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 182, topLeftPositionKinetic.y + 25 };
	bottomText.accumulateModePos = { topLeftPositionAccumulate.x + 91, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 182, topLeftPositionAccumulate.y + 25 };
	bottomText.continuousSingleScansModePos = { topLeftPositionContinuous.x + 91, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 182, topLeftPositionContinuous.y + 25 };
	initPos = bottomText.kineticSeriesModePos;
	bottomText.hwnd = CreateWindowEx(0, "STATIC", "Bottom", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
	bottomText.fontType = "Normal";
	//
	verticalBinningText.kineticSeriesModePos = { topLeftPositionKinetic.x + 182, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 270, topLeftPositionKinetic.y + 25 };
	verticalBinningText.accumulateModePos = { topLeftPositionAccumulate.x + 182, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 270, topLeftPositionAccumulate.y + 25 };
	verticalBinningText.continuousSingleScansModePos = { topLeftPositionContinuous.x + 182, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 270, topLeftPositionContinuous.y + 25 };
	initPos = verticalBinningText.kineticSeriesModePos;
	verticalBinningText.hwnd = CreateWindowEx(0, "STATIC", "V. Bin", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
	verticalBinningText.fontType = "Normal";
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;
	topLeftPositionContinuous.y += 25;
	//
	topEdit.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 91, topLeftPositionKinetic.y + 25 };
	topEdit.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 91, topLeftPositionAccumulate.y + 25 };
	topEdit.continuousSingleScansModePos = { topLeftPositionContinuous.x, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 91, topLeftPositionContinuous.y + 25 };
	initPos = topEdit.kineticSeriesModePos;
	topEdit.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_CENTER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_IMAGE_TOP_EDIT, eHInst, NULL);
	topEdit.fontType = "Normal";
	//
	bottomEdit.kineticSeriesModePos = { topLeftPositionKinetic.x + 91, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 182, topLeftPositionKinetic.y + 25 };
	bottomEdit.accumulateModePos = { topLeftPositionAccumulate.x + 91, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 182, topLeftPositionAccumulate.y + 25 };
	bottomEdit.continuousSingleScansModePos = { topLeftPositionContinuous.x + 91, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 182, topLeftPositionContinuous.y + 25 };
	initPos = bottomEdit.kineticSeriesModePos;
	bottomEdit.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_CENTER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_IMAGE_BOTTOM_EDIT, eHInst, NULL);
	bottomEdit.fontType = "Normal";
	//
	verticalBinningEdit.kineticSeriesModePos = { topLeftPositionKinetic.x + 182, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 270, topLeftPositionKinetic.y + 25 };
	verticalBinningEdit.accumulateModePos = { topLeftPositionAccumulate.x + 182, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 270, topLeftPositionAccumulate.y + 25 };
	verticalBinningEdit.continuousSingleScansModePos = { topLeftPositionContinuous.x + 182, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 270, topLeftPositionContinuous.y + 25 };
	initPos = verticalBinningEdit.kineticSeriesModePos;
	verticalBinningEdit.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_CENTER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_VERTICAL_BIN_EDIT, eHInst, NULL);
	verticalBinningEdit.fontType = "Normal";
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;
	topLeftPositionContinuous.y += 25;
	topDisp.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 91, topLeftPositionKinetic.y + 25 };
	topDisp.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 91, topLeftPositionAccumulate.y + 25 };
	topDisp.continuousSingleScansModePos = { topLeftPositionContinuous.x, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 91, topLeftPositionContinuous.y + 25 };
	initPos = topDisp.kineticSeriesModePos;
	topDisp.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_IMG_TOP_DISP, eHInst, NULL);
	topDisp.fontType = "Normal";
	//
	bottomDisp.kineticSeriesModePos = { topLeftPositionKinetic.x + 91, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 182, topLeftPositionKinetic.y + 25 };
	bottomDisp.accumulateModePos = { topLeftPositionAccumulate.x + 91, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 182, topLeftPositionAccumulate.y + 25 };
	bottomDisp.continuousSingleScansModePos = { topLeftPositionContinuous.x + 91, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 182, topLeftPositionContinuous.y + 25 };
	initPos = bottomDisp.kineticSeriesModePos;
	bottomDisp.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_IMG_BOTTOM_DISP, eHInst, NULL);
	bottomDisp.fontType = "Normal";
	//
	verticalBinningDisp.kineticSeriesModePos = { topLeftPositionKinetic.x + 182, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 270, topLeftPositionKinetic.y + 25 };
	verticalBinningDisp.accumulateModePos = { topLeftPositionAccumulate.x + 182, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 270, topLeftPositionAccumulate.y + 25 };
	verticalBinningDisp.continuousSingleScansModePos = { topLeftPositionContinuous.x + 182, topLeftPositionContinuous.y, topLeftPositionContinuous.x + 270, topLeftPositionContinuous.y + 25 };
	initPos = verticalBinningDisp.kineticSeriesModePos;
	verticalBinningDisp.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_VERTICAL_BIN_DISP, eHInst, NULL);
	verticalBinningDisp.fontType = "Normal";
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;
	topLeftPositionContinuous.y += 25;
	return false;
}

bool CameraImageParametersControl::drawBackgrounds()
{
	// recolor the box, clearing last run
	HDC hDC = GetDC(eCameraWindowHandle);
	SelectObject(hDC, GetStockObject(DC_BRUSH));
	SelectObject(hDC, GetStockObject(DC_PEN));
	// dark green brush
	SetDCBrushColor(hDC, RGB(0, 10, 0));
	// Set the Pen to White
	SetDCPenColor(hDC, RGB(255, 255, 255));
	// Drawing a rectangle with the current Device Context
	for (int imageInc = 0; imageInc < eImageBackgroundAreas.size(); imageInc++)
	{
		// slightly larger than the image zone.
		Rectangle(hDC, eImageBackgroundAreas[imageInc].left - 5, eImageBackgroundAreas[imageInc].top - 5, eImageBackgroundAreas[imageInc].right + 5, eImageBackgroundAreas[imageInc].bottom + 5);
	}
	ReleaseDC(eCameraWindowHandle, hDC);
	return true;
}
bool CameraImageParametersControl::drawRectangleFrame()
{
	// draw rectangles indicating where the pixels are.
	HDC hDC2 = GetDC(eCameraWindowHandle);
	for (int pictureInc = 0; pictureInc < ePixelRectangles.size(); pictureInc++)
	{
		for (int widthInc = 0; widthInc < ePixelRectangles[pictureInc].size(); widthInc++)
		{
			for (int heightInc = 0; heightInc < ePixelRectangles[pictureInc][widthInc].size(); heightInc++)
			{
				FrameRect(hDC2, &ePixelRectangles[pictureInc][widthInc][heightInc], eWhiteBrush);
			}
		}
	}
	return true;
}

bool CameraImageParametersControl::setImageParameters()
{
	this->drawBackgrounds();
	// If new dimensions are set, we don't have data for those.
	eDataExists = false;
	// set all of the image parameters
	std::string tempStr;
	GetWindowText(leftEdit.hwnd, (LPSTR)tempStr.c_str(), 4);
	try
	{
		currentImageParameters.leftBorder = std::stoi(tempStr);
	}
	catch (std::invalid_argument &exception)
	{
		appendText("Left border argument not an integer!", IDC_ERROR_EDIT);
		isReady = false;
		return true;
	}
	RedrawWindow(leftEdit.hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
	SendMessage(leftDisp.hwnd, WM_SETTEXT, 0, (LPARAM)tempStr.c_str());

	GetWindowText(rightEdit.hwnd, (LPSTR)tempStr.c_str(), 4);
	try
	{
		currentImageParameters.rightBorder = std::stoi(tempStr);
	}
	catch (std::invalid_argument &exception)
	{
		appendText("Right border argument not an integer!", IDC_ERROR_EDIT);
		isReady = false;
		return true;
	}
	RedrawWindow(rightEdit.hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
	SendMessage(rightDisp.hwnd, WM_SETTEXT, 0, (LPARAM)tempStr.c_str());

	GetWindowText(topEdit.hwnd, (LPSTR)tempStr.c_str(), 4);
	try
	{
		currentImageParameters.topBorder = std::stoi(tempStr);
	}
	catch (std::invalid_argument &exception)
	{
		appendText("Top border argument not an integer!", IDC_ERROR_EDIT);
		isReady = false;
		return true;
	}
	RedrawWindow(topEdit.hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
	SendMessage(topDisp.hwnd, WM_SETTEXT, 0, (LPARAM)tempStr.c_str());
	//
	GetWindowText(bottomEdit.hwnd, (LPSTR)tempStr.c_str(), 4);
	try
	{
		currentImageParameters.bottomBorder = std::stoi(tempStr);
	}
	catch (std::invalid_argument &exception)
	{
		appendText("Bottom border argument not an integer!", IDC_ERROR_EDIT);
		isReady = false;
		return true;
	}
	RedrawWindow(bottomEdit.hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
	SendMessage(bottomDisp.hwnd, WM_SETTEXT, 0, (LPARAM)tempStr.c_str());

	GetWindowText(horizontalBinningEdit.hwnd, (LPSTR)tempStr.c_str(), 4);
	try
	{
		currentImageParameters.horizontalBinning = std::stoi(tempStr);
	}
	catch (std::invalid_argument &exception)
	{
		appendText("Horizontal binning argument not an integer!", IDC_ERROR_EDIT);
		isReady = false;
		return true;
	}
	RedrawWindow(horizontalBinningEdit.hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
	SendMessage(horizontalBinningDisp.hwnd, WM_SETTEXT, 0, (LPARAM)tempStr.c_str());

	GetWindowText(verticalBinningEdit.hwnd, (LPSTR)tempStr.c_str(), 4);
	try
	{
		currentImageParameters.verticalBinning = std::stoi(tempStr);
	}
	catch (std::invalid_argument &exception)
	{
		appendText("Vertical binning argument not an integer!", IDC_ERROR_EDIT);
		isReady = false;
		return true;
	}
	RedrawWindow(verticalBinningEdit.hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
	SendMessage(verticalBinningDisp.hwnd, WM_SETTEXT, 0, (LPARAM)tempStr.c_str());

	// reset this. There must be at least one pixel...
	eCurrentlySelectedPixel.first = 0;
	eCurrentlySelectedPixel.second = 0;
	// Calculate the number of actual pixels in each dimension.
	currentImageParameters.width = (currentImageParameters.rightBorder - currentImageParameters.leftBorder + 1) / currentImageParameters.horizontalBinning;
	currentImageParameters.height = (currentImageParameters.bottomBorder - currentImageParameters.topBorder + 1) / currentImageParameters.verticalBinning;

	// Check Image parameters
	if (currentImageParameters.leftBorder > currentImageParameters.rightBorder || currentImageParameters.topBorder > currentImageParameters.bottomBorder)
	{
		appendText("ERROR: Image start positions must not be greater than end positions\r\n", IDC_ERROR_EDIT);
		isReady = false;
		return true;
	}
	if (currentImageParameters.leftBorder < 1 || currentImageParameters.rightBorder > eXPixels)
	{
		appendText("ERROR: Image horizontal borders must be greater than 0 and less than the detector width\r\n", IDC_ERROR_EDIT);
		isReady = false;
		return true;
	}
	if (currentImageParameters.topBorder < 1 || currentImageParameters.bottomBorder > eYPixels)
	{
		appendText("ERROR: Image verttical borders must be greater than 0 and less than the detector height\r\n", IDC_ERROR_EDIT);
		isReady = false;
		return true;
	}
	if ((currentImageParameters.rightBorder - currentImageParameters.leftBorder + 1) % currentImageParameters.horizontalBinning != 0)
	{
		appendText("ERROR: Image width must be a multiple of Horizontal Binning\r\n", IDC_ERROR_EDIT);
		isReady = false;
		return true;
	}
	if ((currentImageParameters.bottomBorder - currentImageParameters.topBorder + 1) % currentImageParameters.verticalBinning != 0)
	{
		appendText("ERROR: Image height must be a multiple of Vertical Binning\r\n", IDC_ERROR_EDIT);
		isReady = false;
		return true;
	}
	// made it through successfully.

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
	this->drawRectangleFrame();
	eCameraFileSystem.updateSaveStatus(false);
	isReady = true;
	return false;
}


/*
 * I forget why I needed a second function for this.
 */
bool CameraImageParametersControl::setImageParametersFromInput(imageParameters param)
{
	this->drawBackgrounds();
	eDataExists = false;
	// set all of the image parameters
	currentImageParameters.leftBorder = param.leftBorder;
	SendMessage(leftEdit.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(currentImageParameters.leftBorder).c_str());
	SendMessage(leftDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(currentImageParameters.leftBorder).c_str());

	currentImageParameters.rightBorder = param.rightBorder;
	SendMessage(rightEdit.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(currentImageParameters.rightBorder).c_str());
	SendMessage(rightDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(currentImageParameters.rightBorder).c_str());

	currentImageParameters.topBorder = param.topBorder;
	SendMessage(topEdit.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(currentImageParameters.topBorder).c_str());
	SendMessage(topDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(currentImageParameters.topBorder).c_str());
	
	currentImageParameters.bottomBorder = param.bottomBorder;
	SendMessage(bottomEdit.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(currentImageParameters.bottomBorder).c_str());
	SendMessage(bottomDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(currentImageParameters.bottomBorder).c_str());

	currentImageParameters.horizontalBinning = param.horizontalBinning;
	SendMessage(horizontalBinningEdit.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(currentImageParameters.horizontalBinning).c_str());
	SendMessage(horizontalBinningDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(currentImageParameters.horizontalBinning).c_str());

	currentImageParameters.verticalBinning = param.verticalBinning;
	SendMessage(verticalBinningEdit.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(currentImageParameters.verticalBinning).c_str());
	SendMessage(verticalBinningDisp.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(currentImageParameters.verticalBinning).c_str());

	// reset this. There must be at least one pixel...
	eCurrentlySelectedPixel.first = 0;
	eCurrentlySelectedPixel.second = 0;
	// Calculate the number of actual pixels in each dimension.
	currentImageParameters.width = (currentImageParameters.rightBorder - currentImageParameters.leftBorder + 1) / currentImageParameters.horizontalBinning;
	currentImageParameters.height = (currentImageParameters.bottomBorder - currentImageParameters.topBorder + 1) / currentImageParameters.verticalBinning;

	// Check Image parameters
	if (currentImageParameters.leftBorder > currentImageParameters.rightBorder || currentImageParameters.topBorder > currentImageParameters.bottomBorder)
	{
		appendText("ERROR: Image start positions must not be greater than end positions\r\n", IDC_ERROR_EDIT);
		isReady = false;
		return true;
	}
	if (currentImageParameters.leftBorder < 1 || currentImageParameters.rightBorder > eXPixels)
	{
		appendText("ERROR: Image horizontal borders must be greater than 0 and less than the detector width\r\n", IDC_ERROR_EDIT);
		isReady = false;
		return true;
	}
	if (currentImageParameters.topBorder < 1 || currentImageParameters.bottomBorder > eYPixels)
	{
		appendText("ERROR: Image verttical borders must be greater than 0 and less than the detector height\r\n", IDC_ERROR_EDIT);
		isReady = false;
		return true;
	}
	if ((currentImageParameters.rightBorder - currentImageParameters.leftBorder + 1) % currentImageParameters.horizontalBinning != 0)
	{
		appendText("ERROR: Image width must be a multiple of Horizontal Binning\r\n", IDC_ERROR_EDIT);
		isReady = false;
		return true;
	}
	if ((currentImageParameters.bottomBorder - currentImageParameters.topBorder + 1) % currentImageParameters.verticalBinning != 0)
	{
		appendText("ERROR: Image height must be a multiple of Vertical Binning\r\n", IDC_ERROR_EDIT);
		isReady = false;
		return true;
	}
	// made it through successfully.

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

	eCameraFileSystem.updateSaveStatus(false);
	isReady = true;
	return false;
}

bool CameraImageParametersControl::checkReady()
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

imageParameters CameraImageParametersControl::getImageParameters()
{
	return currentImageParameters;
}

INT_PTR CameraImageParametersControl::colorEdits(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	DWORD controlID = GetDlgCtrlID((HWND)lParam);
	HDC hdcStatic = (HDC)wParam;
	switch (controlID)
	{
		case IDC_IMAGE_BOTTOM_EDIT:
		{
			imageParameters currentImageParameters = eImageControl.getImageParameters();
			SetTextColor(hdcStatic, RGB(255, 255, 255));
			TCHAR textEdit[256];
			SendMessage(bottomEdit.hwnd, WM_GETTEXT, 256, (LPARAM)textEdit);
			int bottom;
			try
			{
				bottom = std::stoi(std::string(textEdit));
				if (bottom == currentImageParameters.bottomBorder)
				{
					// good.
					SetTextColor(hdcStatic, RGB(255, 255, 255));
					SetBkColor(hdcStatic, RGB(100, 110, 100));
					// catch change of color and redraw window.
					if (bottomEdit.colorState != 0)
					{
						bottomEdit.colorState = 0;
						RedrawWindow(bottomEdit.hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
					}
					return (INT_PTR)eGreyGreenBrush;
					break;
				}
			}
			catch (std::exception&)
			{
				// don't do anything with it.
			}
			SetTextColor(hdcStatic, RGB(255, 255, 255));
			SetBkColor(hdcStatic, RGB(150, 100, 100));
			// catch change of color and redraw window.
			if (bottomEdit.colorState != 1)
			{
				bottomEdit.colorState = 1;
				RedrawWindow(bottomEdit.hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			return (INT_PTR)eGreyRedBrush;
			break;
		}
		case IDC_IMAGE_TOP_EDIT:
		{
			imageParameters currentImageParameters = eImageControl.getImageParameters();
			SetTextColor(hdcStatic, RGB(255, 255, 255));
			TCHAR textEdit[256];
			SendMessage(topEdit.hwnd, WM_GETTEXT, 256, (LPARAM)textEdit);
			int top;
			try
			{
				top = std::stoi(std::string(textEdit));
				if (top == currentImageParameters.topBorder)
				{
					// good.
					SetTextColor(hdcStatic, RGB(255, 255, 255));
					SetBkColor(hdcStatic, RGB(100, 110, 100));
					// catch change of color and redraw window.
					if (topEdit.colorState != 0)
					{
						topEdit.colorState = 0;
						RedrawWindow(topEdit.hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
					}
					return (INT_PTR)eGreyGreenBrush;
					break;
				}
			}
			catch (std::exception&)
			{
				// don't do anything with it.
			}
			SetTextColor(hdcStatic, RGB(255, 255, 255));
			SetBkColor(hdcStatic, RGB(150, 100, 100));
			// catch change of color and redraw window.
			if (topEdit.colorState != 1)
			{
				topEdit.colorState = 1;
				RedrawWindow(topEdit.hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			return (INT_PTR)eGreyRedBrush;
			break;
		}
		case IDC_VERTICAL_BIN_EDIT:
		{
			imageParameters currentImageParameters = eImageControl.getImageParameters();
			SetTextColor(hdcStatic, RGB(255, 255, 255));
			TCHAR textEdit[256];
			SendMessage(verticalBinningEdit.hwnd, WM_GETTEXT, 256, (LPARAM)textEdit);
			int verticalBin;
			try
			{
				verticalBin = std::stoi(std::string(textEdit));
				if (verticalBin == currentImageParameters.verticalBinning)
				{
					// good.
					SetTextColor(hdcStatic, RGB(255, 255, 255));
					SetBkColor(hdcStatic, RGB(100, 110, 100));
					// catch change of color and redraw window.
					if (verticalBinningEdit.colorState != 0)
					{
						verticalBinningEdit.colorState = 0;
						RedrawWindow(verticalBinningEdit.hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
					}
					return (INT_PTR)eGreyGreenBrush;
					break;
				}
			}
			catch (std::exception&)
			{
				// don't do anything with it.
			}
			SetTextColor(hdcStatic, RGB(255, 255, 255));
			SetBkColor(hdcStatic, RGB(150, 100, 100));
			// catch change of color and redraw window.
			if (verticalBinningEdit.colorState != 1)
			{
				verticalBinningEdit.colorState = 1;
				RedrawWindow(verticalBinningEdit.hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			return (INT_PTR)eGreyRedBrush;
			break;
		}

		case IDC_IMG_LEFT_EDIT:
		{
			imageParameters currentImageParameters = eImageControl.getImageParameters();
			SetTextColor(hdcStatic, RGB(255, 255, 255));
			TCHAR textEdit[256];
			SendMessage(leftEdit.hwnd, WM_GETTEXT, 256, (LPARAM)textEdit);
			int leftSide;
			try
			{
				leftSide = std::stoi(std::string(textEdit));
				if (leftSide == currentImageParameters.leftBorder)
				{
					// good.
					SetTextColor(hdcStatic, RGB(255, 255, 255));
					SetBkColor(hdcStatic, RGB(100, 110, 100));
					// catch change of color and redraw window.
					if (leftEdit.colorState != 0)
					{
						leftEdit.colorState = 0;
						RedrawWindow(leftEdit.hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
					}
					return (INT_PTR)eGreyGreenBrush;
					break;
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
			SetTextColor(hdcStatic, RGB(255, 255, 255));
			SetBkColor(hdcStatic, RGB(150, 100, 100));
			// catch change of color and redraw window.
			if (leftEdit.colorState != 1)
			{
				leftEdit.colorState = 1;
				RedrawWindow(leftEdit.hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			return (INT_PTR)eGreyRedBrush;
			break;
		}
		case IDC_IMG_RIGHT_EDIT:
		{
			imageParameters currentImageParameters = eImageControl.getImageParameters();
			SetTextColor(hdcStatic, RGB(255, 255, 255));
			TCHAR textEdit[256];
			SendMessage(rightEdit.hwnd, WM_GETTEXT, 256, (LPARAM)textEdit);
			int rightSide;
			try
			{
				rightSide = std::stoi(std::string(textEdit));
				if (rightSide == currentImageParameters.rightBorder)
				{
					// good.
					SetTextColor(hdcStatic, RGB(255, 255, 255));
					SetBkColor(hdcStatic, RGB(100, 110, 100));
					// catch change of color and redraw window.
					if (rightEdit.colorState != 0)
					{
						rightEdit.colorState = 0;
						RedrawWindow(rightEdit.hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
					}
					return (INT_PTR)eGreyGreenBrush;
					break;
				}
			}
			catch (std::exception&)
			{
				// don't do anything with it.
			}
			SetTextColor(hdcStatic, RGB(255, 255, 255));
			SetBkColor(hdcStatic, RGB(150, 100, 100));
			// catch change of color and redraw window.
			if (rightEdit.colorState != 1)
			{
				rightEdit.colorState = 1;
				RedrawWindow(rightEdit.hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			return (INT_PTR)eGreyRedBrush;
			break;
		}
		case IDC_HOR_BIN_EDIT:
		{
			imageParameters currentImageParameters = eImageControl.getImageParameters();
			SetTextColor(hdcStatic, RGB(255, 255, 255));
			TCHAR textEdit[256];
			SendMessage(horizontalBinningEdit.hwnd, WM_GETTEXT, 256, (LPARAM)textEdit);
			int horizontalBin;
			try
			{
				horizontalBin = std::stoi(std::string(textEdit));
				if (horizontalBin == currentImageParameters.horizontalBinning)
				{
					// good.
					SetTextColor(hdcStatic, RGB(255, 255, 255));
					SetBkColor(hdcStatic, RGB(100, 110, 100));
					// catch change of color and redraw window.
					if (horizontalBinningEdit.colorState != 0)
					{
						horizontalBinningEdit.colorState = 0;
						RedrawWindow(horizontalBinningEdit.hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
					}
					return (INT_PTR)eGreyGreenBrush;
					break;
				}
			}
			catch (std::exception&)
			{
				// don't do anything with it.
			}
			SetTextColor(hdcStatic, RGB(255, 255, 255));
			SetBkColor(hdcStatic, RGB(150, 100, 100));
			// catch change of color and redraw window.
			if (horizontalBinningEdit.colorState != 1)
			{
				horizontalBinningEdit.colorState = 1;
				RedrawWindow(horizontalBinningEdit.hwnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			return (INT_PTR)eGreyRedBrush;
			break;
		}
		return DefWindowProc(window, message, wParam, lParam);
	}
	return DefWindowProc(window, message, wParam, lParam);
}


bool CameraImageParametersControl::reorganizeControls(RECT parentRectangle, std::string cameraMode)
{
	reorganizeControl(leftText, cameraMode, parentRectangle);
	reorganizeControl(rightText, cameraMode, parentRectangle);
	reorganizeControl(horizontalBinningText, cameraMode, parentRectangle);
	reorganizeControl(topText, cameraMode, parentRectangle);
	reorganizeControl(bottomText, cameraMode, parentRectangle);
	reorganizeControl(verticalBinningText, cameraMode, parentRectangle);
	reorganizeControl(leftEdit, cameraMode, parentRectangle);
	reorganizeControl(rightEdit, cameraMode, parentRectangle);
	reorganizeControl(horizontalBinningEdit, cameraMode, parentRectangle);
	reorganizeControl(topEdit, cameraMode, parentRectangle);
	reorganizeControl(bottomEdit, cameraMode, parentRectangle);
	reorganizeControl(verticalBinningEdit, cameraMode, parentRectangle);
	reorganizeControl(leftDisp, cameraMode, parentRectangle);
	reorganizeControl(rightDisp, cameraMode, parentRectangle);
	reorganizeControl(horizontalBinningDisp, cameraMode, parentRectangle);
	reorganizeControl(topDisp, cameraMode, parentRectangle);
	reorganizeControl(bottomDisp, cameraMode, parentRectangle);
	reorganizeControl(verticalBinningDisp, cameraMode, parentRectangle);
	reorganizeControl(setImageParametersButton, cameraMode, parentRectangle);
	return false;
}
