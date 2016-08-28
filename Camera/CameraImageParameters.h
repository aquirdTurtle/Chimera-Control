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
		HwndControl leftText;
		HwndControl rightText;
		HwndControl horizontalBinningText;
		HwndControl topText;
		HwndControl bottomText;
		HwndControl verticalBinningText;
		HwndControl leftEdit;
		HwndControl rightEdit;
		HwndControl horizontalBinningEdit;
		HwndControl topEdit;
		HwndControl bottomEdit;
		HwndControl verticalBinningEdit;
		HwndControl leftDisp;
		HwndControl rightDisp;
		HwndControl horizontalBinningDisp;
		HwndControl topDisp;
		HwndControl bottomDisp;
		HwndControl verticalBinningDisp;
		HwndControl setImageParametersButton;
		bool isReady;
		imageParameters currentImageParameters;
};
