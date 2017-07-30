#pragma once

#include "Control.h"
#include <array>
#include <vector>

class PictureOptions
{
	public:
		bool initialize(POINT& kineticPos, POINT& continuousPos, POINT& accumulatePos, HWND parent, int& id);
		bool handleOptionChange(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);
		bool disablePictureControls(int pic);
		bool enablePictureControls(int pic);
		bool setExposureTimes(std::vector<float> times);
		bool setExposureTimes();
		std::array<int, 4> getPictureColors();
		std::vector<float> getUsedExposureTimes();
		std::array<int, 4> getThresholds();
		INT_PTR colorControls( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
		void confirmAcquisitionTimings();
		void setPicturesPerExperiment(unsigned int pics);
		void setThresholds(std::array<int, 4> thresholds);
		bool reorganizeControls(RECT parentRectangle, std::string cameraMode);
	private:
		std::array<int, 4> colors;
		std::vector<float> exposureTimes;
		std::array<int, 4> thresholds;
		unsigned int picsPerExperiment;
		// 
		HwndControl setPictureOptionsButton;
		HwndControl totalPicNumberLabel;
		HwndControl pictureLabel;
		HwndControl exposureLabel;
		HwndControl thresholdLabel;
		HwndControl blackWhiteLabel;
		HwndControl redBlueLabel;
		HwndControl yellowBlueLabel;
		// 
		std::array<HwndControl, 4> totalNumberChoice;
		std::array<HwndControl, 4> pictureNumbers;
		std::array<HwndControl, 4> exposureEdits;
		std::array<HwndControl, 4> thresholdEdits;
		std::array<HwndControl, 4> blackWhiteRadios;
		std::array<HwndControl, 4> yellowBlueRadios;
		std::array<HwndControl, 4> redBlueRadios;
		
};


