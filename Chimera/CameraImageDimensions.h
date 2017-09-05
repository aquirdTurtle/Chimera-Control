#pragma once

#include "Control.h"
#include "imageParameters.h"

struct cameraPositions;
class CameraWindow;
class MainWindow;


class CameraImageDimsControl
{
	public:
		CameraImageDimsControl();
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
		Control<CStatic>  horBinningText;
		Control<CStatic>  bottomLabel;
		Control<CStatic>  topLabel;
		Control<CStatic>  vertBinningText;
		Control<CEdit>  leftEdit;
		Control<CEdit> rightEdit;
		Control<CEdit> horBinningEdit;
		Control<CEdit> bottomEdit;
		Control<CEdit> topEdit;
		Control<CEdit> vertBinningEdit;
		Control<CButton> setImageDimsButton;
		bool isReady;
		imageParameters currentImageParameters;
};
;