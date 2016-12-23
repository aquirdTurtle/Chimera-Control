#pragma once

#include "Control.h"
#include "PictureSettingsControl.h"
#include "CameraImageParameters.h"
#include "Andor.h"

struct cameraPositions;

/*
// The following definition is inside Andor.h
struct AndorRunSettings
{
//
bool emGainModeIsOn;
int readMode;
int acquisitionMode;
std::string triggerMode;
std::string cameraMode;
bool showPicsInRealTime;
//
double kinetiCycleTime;
double accumulationTime;
//
int picsPerRepetition;
int repetitionsPerVariation;
int totalVariations;
int totalPicsInExperiment;
int totalPicsInVariation;
};
*/

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
			runSettings.cameraMode = "Kinetic Series Mode";
			runSettings.triggerMode = "External";
		}
		void initialize(cameraPositions& pos, int& id, CWnd* parent);
		void checkTimings(std::vector<float> exposureTimes, Communicator* comm);
		void checkTimings(float kineticCycleTime, float accumulationTime, std::vector<float> exposureTimes, Communicator* comm);
		imageParameters readImageParameters(CameraWindow* camWin, Communicator* comm);
		void setEmGain(AndorCamera* andorObj, Communicator* comm);
		void reorganizeControls(std::string cameraMode, std::string triggerMode, int width, int height);
		AndorRunSettings getSettings();
	private:
		AndorCamera* andorFriend;
		// Header
		Control<CStatic> header;
		// Accumulation Time
		// Accumulation Number
		// Kinetic Cycle Time
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
		// CameraImageParametersControl
		CameraImageParametersControl imageParametersObj;
		// Individual Picture Settings
		PictureSettingsControl picSettingsObj;
		// the currently selected settings, not necessarily those being used to run the current
		// experiment.
		AndorRunSettings runSettings;
};
