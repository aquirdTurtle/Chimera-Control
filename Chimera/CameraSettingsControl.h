// created by Mark O. Brown
#pragma once

#include "Control.h"
#include "myButton.h"
#include "Version.h"
#include "PictureSettingsControl.h"
#include "CameraImageDimensions.h"
#include "CameraCalibration.h"
#include "Andor.h"
#include "softwareAccumulationOption.h"

struct cameraPositions;

/*
 * This large class maintains all of the settings & user interactions for said settings of the Andor camera. It more or
 * less contains the PictureSettingsControl Class, as this is meant to be the parent of such an object. It is distinct
 * but highly related to the Andor class, where the Andor class is the class that actually manages communications with
 * the camera and some base settings that the user does not change. Because of the close contact between this and the
 * andor class, this object is initialized with a pointer to the andor object.
 ***********************************************************************************************************************/
class AndorCameraSettingsControl
{
	public:
		AndorCameraSettingsControl(AndorCamera* friendInitializer);
		void setVariationNumber(UINT varNumber);
		void setRepsPerVariation(UINT repsPerVar);
		void updateRunSettingsFromPicSettings( );
		CBrush* handleColor(int idNumber, CDC* colorer );
		void initialize(cameraPositions& pos, int& id, CWnd* parent, cToolTips& tooltips);
		void checkTimings(std::vector<float>& exposureTimes);
		void checkTimings(float& kineticCycleTime, float& accumulationTime, std::vector<float>& exposureTimes);

		void updateSettings( );
		void updateMinKineticCycleTime( double time );
		void setEmGain();
		void rearrange(AndorRunModes::mode cameraMode, AndorTriggerMode::mode triggerMode, int width, int height, fontMap fonts);
		void handlePictureSettings(UINT id, AndorCamera* andorObj);
		void updateTriggerMode( );
		void handleTriggerChange(AndorWindow* cameraWindow);
		void handleSetTemperatureOffPress();
		void handleSetTemperaturePress();
		void handleTimer();
		void checkIfReady();
		void cameraIsOn( bool state );
		void handleModeChange( AndorWindow* cameraWindow );
		void updateCameraMode( );
		AndorCameraSettings getSettings();
		AndorCameraSettings getCalibrationSettings( );
		bool getAutoCal( );
		bool getUseCal( );
		void setImageParameters(imageParameters newSettings, AndorWindow* camWin);
		void setRunSettings(AndorRunSettings inputSettings);
		void updateImageDimSettings ( imageParameters settings );
		void updatePicSettings ( andorPicSettingsGroup settings );

		static AndorRunSettings getRunSettingsFromConfig ( std::ifstream& configFile, Version ver );
		static andorPicSettingsGroup getPictureSettingsFromConfig ( std::ifstream& configFile, Version ver );
		static imageParameters getImageDimSettingsFromConfig ( std::ifstream& configFile ,Version ver );

		void handleOpenConfig(std::ifstream& configFile, Version ver );
		void handleNewConfig( std::ofstream& newFile );
		void handleSaveConfig(std::ofstream& configFile);

		void handelSaveMasterConfig(std::stringstream& configFile);
		void handleOpenMasterConfig(std::stringstream& configFile, Version ver, AndorWindow* camWin);

		std::vector<std::vector<long>> getImagesToDraw( const std::vector<std::vector<long>>& rawData  );

		const imageParameters fullResolution = { 1,512,1,512,1,1 };
		std::array<softwareAccumulationOption, 4> getSoftwareAccumulationOptions ( );
	private:
		double getKineticCycleTime( );
		double getAccumulationCycleTime( );
		UINT getAccumulationNumber( );
		imageParameters getImageParameters( );

		AndorCamera* andorFriend;
		Control<CStatic> header;
		// Hardware Accumulation Parameters
		Control<CStatic> accumulationCycleTimeLabel;
		Control<CEdit> accumulationCycleTimeEdit;
		Control<CStatic> accumulationNumberLabel;
		Control<CEdit> accumulationNumberEdit;

		Control<CComboBox> cameraModeCombo;
		Control<CStatic> emGainLabel;
		Control<CEdit> emGainEdit;
		Control<CStatic> emGainDisplay;
		Control<CComboBox> triggerCombo;
		// Temperature
		Control<CleanButton> setTemperatureButton;
		Control<CleanButton> temperatureOffButton;
		Control<CEdit> temperatureEdit;
		Control<CStatic> temperatureDisplay;
		Control<CStatic> temperatureMsg;

		// Kinetic Cycle Time
		Control<CEdit> kineticCycleTimeEdit;
		Control<CStatic> kineticCycleTimeLabel;
		Control<CEdit> minKineticCycleTimeDisp;
		Control<CStatic> minKineticCycleTimeLabel;
		// two subclassed groups.
		ImageDimsControl imageDimensionsObj;
		PictureSettingsControl picSettingsObj;

		CameraCalibration calControl;
		// the currently selected settings, not necessarily those being used to run the current
		// experiment.
		AndorCameraSettings settings;
};

