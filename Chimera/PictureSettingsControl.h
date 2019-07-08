// created by Mark O. Brown
#pragma once

#include "Control.h"
#include "Version.h"
#include "cameraPositions.h"
#include "DoubleEdit.h"
#include "myButton.h"
#include "softwareAccumulationOption.h"
#include <array>
#include <vector>

class AndorCamera;
class AndorCameraSettingsControl;


struct displayTypeOption
{
	bool isDiff = false;
	// zero-indexed.
	UINT whichPicForDif = 0;
};


/*
 * This class handles all of the gui objects for assigning camera settings. It works closely with the Andor class
 * because it eventually needs to communicate all of these settings to the Andor class.
 */
class PictureSettingsControl
{
	public:
		// must have parent. Enforced partially because both are singletons.
		PictureSettingsControl( AndorCameraSettingsControl* parentObj) { parentSettingsControl = parentObj; }
		void handleNewConfig( std::ofstream& newFile );
		void handleSaveConfig(std::ofstream& saveFile);
		void handleOpenConfig(std::ifstream& openFile, Version ver, AndorCamera* andor);
		void initialize( cameraPositions& pos, CWnd* parent, int& id);
		void handleOptionChange(int id, AndorCamera* andorObj);
		void disablePictureControls(int pic);
		void enablePictureControls(int pic);
		void setExposureTimes(std::vector<float>& times, AndorCamera* andorObj);
		void setExposureTimes(AndorCamera* andorObj);
		std::array<int, 4> getPictureColors();
		std::vector<float> getUsedExposureTimes();
		std::array<std::vector<int>, 4> getThresholds();
		std::array<displayTypeOption, 4> getDisplayTypeOptions( );
		CBrush* colorControls(int idNumber, CDC* colorer );
		void confirmAcquisitionTimings();
		void setPicturesPerExperiment(UINT pics, AndorCamera* andorObj);
		void setThresholds( std::array<std::string, 4> thresholds);
		void rearrange(AndorRunModes::mode cameraMode, AndorTriggerMode::mode triggerMode, int width, int height, fontMap fonts);
		UINT getPicsPerRepetition();
		void updateSettings( );
		void setUnofficialPicsPerRep( UINT picNum, AndorCamera* andorObj );
		std::array<std::string, 4> getThresholdStrings();
		std::array<softwareAccumulationOption, 4> getSoftwareAccumulationOptions ( );
		void setSoftwareAccumulationOptions ( std::array<softwareAccumulationOption, 4> opts );
	private:
		AndorCameraSettingsControl* parentSettingsControl;
		std::array<int, 4> colors;
		std::vector<float> exposureTimesUnofficial;
		std::array<std::vector<int>, 4> thresholds;
		// This variable is used by this control and communicated to the andor object, but is not directly accessed
		// while the main camera control needs to figure out how many pictures per repetition there are.
		UINT picsPerRepetitionUnofficial;
		/// Grid of PictureOptions
		Control<CStatic> totalPicNumberLabel;
		Control<CStatic> pictureLabel;
		Control<CStatic> exposureLabel;
		Control<CStatic> thresholdLabel;
		Control<CStatic> colormapLabel;
		Control<CStatic> displayTypeLabel;
		Control<CStatic> softwareAccumulationLabel;
		// 
		std::array<Control<CButton>, 4> totalNumberChoice;
		std::array<Control<CStatic>, 4> pictureNumbers;
		std::array<Control<DoubleEdit>, 4> exposureEdits;
		std::array<Control<DoubleEdit>, 4> thresholdEdits;
		std::array<Control<CComboBox>, 4> colormapCombos;
		std::array<Control<CComboBox>, 4> displayTypeCombos;
		std::array<Control<CleanCheck>, 4> softwareAccumulateAll;
		std::array<Control<CEdit>, 4> softwareAccumulateNum;
};


