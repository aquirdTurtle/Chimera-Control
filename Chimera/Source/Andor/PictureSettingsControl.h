// created by Mark O. Brown
#pragma once

#include "ConfigurationSystems/Version.h"
#include "GeneralImaging/softwareAccumulationOption.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "Andor/andorPicSettingsGroup.h"
#include <array>
#include <vector>
#include "PrimaryWindows/IChimeraQtWindow.h"
#include <qlabel.h>
#include <CustomQtControls/AutoNotifyCtrls.h>

class AndorCameraCore;
class AndorCameraSettingsControl;

struct displayTypeOption{
	bool isDiff = false;
	// zero-indexed.
	unsigned whichPicForDif = 0;
};

/*
 * This class handles all of the gui objects for assigning camera settings. It works closely with the Andor class
 * because it eventually needs to communicate all of these settings to the Andor class.
 */
class PictureSettingsControl {
	public:
		// must have parent. Enforced partially because both are singletons.
		// PictureSettingsControl( )
		void updateAllSettings ( andorPicSettingsGroup inputSettings );
		void handleSaveConfig(ConfigStream& saveFile);
		void handleOpenConfig(ConfigStream& openFile, AndorCameraCore* andor);
		void initialize( QPoint& pos, IChimeraQtWindow* parent );
		void handleOptionChange( );
		void setPictureControlEnabled (int pic, bool enabled);
		void setUnofficialExposures ( std::vector<float> times );
		std::array<int, 4> getPictureColors ( );
		std::array<float, 4> getExposureTimes ( );
		std::vector<float> getUsedExposureTimes();
		std::array<std::vector<int>, 4> getThresholds();
		std::array<displayTypeOption, 4> getDisplayTypeOptions( );
		void setThresholds( std::array<std::string, 4> thresholds);
		unsigned getPicsPerRepetition();
		void updateSettings( );
		void updateColormaps ( std::array<int, 4> colorsIndexes );
		void setUnofficialPicsPerRep( unsigned picNum);
		int getPicScaleFactor ();
		std::array<std::string, 4> getThresholdStrings();
		std::array<softwareAccumulationOption, 4> getSoftwareAccumulationOptions ( );
		void setSoftwareAccumulationOptions ( std::array<softwareAccumulationOption, 4> opts );
		static andorPicSettingsGroup getPictureSettingsFromConfig (ConfigStream& configFile );
		Qt::TransformationMode getTransformationMode ();
		void setEnabledStatus (bool viewRunningSettings);
	private:
		// the internal memory of the settings here is somewhat redundant with the gui objects. It'd probably be better
		// if this didn't exist and all the getters just converted straight from the gui objects, but that's a 
		// refactoring for another time.
		andorPicSettingsGroup currentPicSettings;
		unsigned unofficialPicsPerRep=1;
		/// Grid of PictureOptions
		QLabel* totalPicNumberLabel = nullptr;
		QLabel* pictureLabel = nullptr;
		QLabel* exposureLabel = nullptr;
		QLabel* thresholdLabel = nullptr;
		QLabel* colormapLabel = nullptr;
		QLabel* displayTypeLabel = nullptr;
		QLabel* softwareAccumulationLabel = nullptr;
		QLabel* picScaleFactorLabel = nullptr;
		QLineEdit* picScaleFactorEdit = nullptr;

		CQComboBox* transformationModeCombo=nullptr;
		// 
		std::array<CQRadioButton*, 4> totalNumberChoice = { nullptr };
		std::array<QLabel*, 4> pictureNumbers = { nullptr };
		std::array<CQLineEdit*, 4> exposureEdits = { nullptr };
		std::array<CQLineEdit*, 4> thresholdEdits = { nullptr };
		std::array<CQComboBox*, 4> colormapCombos = { nullptr };
		std::array<CQComboBox*, 4> displayTypeCombos = { nullptr };
		std::array<CQCheckBox*, 4> softwareAccumulateAll = { nullptr };
		std::array<CQLineEdit*, 4> softwareAccumulateNum = { nullptr };
};


