// created by Mark O. Brown
#pragma once

#include <string>
#include <fstream>
#include "Version.h"
#include "Control.h"
#include "commonTypes.h"
#include "imageParameters.h"
#include "BaslerSettings.h"
#include "DoubleEdit.h"

class BaslerSettingsControl
{
	public:
		BaslerSettingsControl ( );
		void initialize(POINT& pos, int& id, CWnd* parent, int picWidth, int picHeight, POINT cameraDims);
		void redrawMotIndicator ( );
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
		void handleOpenConfig ( std::ifstream& configFile, Version ver );

		/// TODO: fill in correct parameters here.
		const imageParameters ScoutFullResolution = { 1,500,1,500,1,1 };
		const imageParameters AceFullResolution = { 1,500,1,500,1,1 };
		const UINT unityGainSetting = 260;
		
		double getMotThreshold ( );
		static baslerSettings getBaslerSettingsFromConfig ( std::ifstream& configFile, Version ver );

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

		Control<CStatic> motThreshold;
		Control<DoubleEdit> motThresholdEdit;
		Control<CStatic> motLoadedColorbox;

		bool motLoaded;

		bool isReady;
};
