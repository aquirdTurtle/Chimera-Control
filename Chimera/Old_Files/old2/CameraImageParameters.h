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

class CameraImageParametersControl
{
	public:
		CameraImageParametersControl();
		bool initialize(POINT& topLeftPositionKinetic, POINT& topLeftPositionAccumulate, POINT& topLeftPositionContinuous,
							  CWnd* parentWindow, bool isTriggerModeSensitive, int& id);
		imageParameters readImageParameters(CameraWindow* camWin);
		bool setImageParametersFromInput(imageParameters param, CameraWindow* camWin);
		bool checkReady();
		imageParameters getImageParameters();
		bool rearrange(std::string cameraMode, std::string triggerMode, int width,
			int height, std::unordered_map<std::string, CFont*> fonts);
		HBRUSH colorEdits(HWND window, UINT message, WPARAM wParam, LPARAM lParam, MainWindow* mainWin);
		bool drawBackgrounds(CameraWindow* camWin);
		// TODO:
		// bool drawPicture();
		// bool drawSelectionCircle();
		// bool drawAnalysisSquares();
	private:
		Control<CStatic> leftText;
		Control<CStatic>  rightText;
		Control<CStatic>  horizontalBinningText;
		Control<CStatic>  topText;
		Control<CStatic>  bottomText;
		Control<CStatic>  verticalBinningText;
		Control<CEdit>  leftEdit;
		Control<CEdit> rightEdit;
		Control<CEdit> horizontalBinningEdit;
		Control<CEdit> topEdit;
		Control<CEdit> bottomEdit;
		Control<CEdit> verticalBinningEdit;
		Control<CButton> setImageParametersButton;
		bool isReady;
		imageParameters currentImageParameters;
};
