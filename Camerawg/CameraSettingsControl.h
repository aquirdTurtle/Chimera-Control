#pragma once

#include "Control.h"
#include "PictureSettingsControl.h"
#include "CameraImageDimensions.h"
#include "Andor.h"

struct cameraPositions;

/*
 * This large class maintains all of the settings & user interactions for said settings of the Andor camera. It more or
 * less contains the PictureSettingsControl Class, as this is meant to be the parent of such an object. It is distinct
 * but highly related to the Andor class, where the Andor class is the class that actually manages communications with
 * the camera and some base settings that the user does not change. Because of the close contact between this and the
 * andor class, this object is initialized with a pointer to the andor object.
 ***********************************************************************************************************************/
class CameraSettingsControl
{
	public:
		CameraSettingsControl::CameraSettingsControl(AndorCamera* friendInitializer) : picSettingsObj(this)
		{ 
			andorFriend = friendInitializer; 
			runSettings.exposureTimes = { 20 };
			runSettings.picsPerRepetition = 1;
			runSettings.kinetiCycleTime = 0.1f;
			if ( ANDOR_SAFEMODE )
			{
				runSettings.picsPerRepetition = 1;
				runSettings.repetitionsPerVariation = 10;
				runSettings.totalVariations = 3;
				runSettings.totalPicsInExperiment = 30;
				runSettings.totalPicsInVariation = 10;
			}
		}
		CBrush* handleColor(int idNumber, CDC* colorer, brushMap brushes, rgbMap rgbs);
		void initialize(cameraPositions& pos, int& id, CWnd* parent, fontMap fonts, std::vector<CToolTipCtrl*>& tooltips);
		void checkTimings(std::vector<float> exposureTimes);
		void checkTimings(float kineticCycleTime, float accumulationTime, std::vector<float> exposureTimes);
		imageParameters readImageParameters(CameraWindow* camWin);
		void setEmGain(AndorCamera* andorObj);
		void rearrange(std::string cameraMode, std::string triggerMode, int width, int height, fontMap fonts);
		void handlePictureSettings(UINT id, AndorCamera* andorObj);
		void handleTriggerControl(CameraWindow* cameraWindow);
		std::array<int, 4> getPaletteNumbers();
		void handleSetTemperatureOffPress();
		void handleSetTemperaturePress();
		void handleTimer();
		void checkIfReady();
		void cameraIsOn( bool state );
		void handleModeChange( CameraWindow* cameraWindow );
		AndorRunSettings getSettings();
		std::array<int, 4> getThresholds();
	private:
		AndorCamera* andorFriend;

		Control<CStatic> header;
		// Accumulation Time
		// Accumulation Number

		// cameraMode
		Control<CComboBox> cameraModeCombo;
		// EM Gain
		Control<CButton> emGainButton;
		Control<CEdit> emGainEdit;
		Control<CStatic> emGainDisplay;
		// Trigger Mode
		Control<CStatic> triggerLabel;
		Control<CComboBox> triggerCombo;
		// Temperature
		Control<CButton> setTemperatureButton;
		Control<CButton> temperatureOffButton;
		Control<CEdit> temperatureEdit;
		Control<CStatic> temperatureDisplay;
		Control<CStatic> temperatureMessage;

		// Kinetic Cycle Time
		Control<CEdit> kineticCycleTimeEdit;
		Control<CStatic> kineticCycleTimeLabel;

		std::string currentControlColor;
		// two subclassed groups.
		CameraImageDimensionsControl imageDimensionsObj;
		PictureSettingsControl picSettingsObj;
		// the currently selected settings, not necessarily those being used to run the current
		// experiment.
		AndorRunSettings runSettings;
};

