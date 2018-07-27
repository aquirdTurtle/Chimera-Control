#pragma once

#include <string>
#include <fstream>
#include "Version.h"
#include "Control.h"
#include "commonTypes.h"
#include "imageParameters.h"

struct baslerSettings
{
	unsigned int rawGain;
	std::string exposureMode;
	double exposureTime;
	std::string cameraMode;
	unsigned int repCount;
	std::string triggerMode;
	double frameRate;
	imageParameters dimensions;
};


class BaslerSettingsControl
{
	public:
		void initialize(POINT& pos, int& id, CWnd* parent, int picWidth, int picHeight, POINT cameraDims);
		
		void handleGain();
		void setStatus(std::string status);
		void handleExposureMode();
		void handleCameraMode();
		void handleFrameRate();
		baslerSettings loadCurrentSettings( );
		baslerSettings getCurrentSettings();		
		void setSettings ( baslerSettings newSettings );
		void updateExposure( double exposure );
		void rearrange(int width, int height, fontMap fonts);
		void handleSavingConfig ( std::ofstream& configFile );
		void handleOpeningConfig ( std::ifstream& configFile, Version ver );
	private:
		ULONG lastTime;
		baslerSettings currentSettings;
		Control<CStatic> statusText;
		// exposure
		Control<CStatic> exposureText;
		Control<CComboBox> exposureModeCombo;
		Control<CEdit> exposureEdit;
		Control<CButton> setExposure;
		// trigger
		Control<CComboBox> triggerCombo;
		// Dimensions & Binning
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

		// camera mode: continuous or set #
		Control<CComboBox> cameraMode;
		Control<CStatic> repText;
		Control<CEdit> repEdit;

		Control<CStatic> frameRateText;
		Control<CEdit> frameRateEdit;
		Control<CStatic> realFrameRate;

		Control<CStatic> gainText;
		Control<CComboBox> gainCombo;
		Control<CEdit> gainEdit;
		Control<CStatic> realGainText;
		Control<CStatic> realGainStatus;

		Control<CButton> linkToMain;
		Control<CStatic> picsPerRepTxt;
		Control<CEdit> picsPerRepEdit;

		bool isReady;
};
