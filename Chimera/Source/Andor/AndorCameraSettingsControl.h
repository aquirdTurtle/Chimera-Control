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
		void initialize( QPoint& pos, IChimeraQtWindow* parent, std::vector<std::string> vertSpeeds, 
						 std::vector<std::string> horSpeeds );
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
		unsigned getHsSpeed ();
		unsigned getVsSpeed ();
		int getVerticalShiftVoltageAmplitude();
		unsigned getFrameTransferMode ();
	private:

		AndorRunSettings currentlyRunningSettings;
		bool currentlyRunning = false;
		bool currentlyUneditable = false;
		double getKineticCycleTime( );
		double getAccumulationCycleTime( );
		unsigned getAccumulationNumber( );
		imageParameters readImageParameters( );
		QLabel* header = nullptr;
		QPushButton* programNow = nullptr;
		QCheckBox* viewRunningSettings = nullptr;
		CQCheckBox* controlAndorCameraCheck = nullptr;
		// Hardware Accumulation Parameters
		QLabel* accumulationCycleTimeLabel = nullptr;
		CQLineEdit* accumulationCycleTimeEdit = nullptr;
		QLabel* accumulationNumberLabel = nullptr;
		CQLineEdit* accumulationNumberEdit = nullptr;
		// 
		CQComboBox* cameraModeCombo = nullptr;
		
		CQComboBox* frameTransferModeCombo = nullptr;
		CQComboBox* verticalShiftSpeedCombo = nullptr;
		CQComboBox* horizontalShiftSpeedCombo = nullptr;

		QLabel* verticalShiftVoltAmpLabel = nullptr;
		CQLineEdit* verticalShiftVoltAmpEdit = nullptr;

		QLabel* emGainLabel = nullptr;
		CQLineEdit* emGainEdit = nullptr;
		CQPushButton* emGainBtn = nullptr;
		QLabel* emGainDisplay = nullptr;
		CQComboBox* triggerCombo = nullptr;
		// Temperature
		CQPushButton* setTemperatureButton = nullptr;
		CQPushButton* temperatureOffButton = nullptr;
		CQLineEdit* temperatureEdit = nullptr;
		QLabel* temperatureDisplay = nullptr;
		QLabel* temperatureMsg = nullptr;

		// Kinetic Cycle Time
		CQLineEdit* kineticCycleTimeEdit = nullptr;
		QLabel* kineticCycleTimeLabel = nullptr;
		QLabel* minKineticCycleTimeDisp = nullptr;
		QLabel* minKineticCycleTimeLabel = nullptr;
		// two subclassed groups.
		ImageDimsControl imageDimensionsObj;
		PictureSettingsControl picSettingsObj;

		CameraCalibration calControl;
		// the currently selected settings, not necessarily those being used to run the current
		// experiment.
		AndorCameraSettings configSettings;
};

