#pragma once

#include "Control.h"
#include "Version.h"
#include "imageParameters.h"

struct cameraPositions;
class CameraWindow;
class MainWindow;


class ImageDimsControl
{
	public:
		ImageDimsControl();
		void initialize( cameraPositions& pos, CWnd* parentWindow, bool isTriggerModeSensitive, int& id );
		void updateWidthHeight( );
		imageParameters readImageParameters();
		void setImageParametersFromInput( imageParameters param, CameraWindow* camWin );
		bool checkReady();
		void handleSave( std::ofstream& saveFile );
		void handleNew( std::ofstream& newfile );
		void handleOpen( std::ifstream& openFile, Version ver );
		imageParameters getImageParameters();
		void rearrange( std::string cameraMode, std::string triggerMode, int width, int height, fontMap fonts );
		HBRUSH colorEdits( HWND window, UINT message, WPARAM wParam, LPARAM lParam, MainWindow* mainWin );
		void drawBackgrounds( CameraWindow* camWin );
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
		bool isReady;
		imageParameters currentImageParameters;
};
;