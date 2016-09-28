#include "stdafx.h"
#include "reorganizeWindow.h"
#include "reorganizeControl.h"
#include "externals.h"

int reorganizeWindow(std::string cameraMode, HWND parentWindow)
{
	RECT parentRectangle;
	GetWindowRect(parentWindow, &parentRectangle);
	/// Reorganize my class objects
	eCameraFileSystem.reorganizeControls(parentRectangle, cameraMode);
	eCameraWindowExperimentTimer.reorganizeControls(parentRectangle, cameraMode);
	eTextingHandler.reorganizeControls(parentRectangle, cameraMode);
	eAutoAnalysisHandler.reorganizeControls(parentRectangle, cameraMode);
	eImageControl.reorganizeControls(parentRectangle, cameraMode);
	eAlerts.reorganizeControls(parentRectangle, cameraMode);
	ePicStats.reorganizeControls(parentRectangle, cameraMode);
	ePictureOptionsControl.reorganizeControls(parentRectangle, cameraMode);
	/// Reorganize all other controls. #
	reorganizeControl(eTempTextDisplayHandle, cameraMode, parentRectangle);
	reorganizeControl(eCurrentTempDisplayHandle, cameraMode, parentRectangle);
	reorganizeControl(eTempOffButton, cameraMode, parentRectangle);
	reorganizeControl(eStatusEditHandle, cameraMode, parentRectangle);
	reorganizeControl(eTriggerTextDisplayHandle, cameraMode, parentRectangle);
	reorganizeControl(eImgLeftSideTextHandle, cameraMode, parentRectangle);
	reorganizeControl(eImageBottomSideTextHandle, cameraMode, parentRectangle);
	reorganizeControl(eImgRightSideTextHandle, cameraMode, parentRectangle);
	reorganizeControl(eImageTopSideTextHandle, cameraMode, parentRectangle);
	reorganizeControl(eVerticalBinningTextHandle, cameraMode, parentRectangle);
	reorganizeControl(eHorizontalBinningTextHandle, cameraMode, parentRectangle);
	reorganizeControl(eKineticCycleTimeTextHandle, cameraMode, parentRectangle);
	reorganizeControl(eTotalPictureNumberTextHandle, cameraMode, parentRectangle);
	reorganizeControl(ePixel1TextDisplay, cameraMode, parentRectangle);
	reorganizeControl(ePixel2TextDisplay, cameraMode, parentRectangle);
	reorganizeControl(eTempDispHandle, cameraMode, parentRectangle);
	reorganizeControl(eImgLeftSideDispHandle, cameraMode, parentRectangle);
	reorganizeControl(eImgRightSideDispHandle, cameraMode, parentRectangle);
	reorganizeControl(eHorizontalBinningDispHandle, cameraMode, parentRectangle);
	reorganizeControl(eImageBottomSideDispHandle, cameraMode, parentRectangle);
	reorganizeControl(eImageTopSideDispHandle, cameraMode, parentRectangle);
	reorganizeControl(eVerticalBinningDispHandle, cameraMode, parentRectangle);
	reorganizeControl(eKineticCycleTimeDispHandle, cameraMode, parentRectangle);
	reorganizeControl(eRepetitionsPerVariationDisp, cameraMode, parentRectangle);
	reorganizeControl(eCurrentAccumulationNumDispHandle, cameraMode, parentRectangle);
	reorganizeControl(eMinCountDispHandle, cameraMode, parentRectangle);
	reorganizeControl(eMaxCountDispHandle, cameraMode, parentRectangle);
	reorganizeControl(eVariationNumberDisp, cameraMode, parentRectangle);
	reorganizeControl(eErrorEditHandle, cameraMode, parentRectangle);
	reorganizeControl(eTempEditHandle, cameraMode, parentRectangle);
	reorganizeControl(eImgLeftSideEditHandle, cameraMode, parentRectangle);
	reorganizeControl(eImageTopEditHandle, cameraMode, parentRectangle);
	reorganizeControl(eImgRightSideEditHandle, cameraMode, parentRectangle);
	reorganizeControl(eImageBottomEditHandle, cameraMode, parentRectangle);
	reorganizeControl(eVerticalBinningEditHandle, cameraMode, parentRectangle);
	reorganizeControl(eHorizontalBinningEditHandle, cameraMode, parentRectangle);
	reorganizeControl(eKineticCycleTimeEditHandle, cameraMode, parentRectangle);
	reorganizeControl(eRepetitionsPerVariationEdit, cameraMode, parentRectangle);
	reorganizeControl(ePixel1XEditHandle, cameraMode, parentRectangle);
	reorganizeControl(ePixel2XEditHandle, cameraMode, parentRectangle);
	reorganizeControl(ePixel1YEditHandle, cameraMode, parentRectangle);
	reorganizeControl(ePixel2YEditHandle, cameraMode, parentRectangle);
	reorganizeControl(eVariationNumberEdit, cameraMode, parentRectangle);
	reorganizeControl(eIncDataFileOptionBoxHandle, cameraMode, parentRectangle);
	reorganizeControl(eSetTemperatureButtonHandle, cameraMode, parentRectangle);
	reorganizeControl(eSetImageParametersButtonHandle, cameraMode, parentRectangle);
	reorganizeControl(eSetAnalysisPixelsButtonHandle, cameraMode, parentRectangle);
	reorganizeControl(eSetKineticSeriesCycleTimeButtonHandle, cameraMode, parentRectangle);
	reorganizeControl(eSetNumberRepetitionsPerVariationButton, cameraMode, parentRectangle);
	reorganizeControl(ePlotAverageCountsBoxHandle, cameraMode, parentRectangle);
	reorganizeControl(eSeVariationNumberButton, cameraMode, parentRectangle);
	reorganizeControl(eClearStatusButtonHandle, cameraMode, parentRectangle);
	reorganizeControl(eTriggerComboHandle, cameraMode, parentRectangle);
	reorganizeControl(eCameraModeComboHandle, cameraMode, parentRectangle);
	reorganizeControl(eAccumulationTimeDisp, cameraMode, parentRectangle);
	reorganizeControl(eAccumulationTimeEdit, cameraMode, parentRectangle);
	reorganizeControl(eSetAccumulationTimeButton, cameraMode, parentRectangle);
	reorganizeControl(eSetAccumulationNumberButton, cameraMode, parentRectangle);
	reorganizeControl(eAccumulationNumberEdit, cameraMode, parentRectangle);
	reorganizeControl(eSetAccumulationNumberDisp, cameraMode, parentRectangle);
	reorganizeControl(eRunningAverageBox, cameraMode, parentRectangle);
	reorganizeControl(eRunningAverageEdit, cameraMode, parentRectangle);
	reorganizeControl(eRunningAverageDisp, cameraMode, parentRectangle);
	reorganizeControl(eSetRunningAverageNumberToAverageButton, cameraMode, parentRectangle);

	reorganizeControl(eSetPlottingFrequencyButton, cameraMode, parentRectangle);
	reorganizeControl(ePlottingFrequencyDisp, cameraMode, parentRectangle);
	reorganizeControl(ePlottingFrequencyEdit, cameraMode, parentRectangle);
	reorganizeControl(eAllPlotsCombo, cameraMode, parentRectangle);
	reorganizeControl(eAllPlotsText, cameraMode, parentRectangle);
	reorganizeControl(eCurrentPlotsCombo, cameraMode, parentRectangle);
	reorganizeControl(eCurrentPlotsText, cameraMode, parentRectangle);

	reorganizeControl(eSeriesProgressBar, cameraMode, parentRectangle);
	reorganizeControl(eSubSeriesProgressBar, cameraMode, parentRectangle);

	reorganizeControl(eMaximumPictureSlider1, cameraMode, parentRectangle);
	reorganizeControl(eMinimumPictureSlider1, cameraMode, parentRectangle);
	reorganizeControl(eMaxSliderNumberEdit1, cameraMode, parentRectangle);
	reorganizeControl(eMinSliderNumberEdit1, cameraMode, parentRectangle);
	reorganizeControl(eMinSliderText1, cameraMode, parentRectangle);
	reorganizeControl(eMaxSliderText1, cameraMode, parentRectangle);
	reorganizeControl(eMaximumPictureSlider2, cameraMode, parentRectangle);
	reorganizeControl(eMinimumPictureSlider2, cameraMode, parentRectangle);
	reorganizeControl(eMaxSliderNumberEdit2, cameraMode, parentRectangle);
	reorganizeControl(eMinSliderNumberEdit2, cameraMode, parentRectangle);
	reorganizeControl(eMinSliderText2, cameraMode, parentRectangle);
	reorganizeControl(eMaxSliderText2, cameraMode, parentRectangle);
	reorganizeControl(eMaximumPictureSlider3, cameraMode, parentRectangle);
	reorganizeControl(eMinimumPictureSlider3, cameraMode, parentRectangle);
	reorganizeControl(eMaxSliderNumberEdit3, cameraMode, parentRectangle);
	reorganizeControl(eMinSliderNumberEdit3, cameraMode, parentRectangle);
	reorganizeControl(eMinSliderText3, cameraMode, parentRectangle);
	reorganizeControl(eMaxSliderText3, cameraMode, parentRectangle);
	reorganizeControl(eMaximumPictureSlider4, cameraMode, parentRectangle);
	reorganizeControl(eMinimumPictureSlider4, cameraMode, parentRectangle);
	reorganizeControl(eMaxSliderNumberEdit4, cameraMode, parentRectangle);
	reorganizeControl(eMinSliderNumberEdit4, cameraMode, parentRectangle);
	reorganizeControl(eMinSliderText4, cameraMode, parentRectangle);
	reorganizeControl(eMaxSliderText4, cameraMode, parentRectangle);
	
	reorganizeControl(eEMGainDisplay, cameraMode, parentRectangle);
	reorganizeControl(eEMGainEdit, cameraMode, parentRectangle);
	reorganizeControl(eEMGainText, cameraMode, parentRectangle);
	reorganizeControl(eSetEMGain, cameraMode, parentRectangle);

	reorganizeControl(eStatusText, cameraMode, parentRectangle);
	reorganizeControl(eErrorText, cameraMode, parentRectangle);
	reorganizeControl(eErrorClear, cameraMode, parentRectangle);
	reorganizeControl(eEMGainEdit, cameraMode, parentRectangle);
	
	// resize the drawing areas
	double widthScale = (parentRectangle.right - parentRectangle.left) / 1936.0;
	double heightScale = (parentRectangle.bottom - parentRectangle.top) / 1056.0;
	eImageBackgroundAreas[0].left = 757 * widthScale;
	eImageBackgroundAreas[0].top = 5 * heightScale;
	eImageBackgroundAreas[0].right = 1227 * widthScale;
	eImageBackgroundAreas[0].bottom = 475 * heightScale;
	eImageBackgroundAreas[1].left = 1341 * widthScale;
	eImageBackgroundAreas[1].top = 5 * heightScale;
	eImageBackgroundAreas[1].right = 1831 * widthScale;
	eImageBackgroundAreas[1].bottom = 475 * heightScale;
	eImageBackgroundAreas[2].left = 757 * widthScale;
	eImageBackgroundAreas[2].top = 525 * heightScale;
	eImageBackgroundAreas[2].right = 1227 * widthScale;
	eImageBackgroundAreas[2].bottom = 995 * heightScale;
	eImageBackgroundAreas[3].left = 1341 * widthScale;
	eImageBackgroundAreas[3].top = 525 * heightScale;
	eImageBackgroundAreas[3].right = 1831 * widthScale;
	eImageBackgroundAreas[3].bottom = 995 * heightScale;
	return 0;
}
