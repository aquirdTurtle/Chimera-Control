#pragma once

#include "Control.h"

struct imageParameters
{
	int leftBorder;
	int rightBorder;
	int topBorder;
	int bottomBorder;
	int horizontalBinning;
	int verticalBinning;
	int width;
	int height;
};

class CameraImageParameters
{
	public:
		CameraImageParameters();
		bool initiateControls(POINT& topLeftPositionKinetic, POINT& topLeftPositionAccumulate, POINT& topLeftPositionContinuous,
							  HWND parentWindow, bool isTriggerModeSensitive);
		bool setImageParameters();
		bool setImageParametersFromInput(imageParameters param);
		bool checkReady();
		imageParameters getImageParameters();
		bool reorganizeControls(RECT parentRectangle, std::string cameraMode);
		INT_PTR colorEdits(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
	private:
		Control leftText;
		Control rightText;
		Control horizontalBinningText;
		Control topText;
		Control bottomText;
		Control verticalBinningText;
		Control leftEdit;
		Control rightEdit;
		Control horizontalBinningEdit;
		Control topEdit;
		Control bottomEdit;
		Control verticalBinningEdit;
		Control leftDisp;
		Control rightDisp;
		Control horizontalBinningDisp;
		Control topDisp;
		Control bottomDisp;
		Control verticalBinningDisp;
		Control setImageParametersButton;
		bool isReady;
		imageParameters currentImageParameters;
};
