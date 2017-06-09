#pragma once

#include "Control.h"

struct cameraPositions;

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

class CameraImageDimensionsControl
{
	public:
		CameraImageDimensionsControl();
		void initialize( cameraPositions& pos, CWnd* parentWindow, bool isTriggerModeSensitive, int& id );
		imageParameters readImageParameters( CameraWindow* camWin );
		void setImageParametersFromInput( imageParameters param, CameraWindow* camWin );
		bool checkReady();
		imageParameters getImageParameters();
		void rearrange( std::string cameraMode, std::string triggerMode, int width, int height, fontMap fonts );
		HBRUSH colorEdits( HWND window, UINT message, WPARAM wParam, LPARAM lParam, MainWindow* mainWin );
		void drawBackgrounds( CameraWindow* camWin );
		void cameraIsOn( bool state );
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
		Control<CButton> setImageDimensionsButton;
		bool isReady;
		imageParameters currentImageParameters;
};
;