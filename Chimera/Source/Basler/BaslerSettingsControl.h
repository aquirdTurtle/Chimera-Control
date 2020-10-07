// created by Mark O. Brown
#pragma once

#include "ConfigurationSystems/Version.h"
#include "Control.h"
#include "GeneralObjects/commonTypes.h"
#include "GeneralImaging/imageParameters.h"
#include "Basler/BaslerSettings.h"
#include "ConfigurationSystems/ConfigStream.h"
#include <string>
#include <fstream>
#include "PrimaryWindows/IChimeraQtWindow.h"
#include <Andor/CameraImageDimensions.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qcombobox.h>

class BaslerSettingsControl{
	public:
		BaslerSettingsControl ( );
		void initialize( QPoint& pos, int picWidth, int picHeight, QPoint cameraDims, IChimeraQtWindow* qtp );
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
		void handleSavingConfig ( ConfigStream& configFile );

		/// TODO: fill in correct parameters here.
		const imageParameters ScoutFullResolution = { 1,500,1,500,1,1 };
		const imageParameters AceFullResolution = { 1,500,1,500,1,1 };
		const unsigned unityGainSetting = 260;
		
		double getMotThreshold ( );
		static baslerSettings getSettingsFromConfig (ConfigStream& configFile, Version ver );

	private:
		unsigned long lastTime;
		baslerSettings currentSettings;
		QLabel* statusText;
		//
		CQCheckBox* baslerExpActiveCheck;
		// exposure
		QLabel* exposureText;
		QComboBox* exposureModeCombo;
		QLineEdit* exposureEdit;
		QPushButton* setExposure;
		// trigger
		QComboBox* triggerCombo;
		ImageDimsControl dims;
		// camera mode: continuous or set #
		QComboBox* cameraMode;
		QLabel* repText;
		QLineEdit* repEdit;

		QLabel* frameRateText;
		QLineEdit* frameRateEdit;
		QLabel* realFrameRate;

		QLabel* gainText;
		QComboBox* gainCombo;
		QLineEdit* gainEdit;
		QLabel* realGainText;
		QLabel* realGainStatus;

		QPushButton* linkToMain;
		QLabel* picsPerRepTxt;
		QLineEdit* picsPerRepEdit;

		QLabel* motThreshold;
		QLineEdit* motThresholdEdit;
		QLabel* motLoadedColorbox;

		bool motLoaded;

		bool isReady;
};
