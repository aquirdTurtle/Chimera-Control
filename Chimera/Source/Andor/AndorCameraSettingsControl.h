// created by Mark O. Brown
#pragma once

#include "ConfigurationSystems/Version.h"
#include "PictureSettingsControl.h"
#include "CameraImageDimensions.h"
#include "CameraCalibration.h"
#include "Andor/AndorCameraCore.h"
#include "ConfigurationSystems/Version.h"
#include "GeneralImaging/softwareAccumulationOption.h"
#include "PrimaryWindows/IChimeraQtWindow.h"
#include <qlabel.h>
#include <qcheckbox>
#include <qcombobox.h>
#include <qlineedit.h>

struct cameraPositions;

/*
 * This large class maintains all of the settings & user interactions for said settings of the Andor camera. It more or
 * less contains the PictureSettingsControl Class, as this is meant to be the parent of such an object. It is distinct
 * but highly related to the Andor class, where the Andor class is the class that actually manages communications with
 * the camera and some base settings that the user does not change. Because of the close contact between this and the
 * andor class, this object is initialized with a pointer to the andor object.
 ***********************************************************************************************************************/
class AndorCameraSettingsControl{
	public:
		AndorCameraSettingsControl();
		void setVariationNumber(unsigned varNumber);
		void setRepsPerVariation(unsigned repsPerVar);
		void updateRunSettingsFromPicSettings( );
		void initialize(QPoint& pos, IChimeraQtWindow* parent );
		void updateSettings( );
		void updateMinKineticCycleTime( double time );
		void setEmGain( bool currentlyOn, int currentEmGainLevel );
		void updateWindowEnabledStatus ();
		void handlePictureSettings();
		void updateTriggerMode( );
		void handleSetTemperaturePress();
		void changeTemperatureDisplay( AndorTemperatureStatus stat );
		void checkIfReady();
		void cameraIsOn( bool state );
		void updateCameraMode( );
		AndorCameraSettings getConfigSettings();
		AndorCameraSettings getCalibrationSettings( );
		bool getAutoCal( );
		bool getUseCal( );
		void setImageParameters(imageParameters newSettings);
		void setRunSettings(AndorRunSettings inputSettings);
		void updateImageDimSettings ( imageParameters settings );
		void updatePicSettings ( andorPicSettingsGroup settings );
		void updateDisplays ();
		static andorPicSettingsGroup getPictureSettingsFromConfig (ConfigStream& configFile);
		void handleSaveConfig(ConfigStream& configFile);
		void handelSaveMasterConfig(std::stringstream& configFile);
		void handleOpenMasterConfig(ConfigStream& configFile, QtAndorWindow* camWin);
		std::vector<Matrix<long>> getImagesToDraw( const std::vector<Matrix<long>>& rawData  );
		const imageParameters fullResolution = { 1,512,1,512,1,1 };
		std::array<softwareAccumulationOption, 4> getSoftwareAccumulationOptions ( );
		Qt::TransformationMode getTransformationMode ();
		void setConfigSettings (AndorRunSettings inputSettings);
		AndorRunSettings getRunningSettings ();
	private:
		AndorRunSettings currentlyRunningSettings;

		bool currentlyUneditable = false;
		double getKineticCycleTime( );
		double getAccumulationCycleTime( );
		unsigned getAccumulationNumber( );
		imageParameters readImageParameters( );
		QLabel* header;
		QCheckBox* viewRunningSettings;
		CQCheckBox* controlAndorCameraCheck;
		// Hardware Accumulation Parameters
		QLabel* accumulationCycleTimeLabel;
		CQLineEdit* accumulationCycleTimeEdit;
		QLabel* accumulationNumberLabel;
		CQLineEdit* accumulationNumberEdit;
		// 
		CQComboBox* cameraModeCombo;
		QLabel* emGainLabel;
		CQLineEdit* emGainEdit;
		CQPushButton* emGainBtn;
		QLabel* emGainDisplay;
		CQComboBox* triggerCombo;
		// Temperature
		CQPushButton* setTemperatureButton;
		CQPushButton* temperatureOffButton;
		CQLineEdit* temperatureEdit;
		QLabel* temperatureDisplay;
		QLabel* temperatureMsg;

		// Kinetic Cycle Time
		CQLineEdit* kineticCycleTimeEdit;
		QLabel* kineticCycleTimeLabel;
		QLabel* minKineticCycleTimeDisp;
		QLabel* minKineticCycleTimeLabel;
		// two subclassed groups.
		ImageDimsControl imageDimensionsObj;
		PictureSettingsControl picSettingsObj;

		CameraCalibration calControl;
		// the currently selected settings, not necessarily those being used to run the current
		// experiment.
		AndorCameraSettings configSettings;
};

