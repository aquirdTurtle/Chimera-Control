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
			// initialize settings. Most of these have been picked to match initial settings set in the "initialize" 
			// function.
			runSettings.exposureTimes = { 0.026f };
			runSettings.picsPerRepetition = 1;
			runSettings.kinetiCycleTime = 0.1f;
			runSettings.repetitionsPerVariation = 10;
			runSettings.totalVariations = 3;
			runSettings.totalPicsInExperiment = 30;
			runSettings.totalPicsInVariation = 10;
			// the read mode never gets changed currently. we always want images.
			runSettings.readMode = 4;
			runSettings.acquisitionMode = 3;
			runSettings.emGainModeIsOn = false;
			runSettings.showPicsInRealTime = false;
			runSettings.triggerMode = "External Trigger";
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
		void handleSetVarNum();
		void handleSetRepsPerVar();
		AndorRunSettings getSettings();
		std::array<int, 4> getThresholds();
		void updatePassivelySetSettings();
	private:
		AndorCamera* andorFriend;

		Control<CStatic> header;
		/// TODO
		// Accumulation Time
		Control<CStatic> accumulationCycleTimeLabel;
		Control<CEdit> accumulationCycleTimeEdit;
		// Accumulation Number
		Control<CStatic> accumulationNumberLabel;
		Control<CEdit> accumulationNumberEdit;

		// cameraMode
		Control<CComboBox> cameraModeCombo;
		// EM Gain
		Control<CButton> emGainButton;
		Control<CEdit> emGainEdit;
		Control<CStatic> emGainDisplay;
		// Trigger Mode
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

		// repetitions per variation
		Control<CButton> setRepsPerVar;
		Control<CEdit> repsPerVarEdit;
		Control<CStatic> repsPerVarDisp;

		// total variations
		Control<CButton> setVarNum;
		Control<CEdit> varNumEdit;
		Control<CStatic> varNumDisp;


		std::string currentControlColor;
		// two subclassed groups.
		CameraImageDimensionsControl imageDimensionsObj;
		PictureSettingsControl picSettingsObj;
		// the currently selected settings, not necessarily those being used to run the current
		// experiment.
		AndorRunSettings runSettings;
};

