#pragma once

#include "Control.h"
#include <array>
#include <vector>

class AndorCamera;
class CameraSettingsControl;

/*
 * This class handles all of the gui objects for assigning camera settings. It works closely with the Andor class
 * because it eventually needs to communicate all of these settings to the Andor class.
 */
class PictureSettingsControl
{
	public:
		// must have parent. Enforced partially because both are singletons.
		PictureSettingsControl::PictureSettingsControl(CameraSettingsControl* parentObj) { parentSettingsControl = parentObj; }
		bool initialize(POINT& kineticPos, POINT& continuousPos, POINT& accumulatePos, CWnd* parent, int& id);
		bool handleOptionChange(UINT id, AndorCamera* andorObj);
		bool disablePictureControls(int pic);
		bool enablePictureControls(int pic);
		bool setExposureTimes(std::vector<float> times, AndorCamera* andorObj);
		bool setExposureTimes(AndorCamera* andorObj);
		std::array<int, 4> getPictureColors();
		std::vector<float> getUsedExposureTimes();
		std::array<int, 4> getThresholds();
		CBrush* colorControls(int idNumber, CDC* colorer, std::unordered_map<std::string, CBrush*> brushes,
			std::unordered_map<std::string, COLORREF> rgbs);
		void confirmAcquisitionTimings();
		void setPicturesPerExperiment(unsigned int pics, AndorCamera* andorObj);
		void setThresholds(std::array<int, 4> thresholds);
		bool rearrange(std::string cameraMode, std::string triggerMode, int width, int height, std::unordered_map<std::string, CFont*> fonts);
		int getPicsPerRepetition();
	private:
		CameraSettingsControl* parentSettingsControl;
		std::array<int, 4> colors;
		std::vector<float> exposureTimesUnofficial;
		std::array<int, 4> thresholds;
		// This variable is used by this control and communicated to the andor object, but is not directly accessed
		// while the main camera control needs to figure out how many pictures per repetition there are.
		unsigned int picsPerRepetitionUnofficial;
		/// Grid of PictureOptions
		Control<CButton> setPictureOptionsButton;		
		Control<CStatic> totalPicNumberLabel;
		Control<CStatic> pictureLabel;
		Control<CStatic> exposureLabel;
		Control<CStatic> thresholdLabel;
		Control<CStatic> blackWhiteLabel;
		Control<CStatic> redBlueLabel;
		Control<CStatic> yellowBlueLabel;
		// 
		std::array<Control<CButton>, 4> totalNumberChoice;
		std::array<Control<CStatic>, 4> pictureNumbers;
		std::array<Control<CEdit>, 4> exposureEdits;
		std::array<Control<CEdit>, 4> thresholdEdits;
		std::array<Control<CButton>, 4> blackWhiteRadios;
		std::array<Control<CButton>, 4> yellowBlueRadios;
		std::array<Control<CButton>, 4> redBlueRadios;
};


