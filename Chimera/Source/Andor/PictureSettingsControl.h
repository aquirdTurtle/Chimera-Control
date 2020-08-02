// created by Mark O. Brown
#pragma once

#include "CustomMfcControlWrappers/Control.h"
#include "ConfigurationSystems/Version.h"
#include "Andor/cameraPositions.h"
#include "CustomMfcControlWrappers/DoubleEdit.h"
#include "CustomMfcControlWrappers/myButton.h"
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
class PictureSettingsControl
{
	public:
		// must have parent. Enforced partially because both are singletons.
		// PictureSettingsControl( )
		void updateAllSettings ( andorPicSettingsGroup inputSettings );
		void handleSaveConfig(ConfigStream& saveFile);
		void handleOpenConfig(ConfigStream& openFile, AndorCameraCore* andor);
		void initialize( POINT& pos, IChimeraQtWindow* parent );
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
		std::array<std::string, 4> getThresholdStrings();
		std::array<softwareAccumulationOption, 4> getSoftwareAccumulationOptions ( );
		void setSoftwareAccumulationOptions ( std::array<softwareAccumulationOption, 4> opts );
		static andorPicSettingsGroup getPictureSettingsFromConfig (ConfigStream& configFile );
	private:
		// the internal memory of the settings here is somewhat redundant with the gui objects. It'd probably be better
		// if this didn't exist and all the getters just converted straight from the gui objects, but that's a 
		// refactoring for another time.
		andorPicSettingsGroup settings;
		/// Grid of PictureOptions
		QLabel* totalPicNumberLabel;
		QLabel* pictureLabel;
		QLabel* exposureLabel;
		QLabel* thresholdLabel;
		QLabel* colormapLabel;
		QLabel* displayTypeLabel;
		QLabel* softwareAccumulationLabel;
		// 
		std::array<CQRadioButton*, 4> totalNumberChoice;
		std::array<QLabel*, 4> pictureNumbers;
		std::array<CQLineEdit*, 4> exposureEdits;
		std::array<CQLineEdit*, 4> thresholdEdits;
		std::array<CQComboBox*, 4> colormapCombos;
		std::array<CQComboBox*, 4> displayTypeCombos;
		std::array<CQCheckBox*, 4> softwareAccumulateAll;
		std::array<CQLineEdit*, 4> softwareAccumulateNum;
};


