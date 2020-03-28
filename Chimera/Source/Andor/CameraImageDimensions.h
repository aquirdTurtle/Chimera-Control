// created by Mark O. Brown
#pragma once

#include "Control.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "GeneralImaging/imageParameters.h"

struct cameraPositions;
class AndorWindow;
class MainWindow;


class ImageDimsControl
{
	public:
		ImageDimsControl();
		void initialize( POINT& pos, CWnd* parentWindow, bool isTriggerModeSensitive, int& id );
		imageParameters readImageParameters();
		void setImageParametersFromInput( imageParameters param );
		bool checkReady();
		void handleSave(ConfigStream& saveFile );
		void handleOpen(ConfigStream& openFile, Version ver );
		static imageParameters getImageDimSettingsFromConfig (ConfigStream& configFile, Version ver );
		imageParameters getImageParameters();
		void rearrange( int width, int height, fontMap fonts );
		HBRUSH colorEdits( HWND window, UINT message, WPARAM wParam, LPARAM lParam, MainWindow* mainWin );
	private:
		Control<CStatic> leftText;
		Control<CStatic> rightText;
		Control<CStatic> horBinningText;
		Control<CStatic> bottomLabel;
		Control<CStatic> topLabel;
		Control<CStatic> vertBinningText;
		Control<CEdit>   leftEdit;
		Control<CEdit>   rightEdit;
		Control<CEdit>   horBinningEdit;
		Control<CEdit>   bottomEdit;
		Control<CEdit>   topEdit;
		Control<CEdit>   vertBinningEdit;
		bool isReady;
		imageParameters currentImageParameters;
};
;