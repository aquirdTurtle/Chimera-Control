#include "stdafx.h"
#include "initializeCameraWindow.h"
#include "Windows.h"
#include "externals.h"
#include "constants.h"
#include "fonts.h"
#include "CreateIdentityPalette.h"
#include "cameraThread.h"
#include "fileManage.h"
#include "Commctrl.h"
#include "Uxtheme.h"
#include "reorganizeWindow.h"

bool initializeCameraWindow(HWND cameraWindow)
{
	/// General format of each of these commands: ////////////////////
	// Set position rectangles for control X using the different rect members of the control. {-1-1-1-1} means that the control doesn't appear in this mode.
	// set initial position to kinetic series mode position
	// create window, putting it into the hwnd member of control X
	// set the font of control X using the .font member
	// complete any extra tasks (e.g. adding items to a combobox).
	/// //////////////////////////////////////////////////////////////
	// These indicators make it easy to add or re-arrange controls that are all in a collumn. After a row of controls these are incremented by the size of that
	// row for the given mode.
	POINT collumnPosKin = { 0,0 }, collumnPosAccum = { 0,0 }, collumnPosCont = { 0, 0};

	///  /// Initialize all controls and HWNDs for this window. ///  ///
	InitCommonControlsEx(0);
	RECT initPos;
	/// Filesystem Controls
	eCameraFileSystem.initializeControls(collumnPosKin, collumnPosAccum, collumnPosCont, cameraWindow, false);
	

	POINT secondColumnPointKinetic = { 480, 150 }, secondColumnPointAccumulate = { 480, 150 }, secondColumnPointContinuous = { 480, 150 };
	eTextingHandler.initializeControls(secondColumnPointKinetic, secondColumnPointAccumulate, secondColumnPointContinuous, cameraWindow, false);
	eAutoAnalysisHandler.initializeControls(secondColumnPointKinetic, secondColumnPointAccumulate, secondColumnPointContinuous, cameraWindow, false);

	/// CAMERA MODE
	eCameraModeComboHandle.kineticSeriesModePos = { 0, collumnPosKin.y, 480, collumnPosKin.y + 800 };
	eCameraModeComboHandle.continuousSingleScansModePos = eCameraModeComboHandle.accumulateModePos = eCameraModeComboHandle.kineticSeriesModePos;
	initPos = eCameraModeComboHandle.kineticSeriesModePos;
	eCameraModeComboHandle.hwnd = CreateWindowEx(0, "COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_CAMERA_MODE_COMBO, eHInst, NULL);
	// add options
	eCameraModeComboHandle.fontType = "Normal";
	SendMessage(eCameraModeComboHandle.hwnd, CB_ADDSTRING, 0, (LPARAM)"Kinetic Series Mode");
	SendMessage(eCameraModeComboHandle.hwnd, CB_ADDSTRING, 0, (LPARAM)"Accumulate Mode");
	SendMessage(eCameraModeComboHandle.hwnd, CB_ADDSTRING, 0, (LPARAM)"Continuous Single Scans Mode");
	// select default
	SendMessage(eCameraModeComboHandle.hwnd, CB_SELECTSTRING, 0, (LPARAM)"Kinetic Series Mode");
	collumnPosKin.y += 30;
	collumnPosAccum.y += 30;
	collumnPosCont.y += 30;
	/// EM GAIN
	
	eSetEMGain.kineticSeriesModePos = { 0, collumnPosKin.y, 120, collumnPosKin.y + 20 };
	eSetEMGain.continuousSingleScansModePos = eSetEMGain.accumulateModePos = eSetEMGain.kineticSeriesModePos;
	initPos = eSetEMGain.kineticSeriesModePos;
	eSetEMGain.hwnd = CreateWindowEx(0, "BUTTON", "Set EM Gain",
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow,
		(HMENU)IDC_SET_EM_GAIN_MODE, eHInst, NULL);
	eSetEMGain.fontType = "Normal";

	eEMGainText.kineticSeriesModePos = { 120, collumnPosKin.y, 280, collumnPosKin.y + 20 };
	eEMGainText.continuousSingleScansModePos = eEMGainText.accumulateModePos = eEMGainText.kineticSeriesModePos;
	initPos = eEMGainText.kineticSeriesModePos;
	eEMGainText.hwnd = CreateWindowEx(0, "STATIC", "EM Gain Setting:",
		WS_CHILD | WS_VISIBLE | BS_RIGHT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow,
		(HMENU)-1, eHInst, NULL);
	eEMGainText.fontType = "Normal";
	//
	eEMGainEdit.kineticSeriesModePos = { 280, collumnPosKin.y, 380, collumnPosKin.y + 20 };
	eEMGainEdit.continuousSingleScansModePos = eEMGainEdit.accumulateModePos = eEMGainEdit.kineticSeriesModePos;
	initPos = eEMGainEdit.kineticSeriesModePos;
	eEMGainEdit.hwnd = CreateWindowEx(0, "EDIT", "",
		WS_CHILD | WS_VISIBLE | BS_RIGHT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow,
		(HMENU)IDC_EM_GAIN_MODE_EDIT, eHInst, NULL);
	eEMGainText.fontType = "Normal";
	
	eEMGainDisplay.kineticSeriesModePos = { 380, collumnPosKin.y, 480, collumnPosKin.y + 20 };
	eEMGainDisplay.continuousSingleScansModePos = eEMGainDisplay.accumulateModePos = eEMGainDisplay.kineticSeriesModePos;
	initPos = eEMGainDisplay.kineticSeriesModePos;
	eEMGainDisplay.hwnd = CreateWindowEx(0, "EDIT", "OFF",
		WS_CHILD | WS_VISIBLE | BS_RIGHT | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow,
		(HMENU)IDC_EM_GAIN_MODE_DISP, eHInst, NULL);
	eEMGainDisplay.fontType = "Normal";
	eEMGainMode = false;

	collumnPosKin.y += 25;
	collumnPosAccum.y += 25;
	collumnPosCont.y += 25;
	/// PICTURES PER EXPERIMENT
	// Set Pictures Per Repetition Button
	eSetPicturesPerRepetitionButtonHandle.kineticSeriesModePos = { 0, collumnPosKin.y, 220, collumnPosKin.y + 25 };
	eSetPicturesPerRepetitionButtonHandle.continuousSingleScansModePos = { 0, collumnPosCont.y, 220, collumnPosCont.y + 25 };
	eSetPicturesPerRepetitionButtonHandle.accumulateModePos = { -1, -1, -1, -1 };
	initPos = eSetPicturesPerRepetitionButtonHandle.kineticSeriesModePos;
	eSetPicturesPerRepetitionButtonHandle.hwnd = CreateWindowEx(0, "BUTTON", "Set Pic.s / Repetition",
		WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_SET_PICTURES_PER_REPITITION_BUTTON, eHInst, NULL);
	eSetPicturesPerRepetitionButtonHandle.fontType = "Normal";
	// Pictures Per Repetition Edit
	ePicturesPerRepetitionEdit.kineticSeriesModePos = { 225, collumnPosKin.y, 320, collumnPosKin.y + 25 };
	ePicturesPerRepetitionEdit.continuousSingleScansModePos = { 225, collumnPosCont.y, 320, collumnPosCont.y + 25 };
	ePicturesPerRepetitionEdit.accumulateModePos = { -1, -1, -1, -1 };
	initPos = ePicturesPerRepetitionEdit.kineticSeriesModePos;
	ePicturesPerRepetitionEdit.hwnd = CreateWindowEx(0, "EDIT", "2",
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_PICTURES_PER_REPETITION_EDIT, eHInst, NULL);
	ePicturesPerRepetitionEdit.fontType = "Normal";
	// Pictures Per Repetition Display
	ePicturesPerRepetitionDisp.kineticSeriesModePos = { 325, collumnPosKin.y, 480, collumnPosKin.y + 25 };
	ePicturesPerRepetitionDisp.continuousSingleScansModePos = { 325, collumnPosCont.y, 480, collumnPosCont.y + 25 };
	ePicturesPerRepetitionDisp.accumulateModePos = { -1, -1, -1, -1 };
	initPos = ePicturesPerRepetitionDisp.kineticSeriesModePos;
	ePicturesPerRepetitionDisp.hwnd = CreateWindowEx(0, "EDIT", "",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_PICTURES_PER_REPETITION_DISP, eHInst, NULL);
	ePicturesPerRepetitionDisp.fontType = "Normal";
	collumnPosKin.y += 30;
	collumnPosCont.y += 30;

	/// REPETITIONS PER VARIATION
	// Set Repetitions Per Variation Button
	eSetNumberRepetitionsPerVariationButton.kineticSeriesModePos = { 0, collumnPosKin.y, 220, collumnPosKin.y + 25 };
	eSetNumberRepetitionsPerVariationButton.continuousSingleScansModePos = { -1,-1,-1,-1 };
	eSetNumberRepetitionsPerVariationButton.accumulateModePos = { -1,-1,-1,-1 };
	initPos = eSetNumberRepetitionsPerVariationButton.kineticSeriesModePos;
	eSetNumberRepetitionsPerVariationButton.hwnd = CreateWindowEx(0, "BUTTON", "Set Rep. / Variation",
		WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_SET_REPETITONS_PER_VARIATION_BUTTON, eHInst, NULL);
	eSetNumberRepetitionsPerVariationButton.fontType = "Normal";
	// Repetitions Per Variation Edit
	eRepetitionsPerVariationEdit.kineticSeriesModePos = { 225, collumnPosKin.y, 320, collumnPosKin.y + 25 };
	eRepetitionsPerVariationEdit.continuousSingleScansModePos = { -1,-1,-1,-1 };
	eRepetitionsPerVariationEdit.accumulateModePos = { -1,-1,-1,-1 };
	initPos = eRepetitionsPerVariationEdit.kineticSeriesModePos;
	eRepetitionsPerVariationEdit.hwnd = CreateWindowEx(0, "EDIT", "10",
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_REPETITIONS_PER_VARIATION_EDIT, eHInst, NULL);
	eRepetitionsPerVariationEdit.fontType = "Normal";
	// Repetitions Per Variation Display
	eRepetitionsPerVariationDisp.kineticSeriesModePos = { 325, collumnPosKin.y, 480, collumnPosKin.y + 25 };
	eRepetitionsPerVariationDisp.continuousSingleScansModePos = { -1,-1,-1,-1 };
	eRepetitionsPerVariationDisp.accumulateModePos = { -1,-1,-1,-1 };
	initPos = eRepetitionsPerVariationDisp.kineticSeriesModePos;
	eRepetitionsPerVariationDisp.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_REPETITIONS_PER_VARIATION_DISP, eHInst, NULL);
	eRepetitionsPerVariationDisp.fontType = "Normal";

	collumnPosKin.y += 30;

	/// VARIATION NUMBER
	// Set Variation Number Button
	eSeVariationNumberButton.kineticSeriesModePos = { 0, collumnPosKin.y, 220, collumnPosKin.y + 25 };
	eSeVariationNumberButton.continuousSingleScansModePos = { -1,-1,-1,-1 };
	eSeVariationNumberButton.accumulateModePos = { -1,-1,-1,-1 };
	initPos = eSeVariationNumberButton.kineticSeriesModePos;
	eSeVariationNumberButton.hwnd = CreateWindowEx(0, "BUTTON", "Set # Variations",
		WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_SET_VARIATION_NUMBER, eHInst, NULL);
	eSeVariationNumberButton.fontType = "Normal";
	// Variation # Edit
	eVariationNumberEdit.kineticSeriesModePos = { 225, collumnPosKin.y, 320, collumnPosKin.y + 25 };
	eVariationNumberEdit.continuousSingleScansModePos = { -1,-1,-1,-1 };
	eVariationNumberEdit.accumulateModePos = { -1,-1,-1,-1 };
	initPos = eVariationNumberEdit.kineticSeriesModePos;
	eVariationNumberEdit.hwnd = CreateWindowEx(0, "EDIT", "1",
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_VARIATION_EDIT, eHInst, NULL);
	eVariationNumberEdit.fontType = "Normal";
	// Variation Number Display
	eVariationNumberDisp.kineticSeriesModePos = { 325, collumnPosKin.y, 480, collumnPosKin.y + 25 };
	eVariationNumberDisp.continuousSingleScansModePos = { -1,-1,-1,-1 };
	eVariationNumberDisp.accumulateModePos = { -1,-1,-1,-1 };
	initPos = eVariationNumberDisp.kineticSeriesModePos;
	eVariationNumberDisp.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_VARIATION_DISP_HANDLE, eHInst, NULL);
	eVariationNumberDisp.fontType = "Normal";
	collumnPosKin.y += 30;

	/// EXPOSURE
	// Set Exposure Button
	eSetExposureButtonHandle.kineticSeriesModePos = { 0, collumnPosKin.y, 480, collumnPosKin.y + 25 };
	eSetExposureButtonHandle.continuousSingleScansModePos = { 0, collumnPosCont.y, 480, collumnPosCont.y + 25 }; 
	eSetExposureButtonHandle.accumulateModePos = { 0, collumnPosAccum.y, 480, collumnPosAccum.y + 25 };
	initPos = eSetExposureButtonHandle.kineticSeriesModePos;
	eSetExposureButtonHandle.hwnd = CreateWindowEx(0, "BUTTON", "Set Exposure Time (ms)",
											 WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
											initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, 
											cameraWindow, (HMENU)IDC_SET_EXPOSURE_BUTTON, eHInst, NULL);
	eSetExposureButtonHandle.fontType = "Normal";
	collumnPosKin.y += 25;
	collumnPosAccum.y += 25;
	collumnPosCont.y += 25;
	// Exposure Edits
	// #1
	eExposure1EditHandle.kineticSeriesModePos = { 0, collumnPosKin.y, 120, collumnPosKin.y + 25 };
	eExposure1EditHandle.continuousSingleScansModePos = { 0, collumnPosCont.y, 120, collumnPosCont.y + 25 };
	eExposure1EditHandle.accumulateModePos = { 0, collumnPosAccum.y, 120, collumnPosAccum.y + 25 };
	initPos = eExposure1EditHandle.kineticSeriesModePos;
	eExposure1EditHandle.hwnd = CreateWindowEx(0, "EDIT", "26",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_EXPOSURE_1_EDIT, eHInst, NULL);
	eExposure1EditHandle.fontType = "Normal";
	// #2
	eExposure2EditHandle.kineticSeriesModePos = { 120, collumnPosKin.y, 240, collumnPosKin.y + 25 };
	eExposure2EditHandle.continuousSingleScansModePos = { 120, collumnPosCont.y, 240, collumnPosCont.y + 25 };
	eExposure2EditHandle.accumulateModePos = { 120, collumnPosAccum.y, 240, collumnPosAccum.y + 25 };
	initPos = eExposure2EditHandle.kineticSeriesModePos;
	eExposure2EditHandle.hwnd = CreateWindowEx(0, "EDIT", "-1",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_EXPOSURE_2_EDIT, eHInst, NULL);
	eExposure2EditHandle.fontType = "Normal";
	// #3
	eExposure3EditHandle.kineticSeriesModePos = { 240, collumnPosKin.y, 360, collumnPosKin.y + 25 };
	eExposure3EditHandle.continuousSingleScansModePos = { 240, collumnPosCont.y, 360, collumnPosCont.y + 25 };
	eExposure3EditHandle.accumulateModePos = { 240, collumnPosAccum.y, 360, collumnPosAccum.y + 25 };
	initPos = eExposure3EditHandle.kineticSeriesModePos;
	eExposure3EditHandle.hwnd = CreateWindowEx(0, "EDIT", "-1",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_EXPOSURE_3_EDIT, eHInst, NULL);
	eExposure3EditHandle.fontType = "Normal";
	// #4
	eExposure4EditHandle.kineticSeriesModePos = { 360, collumnPosKin.y, 480, collumnPosKin.y + 25 };
	eExposure4EditHandle.continuousSingleScansModePos = { 360, collumnPosCont.y, 480, collumnPosCont.y + 25 };
	eExposure4EditHandle.accumulateModePos = { 360, collumnPosAccum.y, 480, collumnPosAccum.y + 25 };
	initPos = eExposure4EditHandle.kineticSeriesModePos;
	eExposure4EditHandle.hwnd = CreateWindowEx(0, "EDIT", "-1",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_EXPOSURE_4_EDIT, eHInst, NULL);
	eExposure4EditHandle.fontType = "Normal";
	collumnPosKin.y += 25;
	collumnPosAccum.y += 25;
	collumnPosCont.y += 25;
	// Exposure Display
	eExposureDispHandle.kineticSeriesModePos = { 0, collumnPosKin.y, 480, collumnPosKin.y + 25 };
	eExposureDispHandle.accumulateModePos = { 0, collumnPosAccum.y, 480, collumnPosAccum.y + 25 };
	eExposureDispHandle.continuousSingleScansModePos = { 0, collumnPosCont.y, 480, collumnPosCont.y + 25 };
	initPos = eExposureDispHandle.kineticSeriesModePos;
	eExposureDispHandle.hwnd = CreateWindowEx(0, "EDIT", "",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_EXPOSURE_DISP, eHInst, NULL);
	eExposureDispHandle.fontType = "Normal";
	collumnPosKin.y += 30;
	collumnPosAccum.y += 30;
	collumnPosCont.y += 30;

	/// TRIGGER
	// Trigger Text

	eTriggerTextDisplayHandle.kineticSeriesModePos = { 0, collumnPosKin.y, 240, collumnPosKin.y + 25 };
	eTriggerTextDisplayHandle.continuousSingleScansModePos = { 0, collumnPosCont.y, 240, collumnPosCont.y + 25 };
	eTriggerTextDisplayHandle.accumulateModePos = { 0, collumnPosAccum.y, 240, collumnPosAccum.y + 25 };
	initPos = eTriggerTextDisplayHandle.kineticSeriesModePos;
	eTriggerTextDisplayHandle.hwnd = CreateWindowEx(0, "STATIC", "Trigger Mode:", WS_CHILD | WS_VISIBLE | SS_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_TRIGGER_DISP, eHInst, NULL);
	eTriggerTextDisplayHandle.fontType = "Normal";
	// trigger combo
	eTriggerComboHandle.kineticSeriesModePos = { 245, collumnPosKin.y, 420, collumnPosKin.y + 800 };
	eTriggerComboHandle.continuousSingleScansModePos = { 245, collumnPosCont.y, 420, collumnPosCont.y + 800 };
	eTriggerComboHandle.accumulateModePos = { 245, collumnPosAccum.y, 420, collumnPosAccum.y + 800 };
	initPos = eTriggerComboHandle.kineticSeriesModePos;
	eTriggerComboHandle.hwnd = CreateWindowEx(0, "COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_TRIGGER_COMBO, eHInst, NULL);
	// set options for the combo
	eTriggerComboHandle.fontType = "Normal";
	SendMessage(eTriggerComboHandle.hwnd, CB_ADDSTRING, 0, (LPARAM)"Internal");
	SendMessage(eTriggerComboHandle.hwnd, CB_ADDSTRING, 0, (LPARAM)"External");
	SendMessage(eTriggerComboHandle.hwnd, CB_ADDSTRING, 0, (LPARAM)"Start On Trigger");
	// Select default trigger
	SendMessage(eTriggerComboHandle.hwnd, CB_SELECTSTRING, 0, (LPARAM)"External");
	collumnPosKin.y += 30;
	collumnPosAccum.y += 30;
	collumnPosCont.y += 30;
	eCurrentTriggerMode = "External";

	/// IMAGE PARAMETERS
	// Set Image Parameters Button
	eSetImageParametersButtonHandle.kineticSeriesModePos = { 0, collumnPosKin.y, 420, collumnPosKin.y + 25 };
	eSetImageParametersButtonHandle.continuousSingleScansModePos = { 0, collumnPosCont.y, 420, collumnPosCont.y + 25 };
	eSetImageParametersButtonHandle.accumulateModePos = { 0, collumnPosAccum.y, 420, collumnPosAccum.y + 25 };
	initPos = eSetImageParametersButtonHandle.kineticSeriesModePos;
	eSetImageParametersButtonHandle.hwnd = CreateWindowEx(0, "BUTTON", "Set Camera Image Dimensions and Binning",
													WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_SET_IMAGE_PARAMS_BUTTON, eHInst, NULL);
	eSetImageParametersButtonHandle.fontType = "Normal";
	collumnPosKin.y += 30;
	collumnPosAccum.y += 30;
	collumnPosCont.y += 30;

	/// Image Horizontal Texts
	// Image Left Side Text
	eImgLeftSideTextHandle.kineticSeriesModePos = { 0, collumnPosKin.y, 140, collumnPosKin.y + 20 };
	eImgLeftSideTextHandle.continuousSingleScansModePos = { 0, collumnPosCont.y, 140, collumnPosCont.y + 20 };
	eImgLeftSideTextHandle.accumulateModePos = { 0, collumnPosAccum.y, 140, collumnPosAccum.y + 20 };
	initPos = eImgLeftSideTextHandle.kineticSeriesModePos;
	eImgLeftSideTextHandle.hwnd = CreateWindowEx(0, "STATIC", "Leftmost", WS_CHILD | WS_VISIBLE | SS_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_IMG_LEFT_TEXT, eHInst, NULL);
	eImgLeftSideTextHandle.fontType = "Normal";
	// Image Right Side Text
	eImgRightSideTextHandle.kineticSeriesModePos = { 140, collumnPosKin.y, 280, collumnPosKin.y + 20 };
	eImgRightSideTextHandle.continuousSingleScansModePos = { 140, collumnPosCont.y, 280, collumnPosCont.y + 20 };
	eImgRightSideTextHandle.accumulateModePos = { 140, collumnPosAccum.y, 280, collumnPosAccum.y + 20 };
	initPos = eImgRightSideTextHandle.kineticSeriesModePos;
	eImgRightSideTextHandle.hwnd = CreateWindowEx(0, "STATIC", "Rightmost", WS_CHILD | WS_VISIBLE | SS_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_IMG_RIGHT_TEXT, eHInst, NULL);
	eImgRightSideTextHandle.fontType = "Normal";
	// Horizontal Binning Text
	eHorizontalBinningTextHandle.kineticSeriesModePos = { 280, collumnPosKin.y, 420, collumnPosKin.y + 20 };
	eHorizontalBinningTextHandle.continuousSingleScansModePos = { 280, collumnPosCont.y, 420, collumnPosCont.y + 20 };
	eHorizontalBinningTextHandle.accumulateModePos = { 280, collumnPosAccum.y, 420, collumnPosAccum.y + 20 };
	initPos = eHorizontalBinningTextHandle.kineticSeriesModePos;
	eHorizontalBinningTextHandle.hwnd = CreateWindowEx(0, "STATIC", "Horizontal Bin Size", WS_CHILD | WS_VISIBLE | SS_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_HOR_BIN_TEXT, eHInst, NULL);
	eHorizontalBinningTextHandle.fontType = "Normal";
	collumnPosKin.y += 25;
	collumnPosAccum.y += 25;
	collumnPosCont.y += 25;

	/// Image Horizontal Edits
	// Image Left Side Edit
	eImgLeftSideEditHandle.kineticSeriesModePos = { 0, collumnPosKin.y, 140, collumnPosKin.y + 20 };
	eImgLeftSideEditHandle.continuousSingleScansModePos = { 0, collumnPosCont.y, 140, collumnPosCont.y + 20 };
	eImgLeftSideEditHandle.accumulateModePos = { 0, collumnPosAccum.y, 140, collumnPosAccum.y + 20 };
	initPos = eImgLeftSideEditHandle.kineticSeriesModePos;
	eImgLeftSideEditHandle.hwnd = CreateWindowEx(0, "EDIT", "1",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_IMG_LEFT_EDIT, eHInst, NULL);
	eImgLeftSideEditHandle.fontType = "Normal";
	// Image Right Side Edit
	eImgRightSideEditHandle.kineticSeriesModePos = { 140, collumnPosKin.y, 280, collumnPosKin.y + 20 };
	eImgRightSideEditHandle.continuousSingleScansModePos = { 140, collumnPosCont.y, 280, collumnPosCont.y + 20 };
	eImgRightSideEditHandle.accumulateModePos = { 140, collumnPosAccum.y, 280, collumnPosAccum.y + 20 };
	initPos = eImgRightSideEditHandle.kineticSeriesModePos;
	eImgRightSideEditHandle.hwnd = CreateWindowEx(0, "EDIT", "5",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_IMG_RIGHT_EDIT, eHInst, NULL);
	eImgRightSideEditHandle.fontType = "Normal";
	// Horizontal Binning Edit
	eHorizontalBinningEditHandle.kineticSeriesModePos = { 280, collumnPosKin.y, 420, collumnPosKin.y + 20 };
	eHorizontalBinningEditHandle.continuousSingleScansModePos = { 280, collumnPosCont.y, 420, collumnPosCont.y + 20 };
	eHorizontalBinningEditHandle.accumulateModePos = { 280, collumnPosAccum.y, 420, collumnPosAccum.y + 20 };
	initPos = eHorizontalBinningEditHandle.kineticSeriesModePos;
	eHorizontalBinningEditHandle.hwnd = CreateWindowEx(0, "EDIT", "1",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_HOR_BIN_EDIT, eHInst, NULL);
	eHorizontalBinningEditHandle.fontType = "Normal";
	collumnPosKin.y += 25;
	collumnPosAccum.y += 25;
	collumnPosCont.y += 25;
	/// Image Horizontal Displays
	// Image Left Side Display
	eImgLeftSideDispHandle.kineticSeriesModePos = { 0, collumnPosKin.y, 140, collumnPosKin.y + 20 };
	eImgLeftSideDispHandle.continuousSingleScansModePos = { 0, collumnPosCont.y, 140, collumnPosCont.y + 20 };
	eImgLeftSideDispHandle.accumulateModePos = { 0, collumnPosAccum.y, 140, collumnPosAccum.y + 20 };
	initPos = eImgLeftSideDispHandle.kineticSeriesModePos;
	eImgLeftSideDispHandle.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_IMG_LEFT_DISP, eHInst, NULL);
	eImgLeftSideDispHandle.fontType = "Normal";
	// Image Right Side Display
	eImgRightSideDispHandle.kineticSeriesModePos = { 140, collumnPosKin.y, 280, collumnPosKin.y + 20 };
	eImgRightSideDispHandle.continuousSingleScansModePos = { 140, collumnPosCont.y, 280, collumnPosCont.y + 20 };
	eImgRightSideDispHandle.accumulateModePos = { 140, collumnPosAccum.y, 280, collumnPosAccum.y + 20 };
	initPos = eImgRightSideDispHandle.kineticSeriesModePos;
	eImgRightSideDispHandle.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_IMG_RIGHT_DISP, eHInst, NULL);
	eImgRightSideDispHandle.fontType = "Normal";
	// Horizontal Binning Display
	eHorizontalBinningDispHandle.kineticSeriesModePos = { 280, collumnPosKin.y, 420, collumnPosKin.y + 20 };
	eHorizontalBinningDispHandle.continuousSingleScansModePos = { 280, collumnPosCont.y, 420, collumnPosCont.y + 20 };
	eHorizontalBinningDispHandle.accumulateModePos = { 280, collumnPosAccum.y, 420, collumnPosAccum.y + 20 };
	initPos = eHorizontalBinningDispHandle.kineticSeriesModePos;
	eHorizontalBinningDispHandle.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_HOR_BIN_DISP, eHInst, NULL);
	eHorizontalBinningDispHandle.fontType = "Normal";
	collumnPosKin.y += 25;
	collumnPosAccum.y += 25;
	collumnPosCont.y += 25;
	/// Image Veritcal Texts
	// Image Top Side Text
	eImageBottomSideTextHandle.kineticSeriesModePos = { 0, collumnPosKin.y, 140, collumnPosKin.y + 20 };
	eImageBottomSideTextHandle.continuousSingleScansModePos = { 0, collumnPosCont.y, 140, collumnPosCont.y + 20 };
	eImageBottomSideTextHandle.accumulateModePos = { 0, collumnPosAccum.y, 140, collumnPosAccum.y + 20 };
	initPos = eImageBottomSideTextHandle.kineticSeriesModePos;
	eImageBottomSideTextHandle.hwnd = CreateWindowEx(0, "STATIC", "Bottommost", WS_CHILD | WS_VISIBLE | SS_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_IMG_TOP_TEXT, eHInst, NULL);
	 eImageBottomSideTextHandle.fontType = "Normal";
	// Image Bottom Side Text
	eImageTopSideTextHandle.kineticSeriesModePos = { 140, collumnPosKin.y, 280, collumnPosKin.y + 20 };
	eImageTopSideTextHandle.continuousSingleScansModePos = { 140, collumnPosCont.y, 280, collumnPosCont.y + 20 };
	eImageTopSideTextHandle.accumulateModePos = { 140, collumnPosAccum.y, 280, collumnPosAccum.y + 20 };
	initPos = eImageTopSideTextHandle.kineticSeriesModePos;
	eImageTopSideTextHandle.hwnd = CreateWindowEx(0, "STATIC", "Topmost", WS_CHILD | WS_VISIBLE | SS_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_IMG_BOTTOM_TEXT, eHInst, NULL);
	eImageTopSideTextHandle.fontType = "Normal";
	// Vertical Binning Text
	eVerticalBinningTextHandle.kineticSeriesModePos = { 280, collumnPosKin.y, 420, collumnPosKin.y + 20 };
	eVerticalBinningTextHandle.continuousSingleScansModePos = { 280, collumnPosCont.y, 420, collumnPosCont.y + 20 };
	eVerticalBinningTextHandle.accumulateModePos = { 280, collumnPosAccum.y, 420, collumnPosAccum.y + 20 };
	initPos = eVerticalBinningTextHandle.kineticSeriesModePos;
	eVerticalBinningTextHandle.hwnd = CreateWindowEx(0, "STATIC", "Vertical Bin Size", WS_CHILD | WS_VISIBLE | SS_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_VERTICAL_BIN_TEXT, eHInst, NULL);
	eVerticalBinningTextHandle.fontType = "Normal";
	collumnPosKin.y += 25;
	collumnPosAccum.y += 25;
	collumnPosCont.y += 25;
	/// Image Vertical Edits
	/// This is reversed top -> bottom... I don't want to think about it.
	// Image Top Side Edit
	eImageTopEditHandle.kineticSeriesModePos = { 0, collumnPosKin.y, 140, collumnPosKin.y + 20 };
	eImageTopEditHandle.continuousSingleScansModePos = { 0, collumnPosCont.y, 140, collumnPosCont.y + 20 };
	eImageTopEditHandle.accumulateModePos = { 0, collumnPosAccum.y, 140, collumnPosAccum.y + 20 };
	initPos = eImageTopEditHandle.kineticSeriesModePos;
	eImageTopEditHandle.hwnd = CreateWindowEx(0, "EDIT", "1",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_IMAGE_TOP_EDIT, eHInst, NULL);
	eImageTopEditHandle.fontType = "Normal";
	// Image Bottom Side Edit
	eImageBottomEditHandle.kineticSeriesModePos = { 140, collumnPosKin.y, 280, collumnPosKin.y + 20 };
	eImageBottomEditHandle.continuousSingleScansModePos = { 140, collumnPosCont.y, 280, collumnPosCont.y + 20 };
	eImageBottomEditHandle.accumulateModePos = { 140, collumnPosAccum.y, 280, collumnPosAccum.y + 20 };
	initPos = eImageBottomEditHandle.kineticSeriesModePos;
	eImageBottomEditHandle.hwnd = CreateWindowEx(0, "EDIT", "5",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_IMAGE_BOTTOM_EDIT, eHInst, NULL);
	eImageBottomEditHandle.fontType = "Normal";
	// Vertical Binning Edit
	eVerticalBinningEditHandle.kineticSeriesModePos = { 280, collumnPosKin.y, 420, collumnPosKin.y + 20 };
	eVerticalBinningEditHandle.continuousSingleScansModePos = { 280, collumnPosCont.y, 420, collumnPosCont.y + 20 };
	eVerticalBinningEditHandle.accumulateModePos = { 280, collumnPosAccum.y, 420, collumnPosAccum.y + 20 };
	initPos = eVerticalBinningEditHandle.kineticSeriesModePos;
	eVerticalBinningEditHandle.hwnd = CreateWindowEx(0, "EDIT", "1",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_VERTICAL_BIN_EDIT, eHInst, NULL);
	eVerticalBinningEditHandle.fontType = "Normal";
	collumnPosKin.y += 25;
	collumnPosAccum.y += 25;
	collumnPosCont.y += 25;
	/// Image Veritcal Displays
	// Image Top Side Display
	eImageBottomSideDispHandle.kineticSeriesModePos = { 0, collumnPosKin.y, 140, collumnPosKin.y + 20 };
	eImageBottomSideDispHandle.continuousSingleScansModePos = { 0, collumnPosCont.y, 140, collumnPosCont.y + 20 };
	eImageBottomSideDispHandle.accumulateModePos = { 0, collumnPosAccum.y, 140, collumnPosAccum.y + 20 };
	initPos = eImageBottomSideDispHandle.kineticSeriesModePos;
	eImageBottomSideDispHandle.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_IMG_TOP_DISP, eHInst, NULL);
	eImageBottomSideDispHandle.fontType = "Normal";
	// Image Bottom Side Display
	eImageTopSideDispHandle.kineticSeriesModePos = { 140, collumnPosKin.y, 280, collumnPosKin.y + 20 };
	eImageTopSideDispHandle.continuousSingleScansModePos = { 140, collumnPosCont.y, 280, collumnPosCont.y + 20 };
	eImageTopSideDispHandle.accumulateModePos = { 140, collumnPosAccum.y, 280, collumnPosAccum.y + 20 };
	initPos = eImageTopSideDispHandle.kineticSeriesModePos;
	eImageTopSideDispHandle.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_IMG_BOTTOM_DISP, eHInst, NULL);
	eImageTopSideDispHandle.fontType = "Normal";
	// Vertical Binning Display
	eVerticalBinningDispHandle.kineticSeriesModePos = { 280, collumnPosKin.y, 420, collumnPosKin.y + 20 };
	eVerticalBinningDispHandle.continuousSingleScansModePos = { 280, collumnPosCont.y, 420, collumnPosCont.y + 20 };
	eVerticalBinningDispHandle.accumulateModePos = { 280, collumnPosAccum.y, 420, collumnPosAccum.y + 20 };
	initPos = eVerticalBinningDispHandle.kineticSeriesModePos;
	eVerticalBinningDispHandle.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_VERTICAL_BIN_DISP, eHInst, NULL);
	eVerticalBinningDispHandle.fontType = "Normal";
	collumnPosKin.y += 25;
	collumnPosAccum.y += 25;
	collumnPosCont.y += 25;

	/// KINETIC CYCLE TIME
	// Set Kinetic Series Cycle Time Buttonexperment
	eSetKineticSeriesCycleTimeButtonHandle.kineticSeriesModePos = { 0, collumnPosKin.y, 220, collumnPosKin.y + 25 };
	eSetKineticSeriesCycleTimeButtonHandle.continuousSingleScansModePos = { -1,-1,-1,-1 };
	eSetKineticSeriesCycleTimeButtonHandle.accumulateModePos = { -1,-1,-1,-1 };
	initPos = eSetKineticSeriesCycleTimeButtonHandle.kineticSeriesModePos;
	eSetKineticSeriesCycleTimeButtonHandle.triggerModeSensitive = -1;
	eSetKineticSeriesCycleTimeButtonHandle.hwnd = CreateWindowEx(0, "BUTTON", "Set Kinetic Cycle Time (ms)",
														   WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_SET_KINETIC_CYCLE_TIME_BUTTON, eHInst, NULL);
	eSetKineticSeriesCycleTimeButtonHandle.fontType = "Normal";
	// Kinetic Cycle Time Edit
	eKineticCycleTimeEditHandle.kineticSeriesModePos = { 225, collumnPosKin.y, 320, collumnPosKin.y + 25 };
	eKineticCycleTimeEditHandle.continuousSingleScansModePos = { -1,-1,-1,-1 };
	eKineticCycleTimeEditHandle.accumulateModePos = { -1,-1,-1,-1 };
	initPos = eKineticCycleTimeEditHandle.kineticSeriesModePos;
	eKineticCycleTimeEditHandle.triggerModeSensitive = -1;
	eKineticCycleTimeEditHandle.hwnd = CreateWindowEx(0, "EDIT", "500",
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_KINETIC_CYCLE_TIME_EDIT, eHInst, NULL);
	eKineticCycleTimeEditHandle.fontType = "Normal";
	// Kinetic Cycle Time Display
	eKineticCycleTimeDispHandle.kineticSeriesModePos = { 325, collumnPosKin.y, 420, collumnPosKin.y + 25 };
	eKineticCycleTimeDispHandle.continuousSingleScansModePos = { -1,-1,-1,-1 };
	eKineticCycleTimeDispHandle.accumulateModePos = { -1,-1,-1,-1 };
	initPos = eKineticCycleTimeDispHandle.kineticSeriesModePos;
	eKineticCycleTimeDispHandle.triggerModeSensitive = -1;
	eKineticCycleTimeDispHandle.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_KINETIC_CYCLE_TIME_DISP, eHInst, NULL);
	eKineticCycleTimeDispHandle.fontType = "Normal";
	collumnPosKin.y += 30;

	/// ACCUMULATION TIME
	// Set Accumulation Time Button
	eSetAccumulationTimeButton.kineticSeriesModePos = { -1, -1, -1, -1 };
	eSetAccumulationTimeButton.continuousSingleScansModePos = { -1, -1, -1, -1 };
	eSetAccumulationTimeButton.accumulateModePos = { 0, collumnPosAccum.y, 220, collumnPosAccum.y + 25 };
	initPos = eSetAccumulationTimeButton.kineticSeriesModePos;
	eSetAccumulationTimeButton.triggerModeSensitive = -1;
	eSetAccumulationTimeButton.hwnd = CreateWindowEx(0, "BUTTON", "Set Accumulation Time",
		WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_SET_ACCUMULATION_TIME_BUTTON, eHInst, NULL);
	ShowWindow(eSetAccumulationTimeButton.hwnd, SW_HIDE);
	eSetAccumulationTimeButton.fontType = "Normal";
	// Accumulation Time Edit
	eAccumulationTimeEdit.kineticSeriesModePos = { -1, -1, -1, -1 };
	eAccumulationTimeEdit.continuousSingleScansModePos = { -1, -1, -1, -1 };
	eAccumulationTimeEdit.accumulateModePos = { 225, collumnPosAccum.y, 320, collumnPosAccum.y + 25 };
	initPos = eAccumulationTimeEdit.kineticSeriesModePos;
	eAccumulationTimeEdit.triggerModeSensitive = -1;
	eAccumulationTimeEdit.hwnd = CreateWindowEx(0, "EDIT", "1",
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_ACCUMULATION_TIME_EDIT, eHInst, NULL);
	ShowWindow(eSetAccumulationTimeButton.hwnd, SW_HIDE);
	eAccumulationTimeEdit.fontType = "Normal";
	// Accumulation Time Display
	eAccumulationTimeDisp.kineticSeriesModePos = { -1, -1, -1, -1 };
	eAccumulationTimeDisp.continuousSingleScansModePos = { -1, -1, -1, -1 };
	eAccumulationTimeDisp.accumulateModePos = { 325, collumnPosAccum.y, 420, collumnPosAccum.y + 25 };
	initPos = eAccumulationTimeDisp.kineticSeriesModePos;
	eAccumulationTimeDisp.triggerModeSensitive = -1;
	eAccumulationTimeDisp.hwnd = CreateWindowEx(0, "EDIT", "",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_ACCUMULATION_TIME_DISP, eHInst, NULL);
	ShowWindow(eAccumulationTimeDisp.hwnd, SW_HIDE);
	eAccumulationTimeDisp.fontType = "Normal";
	collumnPosAccum.y += 30;
	/// ACCUMULATION NUMBER
	// Set Accumulation Number Button
	eSetAccumulationNumberButton.kineticSeriesModePos = { -1, -1, -1, -1 };
	eSetAccumulationNumberButton.continuousSingleScansModePos = { -1, -1, -1, -1 };
	eSetAccumulationNumberButton.accumulateModePos = { 0, collumnPosAccum.y, 220, collumnPosAccum.y + 25 };
	initPos = eSetAccumulationNumberButton.kineticSeriesModePos;
	eSetAccumulationNumberButton.triggerModeSensitive = 1;
	eSetAccumulationNumberButton.hwnd = CreateWindowEx(0, "BUTTON", "Set # of Accumulations",
		WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_SET_ACCUMULATION_NUMBER_BUTTON, eHInst, NULL);
	ShowWindow(eSetAccumulationNumberButton.hwnd, SW_HIDE);
	eSetAccumulationNumberButton.fontType = "Normal";
	// Accumulation Number Edit
	eAccumulationNumberEdit.kineticSeriesModePos = { -1, -1, -1, -1 };
	eAccumulationNumberEdit.continuousSingleScansModePos = { -1, -1, -1, -1 };
	eAccumulationNumberEdit.accumulateModePos = { 225, collumnPosAccum.y, 320, collumnPosAccum.y + 25 };
	initPos = eAccumulationNumberEdit.kineticSeriesModePos;
	eAccumulationNumberEdit.triggerModeSensitive = 1;
	eAccumulationNumberEdit.hwnd = CreateWindowEx(0, "EDIT", "1",
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_ACCUMULATION_NUMBER_EDIT, eHInst, NULL);
	ShowWindow(eAccumulationNumberEdit.hwnd, SW_HIDE);
	eAccumulationNumberEdit.fontType = "Normal";
	// Accumulation Time Display
	eSetAccumulationNumberDisp.kineticSeriesModePos = { -1, -1, -1, -1 };
	eSetAccumulationNumberDisp.continuousSingleScansModePos = { -1, -1, -1, -1 };
	eSetAccumulationNumberDisp.accumulateModePos = { 325, collumnPosAccum.y, 420, collumnPosAccum.y + 25 };
	initPos = eSetAccumulationNumberDisp.kineticSeriesModePos;
	eSetAccumulationNumberDisp.triggerModeSensitive = 1;
	eSetAccumulationNumberDisp.hwnd = CreateWindowEx(0, "EDIT", "",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_ACCUMULATION_NUMBER_DISP, eHInst, NULL);
	ShowWindow(eSetAccumulationNumberDisp.hwnd, SW_HIDE);
	eSetAccumulationNumberDisp.fontType = "Normal";
	collumnPosAccum.y += 30;

	/// ATOM THRESHOLD
	// Set Atom Threshold Button
	eSetAtomThresholdButtonHandle.kineticSeriesModePos = { 0, collumnPosKin.y, 220, collumnPosKin.y + 25 };
	eSetAtomThresholdButtonHandle.continuousSingleScansModePos = { 0, collumnPosCont.y, 220, collumnPosCont.y + 25 };
	eSetAtomThresholdButtonHandle.accumulateModePos = { 0, collumnPosAccum.y, 220, collumnPosAccum.y + 25 };
	initPos = eSetAtomThresholdButtonHandle.kineticSeriesModePos;
	eSetAtomThresholdButtonHandle.triggerModeSensitive = 1;
	eSetAtomThresholdButtonHandle.hwnd = CreateWindowEx(0, "BUTTON", "Set Atom Threshold",
													   WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_SET_DETECTION_THRESHOLD_BUTTON, eHInst, NULL);
	eSetAtomThresholdButtonHandle.fontType = "Normal";
	// Atom Threshold Edit
	eAtomThresholdEditHandle.kineticSeriesModePos = { 225, collumnPosKin.y, 320, collumnPosKin.y + 25 };
	eAtomThresholdEditHandle.continuousSingleScansModePos = { 225, collumnPosCont.y, 320, collumnPosCont.y + 25 };
	eAtomThresholdEditHandle.accumulateModePos = { 225, collumnPosAccum.y, 320, collumnPosAccum.y + 25 };
	initPos = eAtomThresholdEditHandle.kineticSeriesModePos;
	eAtomThresholdEditHandle.triggerModeSensitive = 1;
	eAtomThresholdEditHandle.hwnd = CreateWindowEx(0, "EDIT", "100",
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_DETECTION_THRESHOLD_EDIT_HANDLE, eHInst, NULL);
	eAtomThresholdEditHandle.fontType = "Normal";
	// Atom Threshold Display
	eAtomThresholdDispHandle.kineticSeriesModePos = { 325, collumnPosKin.y, 420, collumnPosKin.y + 25 };
	eAtomThresholdDispHandle.continuousSingleScansModePos = { 325, collumnPosCont.y, 420, collumnPosCont.y + 25 };
	eAtomThresholdDispHandle.accumulateModePos = { 325, collumnPosAccum.y, 420, collumnPosAccum.y + 25 };
	initPos = eAtomThresholdDispHandle.kineticSeriesModePos;
	eAtomThresholdDispHandle.triggerModeSensitive = 1;
	eAtomThresholdDispHandle.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_DETECTION_THRESHOLD_DISP_HANDLE, eHInst, NULL);
	eAtomThresholdDispHandle.fontType = "Normal";
	collumnPosKin.y += 30;
	collumnPosAccum.y += 30;
	collumnPosCont.y += 30;



	/// RUNNING AVERAGE OPTIONS
	// Set Number of Runs to Average
	eSetRunningAverageNumberToAverageButton.kineticSeriesModePos = { -1, -1, -1, -1 };
	eSetRunningAverageNumberToAverageButton.continuousSingleScansModePos = { 0, collumnPosCont.y, 220, collumnPosCont.y + 25 };
	eSetRunningAverageNumberToAverageButton.accumulateModePos = { -1, -1, -1, -1 };
	initPos = eSetRunningAverageNumberToAverageButton.kineticSeriesModePos;
	eSetRunningAverageNumberToAverageButton.triggerModeSensitive = 1;
	eSetRunningAverageNumberToAverageButton.hwnd = CreateWindowEx(0, "BUTTON", "Set # of Pictures to Average",
		WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_RUNNING_AVERAGE_BUTTON, eHInst, NULL);
	ShowWindow(eSetRunningAverageNumberToAverageButton.hwnd, SW_HIDE);
	eSetRunningAverageNumberToAverageButton.fontType = "Normal";
	// Running Average Edit
	eRunningAverageEdit.kineticSeriesModePos = { -1, -1, -1, -1 };
	eRunningAverageEdit.continuousSingleScansModePos = { 225, collumnPosCont.y, 320, collumnPosCont.y + 25 };
	eRunningAverageEdit.accumulateModePos = { -1, -1, -1, -1 };
	initPos = eRunningAverageEdit.kineticSeriesModePos;
	eRunningAverageEdit.triggerModeSensitive = 1;
	eRunningAverageEdit.hwnd = CreateWindowEx(0, "EDIT", "5",
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_RUNNING_AVERAGE_EDIT, eHInst, NULL);
	ShowWindow(eRunningAverageEdit.hwnd, SW_HIDE);
	eRunningAverageEdit.fontType = "Normal";
	// Running Average Edit Display
	eRunningAverageDisp.kineticSeriesModePos = { -1, -1, -1, -1 };
	eRunningAverageDisp.continuousSingleScansModePos = { 325, collumnPosCont.y, 420, collumnPosCont.y + 25 };
	eRunningAverageDisp.accumulateModePos = { -1, -1, -1, -1 };
	initPos = eRunningAverageDisp.kineticSeriesModePos;
	eRunningAverageDisp.triggerModeSensitive = 1;
	eRunningAverageDisp.hwnd = CreateWindowEx(0, "EDIT", "",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_RUNNING_AVERAGE_DISP, eHInst, NULL);
	ShowWindow(eRunningAverageDisp.hwnd, SW_HIDE);
	eRunningAverageDisp.fontType = "Normal";

	/// PLOTTING FREQUENCY CONTROLS
	// Set Plotting Frequency
	eSetPlottingFrequencyButton.kineticSeriesModePos = { 0, collumnPosKin.y, 220, collumnPosKin.y + 25 };
	eSetPlottingFrequencyButton.continuousSingleScansModePos = { 0, collumnPosCont.y, 220, collumnPosCont.y + 25 };
	eSetPlottingFrequencyButton.accumulateModePos = { 0, collumnPosAccum.y, 220, collumnPosAccum.y + 25 };
	initPos = eSetPlottingFrequencyButton.kineticSeriesModePos;
	eSetPlottingFrequencyButton.triggerModeSensitive = 1;
	eSetPlottingFrequencyButton.hwnd = CreateWindowEx(0, "BUTTON", "Set Plotting Frequency",
		WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_PLOTTING_FREQUENCY_BUTTON, eHInst, NULL);
	eSetPlottingFrequencyButton.fontType = "Normal";
	// Plotting Frequency Edit
	ePlottingFrequencyEdit.kineticSeriesModePos = { 225, collumnPosKin.y, 320, collumnPosKin.y + 25 };
	ePlottingFrequencyEdit.continuousSingleScansModePos = { 225, collumnPosCont.y, 320, collumnPosCont.y + 25 };
	ePlottingFrequencyEdit.accumulateModePos = { 225, collumnPosAccum.y, 320, collumnPosAccum.y + 25 };
	initPos = ePlottingFrequencyEdit.kineticSeriesModePos;
	ePlottingFrequencyEdit.triggerModeSensitive = 1;
	ePlottingFrequencyEdit.hwnd = CreateWindowEx(0, "EDIT", "1",
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_PLOTTING_FREQUENCY_EDIT, eHInst, NULL);
	ePlottingFrequencyEdit.fontType = "Normal";
	// Plotting Frequency Display
	ePlottingFrequencyDisp.kineticSeriesModePos = { 325, collumnPosKin.y, 420, collumnPosKin.y + 25 };
	ePlottingFrequencyDisp.continuousSingleScansModePos = { 325, collumnPosCont.y, 420, collumnPosCont.y + 25 };
	ePlottingFrequencyDisp.accumulateModePos = { 325, collumnPosAccum.y, 420, collumnPosAccum.y + 25 };
	initPos = ePlottingFrequencyDisp.kineticSeriesModePos;
	ePlottingFrequencyDisp.triggerModeSensitive = 1;
	ePlottingFrequencyDisp.hwnd = CreateWindowEx(0, "EDIT", "1",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_PLOTTING_FREQUENCY_DISP, eHInst, NULL);
	ePlottingFrequencyDisp.fontType = "Normal";
	collumnPosKin.y += 30;
	collumnPosAccum.y += 30;
	collumnPosCont.y += 30;

	/// Increment Data File Names Box
	eIncDataFileOptionBoxHandle.kineticSeriesModePos = { 0, collumnPosKin.y, 420, collumnPosKin.y + 20 };
	eIncDataFileOptionBoxHandle.continuousSingleScansModePos = { 0, collumnPosCont.y, 420, collumnPosCont.y + 20 };
	eIncDataFileOptionBoxHandle.accumulateModePos = { 0, collumnPosAccum.y, 420, collumnPosAccum.y + 20 };
	initPos = eIncDataFileOptionBoxHandle.kineticSeriesModePos;
	eIncDataFileOptionBoxHandle.triggerModeSensitive = 1;
	eIncDataFileOptionBoxHandle.hwnd = CreateWindowEx(0, "BUTTON", "Increment Data File Name",
												WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, 
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow,
												(HMENU)IDC_INCREMENT_FILE_OPTION_BUTTON, eHInst, NULL);
	eIncDataFileOptionBoxHandle.fontType = "Normal";
	SendMessage(eIncDataFileOptionBoxHandle.hwnd, BM_SETCHECK, BST_UNCHECKED, NULL);
	eIncDataFileNamesOption = false;
	collumnPosKin.y += 25;
	collumnPosAccum.y += 25;
	collumnPosCont.y += 25;
	/// PLOTTING 2.0 ////////////////////
	/// All plots
	// all plots Text
	eAllPlotsText.kineticSeriesModePos = { 0, collumnPosKin.y, 100, collumnPosKin.y + 25 };
	eAllPlotsText.continuousSingleScansModePos = { 0, collumnPosCont.y, 100, collumnPosCont.y + 25 };
	eAllPlotsText.accumulateModePos = { 0, collumnPosAccum.y, 100, collumnPosAccum.y + 25 };
	initPos = eAllPlotsText.kineticSeriesModePos;
	eAllPlotsText.triggerModeSensitive = 1;
	eAllPlotsText.hwnd = CreateWindowEx(0, "STATIC", "All Plots:", WS_CHILD | WS_VISIBLE | SS_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_PLOTTING_TEXT, eHInst, NULL);
	eAllPlotsText.fontType = "Normal";
	// all plots combo
	eAllPlotsCombo.kineticSeriesModePos = { 100, collumnPosKin.y, 420, collumnPosKin.y + 800 };
	eAllPlotsCombo.continuousSingleScansModePos = { 100, collumnPosCont.y, 420, collumnPosCont.y + 25 };
	eAllPlotsCombo.accumulateModePos = { 100, collumnPosAccum.y, 420, collumnPosAccum.y + 800 };
	initPos = eAllPlotsCombo.kineticSeriesModePos;
	eAllPlotsCombo.triggerModeSensitive = 1;
	eAllPlotsCombo.hwnd = CreateWindowEx(0, "COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_PLOTTING_COMBO, eHInst, NULL);
	// set options for the combo
	eAllPlotsCombo.fontType = "Normal";
	fileManage::reloadCombo(eAllPlotsCombo.hwnd, PLOT_FILES_SAVE_LOCATION, "*.plot", "__NONE__");
	collumnPosKin.y += 30;
	collumnPosAccum.y += 30;
	collumnPosCont.y += 30;

	/// Current Plots
	// current plots Text
	eCurrentPlotsText.kineticSeriesModePos = { 0, collumnPosKin.y, 100, collumnPosKin.y + 25 };
	eCurrentPlotsText.continuousSingleScansModePos = { 0, collumnPosCont.y, 100, collumnPosCont.y + 25 };
	eCurrentPlotsText.accumulateModePos = { 0, collumnPosAccum.y, 100, collumnPosAccum.y + 25 };
	initPos = eCurrentPlotsText.kineticSeriesModePos;
	eCurrentPlotsText.triggerModeSensitive = 1;
	eCurrentPlotsText.hwnd = CreateWindowEx(0, "STATIC", "Current Plots:", WS_CHILD | WS_VISIBLE | SS_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_CURRENT_PLOTS_TEXT, eHInst, NULL);
	eCurrentPlotsText.fontType = "Normal";
	// current Plots Combo
	eCurrentPlotsCombo.kineticSeriesModePos = { 100, collumnPosKin.y, 420, collumnPosKin.y + 800 };
	eCurrentPlotsCombo.continuousSingleScansModePos = { 100, collumnPosCont.y, 420, collumnPosCont.y + 800 };
	eCurrentPlotsCombo.accumulateModePos = { 100, collumnPosAccum.y, 420, collumnPosAccum.y + 800 };
	initPos = eCurrentPlotsCombo.kineticSeriesModePos;
	eCurrentPlotsCombo.triggerModeSensitive = 1;
	eCurrentPlotsCombo.hwnd = CreateWindowEx(0, "COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_CURRENT_PLOTS_COMBO, eHInst, NULL);
	// set options for the combo
	eCurrentPlotsCombo.fontType = "Normal";
	collumnPosKin.y += 30;
	collumnPosAccum.y += 30;
	collumnPosCont.y += 30;

	/// CURRENT IMAGE DATA
	// Current Accumulation Number Display
	eCurrentAccumulationNumDispHandle.kineticSeriesModePos = { 480, 0, 752, 25 };
	eCurrentAccumulationNumDispHandle.continuousSingleScansModePos = eCurrentAccumulationNumDispHandle.accumulateModePos = eCurrentAccumulationNumDispHandle.kineticSeriesModePos;
	initPos = eCurrentAccumulationNumDispHandle.kineticSeriesModePos;
	eCurrentAccumulationNumDispHandle.hwnd = CreateWindowEx(0, "EDIT", "Accumulation ?/?", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_ACCUMULATION_NUM_DISP, eHInst, NULL);
	eCurrentAccumulationNumDispHandle.fontType = "Normal";

	/// Picture labels
	//ePictureText
	ePictureText.kineticSeriesModePos = { 480, 25, 548, 50 };
	ePictureText.continuousSingleScansModePos = ePictureText.accumulateModePos = ePictureText.kineticSeriesModePos;
	initPos = ePictureText.kineticSeriesModePos;
	ePictureText.hwnd = CreateWindowEx(0, "STATIC", "Pic:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_PICTURE_DISP, eHInst, NULL);
	ePictureText.fontType = "Normal";

	// #1
	ePic1Text.kineticSeriesModePos = { 480, 50, 548, 75 };
	ePic1Text.continuousSingleScansModePos = ePic1Text.accumulateModePos = ePic1Text.kineticSeriesModePos;
	initPos = ePic1Text.kineticSeriesModePos;
	ePic1Text.hwnd = CreateWindowEx(0, "STATIC", "#1:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_PICTURE_1_DISP, eHInst, NULL);
	ePic1Text.fontType = "Normal";
	// #2
	ePic2Text.kineticSeriesModePos = { 480, 75, 548, 100 };
	ePic2Text.continuousSingleScansModePos = ePic2Text.accumulateModePos = ePic2Text.kineticSeriesModePos;
	initPos = ePic2Text.kineticSeriesModePos;
	ePic2Text.hwnd = CreateWindowEx(0, "STATIC", "#2:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_PICTURE_2_DISP, eHInst, NULL);
	ePic2Text.fontType = "Normal";
	// #3
	ePic3Text.kineticSeriesModePos = { 480, 100, 548, 125 };
	ePic3Text.continuousSingleScansModePos = ePic3Text.accumulateModePos = ePic3Text.kineticSeriesModePos;
	initPos = ePic3Text.kineticSeriesModePos;
	ePic3Text.hwnd = CreateWindowEx(0, "STATIC", "#3:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_PICTURE_3_DISP, eHInst, NULL);
	ePic3Text.fontType = "Normal";
	// #4
	ePic4Text.kineticSeriesModePos = { 480, 125, 548, 150 };
	ePic4Text.continuousSingleScansModePos = ePic4Text.accumulateModePos = ePic4Text.kineticSeriesModePos;
	initPos = ePic4Text.kineticSeriesModePos;
	ePic4Text.hwnd = CreateWindowEx(0, "STATIC", "#4:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_PICTURE_4_DISP, eHInst, NULL);
	ePic4Text.fontType = "Normal";
	/// Max Count Edits

	// Max Count Display 742 - 480 )/2 = 131 
	eMaxCountDispHandle.kineticSeriesModePos = { 548, 25, 616, 50 };
	eMaxCountDispHandle.continuousSingleScansModePos = eMaxCountDispHandle.accumulateModePos = eMaxCountDispHandle.kineticSeriesModePos;
	initPos = eMaxCountDispHandle.kineticSeriesModePos;
	eMaxCountDispHandle.hwnd = CreateWindowEx(0, "STATIC", "Max:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MAX_COUNT_DISP, eHInst, NULL);
	eMaxCountDispHandle.fontType = "Normal";
	// #1
	ePic1MaxCountDisp.kineticSeriesModePos = { 548, 50, 616, 75 };
	ePic1MaxCountDisp.continuousSingleScansModePos = ePic1MaxCountDisp.accumulateModePos = ePic1MaxCountDisp.kineticSeriesModePos;
	initPos = ePic1MaxCountDisp.kineticSeriesModePos;
	ePic1MaxCountDisp.hwnd = CreateWindowEx(0, "EDIT", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MAX_COUNT_1_DISP, eHInst, NULL);
	ePic1MaxCountDisp.fontType = "Normal";
	// #2
	ePic2MaxCountDisp.kineticSeriesModePos = { 548, 75, 616, 100 };
	ePic2MaxCountDisp.continuousSingleScansModePos = ePic2MaxCountDisp.accumulateModePos = ePic2MaxCountDisp.kineticSeriesModePos;
	initPos = ePic2MaxCountDisp.kineticSeriesModePos;
	ePic2MaxCountDisp.hwnd = CreateWindowEx(0, "EDIT", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MAX_COUNT_2_DISP, eHInst, NULL);
	ePic2MaxCountDisp.fontType = "Normal";
	// #3
	ePic3MaxCountDisp.kineticSeriesModePos = { 548, 100, 616, 125 };
	ePic3MaxCountDisp.continuousSingleScansModePos = ePic3MaxCountDisp.accumulateModePos = ePic3MaxCountDisp.kineticSeriesModePos;
	initPos = ePic3MaxCountDisp.kineticSeriesModePos;
	ePic3MaxCountDisp.hwnd = CreateWindowEx(0, "EDIT", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MAX_COUNT_3_DISP, eHInst, NULL);
	ePic3MaxCountDisp.fontType = "Normal";
	// #4
	ePic4MaxCountDisp.kineticSeriesModePos = { 548, 125, 616, 150 };
	ePic4MaxCountDisp.continuousSingleScansModePos = ePic4MaxCountDisp.accumulateModePos = ePic4MaxCountDisp.kineticSeriesModePos;
	initPos = ePic4MaxCountDisp.kineticSeriesModePos;
	ePic4MaxCountDisp.hwnd = CreateWindowEx(0, "EDIT", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MAX_COUNT_4_DISP, eHInst, NULL);
	ePic4MaxCountDisp.fontType = "Normal";
	/// Min Counts
	// Min Count Display
	eMinCountDispHandle.kineticSeriesModePos = { 616, 25, 684, 50 };
	eMinCountDispHandle.continuousSingleScansModePos = eMinCountDispHandle.accumulateModePos = eMinCountDispHandle.kineticSeriesModePos;
	initPos = eMinCountDispHandle.kineticSeriesModePos;
	eMinCountDispHandle.hwnd = CreateWindowEx(0, "STATIC", "Min:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MIN_COUNT_DISP, eHInst, NULL);
	eMinCountDispHandle.fontType = "Normal";
	// #1
	ePic1MinCountDisp.kineticSeriesModePos = { 616, 50, 684, 75 };
	ePic1MinCountDisp.continuousSingleScansModePos = ePic1MinCountDisp.accumulateModePos = ePic1MinCountDisp.kineticSeriesModePos;
	initPos = ePic1MinCountDisp.kineticSeriesModePos;
	ePic1MinCountDisp.hwnd = CreateWindowEx(0, "STATIC", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MIN_COUNT_1_DISP, eHInst, NULL);
	ePic1MinCountDisp.fontType = "Normal";
	// #2
	ePic2MinCountDisp.kineticSeriesModePos = { 616, 75, 684, 100 };
	ePic2MinCountDisp.continuousSingleScansModePos = ePic2MinCountDisp.accumulateModePos = ePic2MinCountDisp.kineticSeriesModePos;
	initPos = ePic2MinCountDisp.kineticSeriesModePos;
	ePic2MinCountDisp.hwnd = CreateWindowEx(0, "STATIC", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MIN_COUNT_2_DISP, eHInst, NULL);
	ePic2MinCountDisp.fontType = "Normal";
	// #3
	ePic3MinCountDisp.kineticSeriesModePos = { 616, 100, 684, 125 };
	ePic3MinCountDisp.continuousSingleScansModePos = ePic3MinCountDisp.accumulateModePos = ePic3MinCountDisp.kineticSeriesModePos;
	initPos = ePic3MinCountDisp.kineticSeriesModePos;
	ePic3MinCountDisp.hwnd = CreateWindowEx(0, "STATIC", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MIN_COUNT_3_DISP, eHInst, NULL);
	ePic3MinCountDisp.fontType = "Normal";
	// #4
	ePic4MinCountDisp.kineticSeriesModePos = { 616, 125, 684, 150 };
	ePic4MinCountDisp.continuousSingleScansModePos = ePic4MinCountDisp.accumulateModePos = ePic4MinCountDisp.kineticSeriesModePos;
	initPos = ePic4MinCountDisp.kineticSeriesModePos;
	ePic4MinCountDisp.hwnd = CreateWindowEx(0, "STATIC", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MIN_COUNT_4_DISP, eHInst, NULL);
	ePic4MinCountDisp.fontType = "Normal";
	/// Selection Counts
	//eSelectionText
	eSelectionText.kineticSeriesModePos = { 684, 25, 752, 50 };
	eSelectionText.continuousSingleScansModePos = eSelectionText.accumulateModePos = eSelectionText.kineticSeriesModePos;
	initPos = eSelectionText.kineticSeriesModePos;
	eSelectionText.hwnd = CreateWindowEx(0, "STATIC", "Sel:", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_SELECTION_DISP, eHInst, NULL);
	eSelectionText.fontType = "Normal";
	// #1
	ePic1SelectionCountDisp.kineticSeriesModePos = { 684, 50, 752, 75 };
	ePic1SelectionCountDisp.continuousSingleScansModePos = ePic1SelectionCountDisp.accumulateModePos = ePic1SelectionCountDisp.kineticSeriesModePos;
	initPos = ePic1SelectionCountDisp.kineticSeriesModePos;
	ePic1SelectionCountDisp.hwnd = CreateWindowEx(0, "STATIC", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_SELECTION_1_DISP, eHInst, NULL);
	ePic1SelectionCountDisp.fontType = "Normal";
	// #2
	ePic2SelectionCountDisp.kineticSeriesModePos = { 684, 75, 752, 100 };
	ePic2SelectionCountDisp.continuousSingleScansModePos = ePic2SelectionCountDisp.accumulateModePos = ePic2SelectionCountDisp.kineticSeriesModePos;
	initPos = ePic2SelectionCountDisp.kineticSeriesModePos;
	ePic2SelectionCountDisp.hwnd = CreateWindowEx(0, "STATIC", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_SELECTION_2_DISP, eHInst, NULL);
	ePic2SelectionCountDisp.fontType = "Normal";
	// #3
	ePic3SelectionCountDisp.kineticSeriesModePos = { 684, 100, 752, 125 };
	ePic3SelectionCountDisp.continuousSingleScansModePos = ePic3SelectionCountDisp.accumulateModePos = ePic3SelectionCountDisp.kineticSeriesModePos;
	initPos = ePic3SelectionCountDisp.kineticSeriesModePos;
	ePic3SelectionCountDisp.hwnd = CreateWindowEx(0, "STATIC", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_SELECTION_3_DISP, eHInst, NULL);
	ePic3SelectionCountDisp.fontType = "Normal";
	// #4
	ePic4SelectionCountDisp.kineticSeriesModePos = { 684, 125, 752, 150 };
	ePic4SelectionCountDisp.continuousSingleScansModePos = ePic4SelectionCountDisp.accumulateModePos = ePic4SelectionCountDisp.kineticSeriesModePos;
	initPos = ePic4SelectionCountDisp.kineticSeriesModePos;
	ePic4SelectionCountDisp.hwnd = CreateWindowEx(0, "STATIC", "-", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_SELECTION_4_DISP, eHInst, NULL);
	ePic4SelectionCountDisp.fontType = "Normal";

	/// IMAGE CONTRAST CONTROL #1
	// "min" text
	eMinSliderText1.accumulateModePos = eMinSliderText1.continuousSingleScansModePos = eMinSliderText1.kineticSeriesModePos = { 1236, 0, 1286, 30 };
	initPos = eMinSliderText1.kineticSeriesModePos;
	eMinSliderText1.hwnd = CreateWindowEx(0, "STATIC", "MIN", WS_CHILD | WS_VISIBLE | SS_CENTER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MIN_SLIDER_1_TEXT, eHInst, NULL);
	eMinSliderText1.fontType = "Normal";
	// minimum slider
	eMinimumPictureSlider1.accumulateModePos = eMinimumPictureSlider1.continuousSingleScansModePos = eMinimumPictureSlider1.kineticSeriesModePos = { 1236, 60, 1286, 480 };
	initPos = eMinimumPictureSlider1.kineticSeriesModePos;
	eMinimumPictureSlider1.hwnd = CreateWindowEx(0, TRACKBAR_CLASS, 0, WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_VERT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_MIN_SLIDER_1, eHInst, NULL);
	SendMessage(eMinimumPictureSlider1.hwnd, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(eMinimumSliderCount, eMaximumSliderCount));
	SendMessage(eMinimumPictureSlider1.hwnd, TBM_SETPAGESIZE, 0, (LPARAM)(eMaximumSliderCount - eMinimumSliderCount));

	// minimum number text
	eMinSliderNumberEdit1.continuousSingleScansModePos = eMinSliderNumberEdit1.accumulateModePos = eMinSliderNumberEdit1.kineticSeriesModePos = { 1236, 30, 1286, 60 };
	initPos = eMinSliderNumberEdit1.kineticSeriesModePos;
	eMinSliderNumberEdit1.hwnd = CreateWindowEx(0, "EDIT", "95", WS_CHILD | WS_VISIBLE | SS_LEFT | ES_AUTOHSCROLL,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MIN_SLIDER_1_NUMBER_EDIT, eHInst, NULL);
	eMinSliderNumberEdit1.fontType = "Normal";

	// "max" text
	eMaxSliderText1.accumulateModePos = eMaxSliderText1.continuousSingleScansModePos = eMaxSliderText1.kineticSeriesModePos = { 1286, 0, 1336, 30 };
	initPos = eMaxSliderText1.kineticSeriesModePos;
	eMaxSliderText1.hwnd = CreateWindowEx(0, "STATIC", "MAX", WS_CHILD | WS_VISIBLE | SS_CENTER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MAX_SLIDER_1_TEXT, eHInst, NULL);
	eMaxSliderText1.fontType = "Normal";
	// maximum slider
	eMaximumPictureSlider1.accumulateModePos = eMaximumPictureSlider1.continuousSingleScansModePos = eMaximumPictureSlider1.kineticSeriesModePos = { 1286, 60, 1336, 480 };
	initPos = eMaximumPictureSlider1.kineticSeriesModePos;
	eMaximumPictureSlider1.hwnd = CreateWindowEx(0, TRACKBAR_CLASS, 0, WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_VERT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_MAX_SLIDER_1, eHInst, NULL);
	SendMessage(eMaximumPictureSlider1.hwnd, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(eMinimumSliderCount, eMaximumSliderCount));
	SendMessage(eMaximumPictureSlider1.hwnd, TBM_SETPAGESIZE, 0, (LPARAM)(eMaximumSliderCount - eMinimumSliderCount));
	// maximum number text
	eMaxSliderNumberEdit1.continuousSingleScansModePos = eMaxSliderNumberEdit1.accumulateModePos = eMaxSliderNumberEdit1.kineticSeriesModePos = { 1286, 30, 1336, 60 };
	initPos = eMaxSliderNumberEdit1.kineticSeriesModePos;
	eMaxSliderNumberEdit1.hwnd = CreateWindowEx(0, "EDIT", "325", WS_CHILD | WS_VISIBLE | SS_LEFT | ES_AUTOHSCROLL,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MAX_SLIDER_1_NUMBER_EDIT, eHInst, NULL);
	eMaxSliderNumberEdit1.fontType = "Normal";
	/// IMAGE CONTRAST CONTROL #2
	// "min" text
	eMinSliderText2.accumulateModePos = eMinSliderText2.continuousSingleScansModePos = eMinSliderText2.kineticSeriesModePos = { 1840, 0, 1880, 30 };
	initPos = eMinSliderText2.kineticSeriesModePos;
	eMinSliderText2.hwnd = CreateWindowEx(0, "STATIC", "MIN", WS_CHILD | WS_VISIBLE | SS_CENTER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MIN_SLIDER_2_TEXT, eHInst, NULL);
	eMinSliderText2.fontType = "Normal";
	// minimum slider
	eMinimumPictureSlider2.accumulateModePos = eMinimumPictureSlider2.continuousSingleScansModePos = eMinimumPictureSlider2.kineticSeriesModePos = { 1840, 60, 1880, 480 };
	initPos = eMinimumPictureSlider2.kineticSeriesModePos;
	eMinimumPictureSlider2.hwnd = CreateWindowEx(0, TRACKBAR_CLASS, 0, WS_CHILD | WS_VISIBLE |	TBS_AUTOTICKS | TBS_VERT, 
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_MIN_SLIDER_2, eHInst,	NULL);
	SendMessage(eMinimumPictureSlider2.hwnd, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(eMinimumSliderCount, eMaximumSliderCount));
	SendMessage(eMinimumPictureSlider2.hwnd, TBM_SETPAGESIZE, 0, (LPARAM)(eMaximumSliderCount - eMinimumSliderCount));

	// minimum number text
	eMinSliderNumberEdit2.continuousSingleScansModePos = eMinSliderNumberEdit2.accumulateModePos = eMinSliderNumberEdit2.kineticSeriesModePos = { 1840, 30, 1880, 60 };
	initPos = eMinSliderNumberEdit2.kineticSeriesModePos;
	eMinSliderNumberEdit2.hwnd = CreateWindowEx(0, "EDIT", "95", WS_CHILD | WS_VISIBLE | SS_LEFT | ES_AUTOHSCROLL,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MIN_SLIDER_2_NUMBER_EDIT, eHInst, NULL);
	eMinSliderNumberEdit2.fontType = "Normal";

	// "max" text
	eMaxSliderText2.accumulateModePos = eMaxSliderText2.continuousSingleScansModePos = eMaxSliderText2.kineticSeriesModePos = { 1880, 0, 1920, 30 };
	initPos = eMaxSliderText2.kineticSeriesModePos;
	eMaxSliderText2.hwnd = CreateWindowEx(0, "STATIC", "MAX", WS_CHILD | WS_VISIBLE | SS_CENTER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MAX_SLIDER_2_TEXT, eHInst, NULL);
	eMaxSliderText2.fontType = "Normal";
	// maximum slider
	eMaximumPictureSlider2.accumulateModePos = eMaximumPictureSlider2.continuousSingleScansModePos = eMaximumPictureSlider2.kineticSeriesModePos = { 1880, 60, 1920, 480 };
	initPos = eMaximumPictureSlider2.kineticSeriesModePos;
	eMaximumPictureSlider2.hwnd = CreateWindowEx(0, TRACKBAR_CLASS, 0, WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_VERT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_MAX_SLIDER_2, eHInst, NULL);
	SendMessage(eMaximumPictureSlider2.hwnd, TBM_SETRANGE, (WPARAM)TRUE,	(LPARAM)MAKELONG(eMinimumSliderCount, eMaximumSliderCount));
	SendMessage(eMaximumPictureSlider2.hwnd, TBM_SETPAGESIZE, 0, (LPARAM)(eMaximumSliderCount - eMinimumSliderCount));

	// maximum number text
	eMaxSliderNumberEdit2.continuousSingleScansModePos = eMaxSliderNumberEdit2.accumulateModePos = eMaxSliderNumberEdit2.kineticSeriesModePos = { 1880, 30, 1920, 60 };
	initPos = eMaxSliderNumberEdit2.kineticSeriesModePos;
	eMaxSliderNumberEdit2.hwnd = CreateWindowEx(0, "EDIT", "325", WS_CHILD | WS_VISIBLE | SS_LEFT | ES_AUTOHSCROLL,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MAX_SLIDER_2_NUMBER_EDIT, eHInst, NULL);
	eMaxSliderNumberEdit2.fontType = "Normal";

	/// IMAGE CONTRAST CONTROL #3
	// "min" text
	eMinSliderText3.accumulateModePos = eMinSliderText3.continuousSingleScansModePos = eMinSliderText3.kineticSeriesModePos = { 1236, 520, 1286, 550 };
	initPos = eMinSliderText3.kineticSeriesModePos;
	eMinSliderText3.hwnd = CreateWindowEx(0, "STATIC", "MIN", WS_CHILD | WS_VISIBLE | SS_CENTER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MIN_SLIDER_3_TEXT, eHInst, NULL);
	eMinSliderText3.fontType = "Normal";
	// minimum slider
	eMinimumPictureSlider3.accumulateModePos = eMinimumPictureSlider3.continuousSingleScansModePos = eMinimumPictureSlider3.kineticSeriesModePos = { 1236, 580, 1286, 1000 };
	initPos = eMinimumPictureSlider3.kineticSeriesModePos;
	eMinimumPictureSlider3.hwnd = CreateWindowEx(0, TRACKBAR_CLASS, 0, WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_VERT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_MIN_SLIDER_3, eHInst, NULL);
	SendMessage(eMinimumPictureSlider3.hwnd, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(eMinimumSliderCount, eMaximumSliderCount));
	SendMessage(eMinimumPictureSlider3.hwnd, TBM_SETPAGESIZE, 0, (LPARAM)(eMaximumSliderCount - eMinimumSliderCount));

	// minimum number text
	eMinSliderNumberEdit3.continuousSingleScansModePos = eMinSliderNumberEdit3.accumulateModePos = eMinSliderNumberEdit3.kineticSeriesModePos = { 1236, 550, 1286, 580 };
	initPos = eMinSliderNumberEdit3.kineticSeriesModePos;
	eMinSliderNumberEdit3.hwnd = CreateWindowEx(0, "EDIT", "95", WS_CHILD | WS_VISIBLE | SS_LEFT | ES_AUTOHSCROLL,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MIN_SLIDER_3_NUMBER_EDIT, eHInst, NULL);
	eMinSliderNumberEdit3.fontType = "Normal";

	// "max" text
	eMaxSliderText3.accumulateModePos = eMaxSliderText3.continuousSingleScansModePos = eMaxSliderText3.kineticSeriesModePos = { 1286, 520, 1336, 550 };
	initPos = eMaxSliderText3.kineticSeriesModePos;
	eMaxSliderText3.hwnd = CreateWindowEx(0, "STATIC", "MAX", WS_CHILD | WS_VISIBLE | SS_CENTER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MAX_SLIDER_3_TEXT, eHInst, NULL);
	eMaxSliderText3.fontType = "Normal";
	// maximum slider
	eMaximumPictureSlider3.accumulateModePos = eMaximumPictureSlider3.continuousSingleScansModePos = eMaximumPictureSlider3.kineticSeriesModePos = { 1286, 580, 1336, 1000 };
	initPos = eMaximumPictureSlider3.kineticSeriesModePos;
	eMaximumPictureSlider3.hwnd = CreateWindowEx(0, TRACKBAR_CLASS, 0, WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_VERT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_MAX_SLIDER_3, eHInst, NULL);
	SendMessage(eMaximumPictureSlider3.hwnd, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(eMinimumSliderCount, eMaximumSliderCount));
	SendMessage(eMaximumPictureSlider3.hwnd, TBM_SETPAGESIZE, 0, (LPARAM)(eMaximumSliderCount - eMinimumSliderCount));

	// maximum number text
	eMaxSliderNumberEdit3.continuousSingleScansModePos = eMaxSliderNumberEdit3.accumulateModePos = eMaxSliderNumberEdit3.kineticSeriesModePos = { 1286, 550, 1336, 580 };
	initPos = eMaxSliderNumberEdit3.kineticSeriesModePos;
	eMaxSliderNumberEdit3.hwnd = CreateWindowEx(0, "EDIT", "325", WS_CHILD | WS_VISIBLE | SS_LEFT | ES_AUTOHSCROLL,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MAX_SLIDER_3_NUMBER_EDIT, eHInst, NULL);
	eMaxSliderNumberEdit3.fontType = "Normal";
	/// IMAGE CONTRAST CONTROL #4
	// "min" text
	eMinSliderText4.accumulateModePos = eMinSliderText4.continuousSingleScansModePos = eMinSliderText4.kineticSeriesModePos = { 1840, 520, 1880, 550 };
	initPos = eMinSliderText4.kineticSeriesModePos;
	eMinSliderText4.hwnd = CreateWindowEx(0, "STATIC", "MIN", WS_CHILD | WS_VISIBLE | SS_CENTER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MIN_SLIDER_4_TEXT, eHInst, NULL);
	eMinSliderText4.fontType = "Normal";
	// minimum slider
	eMinimumPictureSlider4.accumulateModePos = eMinimumPictureSlider4.continuousSingleScansModePos = eMinimumPictureSlider4.kineticSeriesModePos = { 1840, 580, 1880, 1000 };
	initPos = eMinimumPictureSlider4.kineticSeriesModePos;
	eMinimumPictureSlider4.hwnd = CreateWindowEx(0, TRACKBAR_CLASS, 0, WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_VERT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_MIN_SLIDER_4, eHInst, NULL);
	SendMessage(eMinimumPictureSlider4.hwnd, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(eMinimumSliderCount, eMaximumSliderCount));
	SendMessage(eMinimumPictureSlider4.hwnd, TBM_SETPAGESIZE, 0, (LPARAM)(eMaximumSliderCount - eMinimumSliderCount));

	// minimum number text
	eMinSliderNumberEdit4.continuousSingleScansModePos = eMinSliderNumberEdit4.accumulateModePos = eMinSliderNumberEdit4.kineticSeriesModePos = { 1840, 550, 1880, 580 };
	initPos = eMinSliderNumberEdit4.kineticSeriesModePos;
	eMinSliderNumberEdit4.hwnd = CreateWindowEx(0, "EDIT", "95", WS_CHILD | WS_VISIBLE | SS_LEFT | ES_AUTOHSCROLL,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MIN_SLIDER_4_NUMBER_EDIT, eHInst, NULL);
	eMinSliderNumberEdit4.fontType = "Normal";

	// "max" text
	eMaxSliderText4.accumulateModePos = eMaxSliderText4.continuousSingleScansModePos = eMaxSliderText4.kineticSeriesModePos = { 1880, 520, 1920, 550 };
	initPos = eMaxSliderText4.kineticSeriesModePos;
	eMaxSliderText4.hwnd = CreateWindowEx(0, "STATIC", "MAX", WS_CHILD | WS_VISIBLE | SS_CENTER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MAX_SLIDER_4_TEXT, eHInst, NULL);
	eMaxSliderText4.fontType = "Normal";
	// maximum slider
	eMaximumPictureSlider4.accumulateModePos = eMaximumPictureSlider4.continuousSingleScansModePos = eMaximumPictureSlider4.kineticSeriesModePos = { 1880, 580, 1920, 1000 };
	initPos = eMaximumPictureSlider4.kineticSeriesModePos;
	eMaximumPictureSlider4.hwnd = CreateWindowEx(0, TRACKBAR_CLASS, 0, WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_VERT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_MAX_SLIDER_4, eHInst, NULL);
	SendMessage(eMaximumPictureSlider4.hwnd, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(eMinimumSliderCount, eMaximumSliderCount));
	SendMessage(eMaximumPictureSlider4.hwnd, TBM_SETPAGESIZE, 0, (LPARAM)(eMaximumSliderCount - eMinimumSliderCount));

	// maximum number text
	eMaxSliderNumberEdit4.continuousSingleScansModePos = eMaxSliderNumberEdit4.accumulateModePos = eMaxSliderNumberEdit4.kineticSeriesModePos = { 1880, 550, 1920, 580 };
	initPos = eMaxSliderNumberEdit4.kineticSeriesModePos;
	eMaxSliderNumberEdit4.hwnd = CreateWindowEx(0, "EDIT", "325", WS_CHILD | WS_VISIBLE | SS_LEFT | ES_AUTOHSCROLL,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_MAX_SLIDER_4_NUMBER_EDIT, eHInst, NULL);
	eMaxSliderNumberEdit4.fontType = "Normal";

	/// TIMER
	POINT timerLocation = { 752, 480 };
	eCameraWindowExperimentTimer.initializeControls(timerLocation, timerLocation, timerLocation, cameraWindow, false);
	/// STATUS
	// Title
	eStatusText.kineticSeriesModePos = { 0, 645, 400, 670 };
	eStatusText.continuousSingleScansModePos = eStatusText.accumulateModePos = eStatusText.kineticSeriesModePos;
	initPos = eStatusText.kineticSeriesModePos;
	eStatusText.hwnd = CreateWindowEx(0, "STATIC", "Camera Status Messages", WS_CHILD | WS_VISIBLE | ES_CENTER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)-1, eHInst, NULL);
	eStatusText.fontType = "Normal";

	// Clear Status Button
	eClearStatusButtonHandle.kineticSeriesModePos = { 400, 645, 480, 670 };
	eClearStatusButtonHandle.continuousSingleScansModePos = eClearStatusButtonHandle.accumulateModePos = eClearStatusButtonHandle.kineticSeriesModePos;
	initPos = eClearStatusButtonHandle.kineticSeriesModePos;
	eClearStatusButtonHandle.hwnd = CreateWindowEx(0, "BUTTON", "CLEAR", WS_CHILD | WS_VISIBLE,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_CLEAR_STATUS_BUTTON, eHInst, NULL);
	eClearStatusButtonHandle.fontType = "Normal";
	// Status Edit Handle
	eStatusEditHandle.kineticSeriesModePos = { 0, 670, 480, 925 };
	eStatusEditHandle.continuousSingleScansModePos = eStatusEditHandle.accumulateModePos = eStatusEditHandle.kineticSeriesModePos;
	initPos = eStatusEditHandle.kineticSeriesModePos;
	eStatusEditHandle.hwnd = CreateWindowEx(0, "EDIT", "",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_MULTILINE | WS_VSCROLL | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_STATUS_EDIT, eHInst, NULL);
	eStatusEditHandle.fontType = "Normal";

	// ~2X the default text limit. As high as possible without a rich edit control.
	SendMessage(eStatusEditHandle.hwnd, EM_LIMITTEXT, 64000, 0);

	/// TEMPERATURE
	// Set temperature Button
	eSetTemperatureButtonHandle.kineticSeriesModePos = { 0, 925, 270, 950 };
	initPos = eSetTemperatureButtonHandle.kineticSeriesModePos;
	eSetTemperatureButtonHandle.continuousSingleScansModePos = eSetTemperatureButtonHandle.accumulateModePos = eSetTemperatureButtonHandle.kineticSeriesModePos;
	eSetTemperatureButtonHandle.hwnd = CreateWindowEx(0, "BUTTON", "Set Camera Temperature (C)",
		WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_SET_TEMP_BUTTON, eHInst, NULL);
	eSetTemperatureButtonHandle.fontType = "Normal";
	
	// Temperature Edit
	eTempEditHandle.kineticSeriesModePos = { 270, 925, 350, 950 };
	eTempEditHandle.continuousSingleScansModePos = eTempEditHandle.accumulateModePos = eTempEditHandle.kineticSeriesModePos;
	initPos = eTempEditHandle.kineticSeriesModePos;
	eTempEditHandle.hwnd = CreateWindowEx(0, "EDIT", "0",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_TEMP_EDIT, eHInst, NULL);
	eTempEditHandle.fontType = "Normal";

	// Temperature Setting Display
	eTempDispHandle.kineticSeriesModePos = { 350, 925, 430, 950 };
	eTempDispHandle.continuousSingleScansModePos = eTempDispHandle.accumulateModePos = eTempDispHandle.kineticSeriesModePos;
	initPos = eTempDispHandle.kineticSeriesModePos;
	eTempDispHandle.hwnd = CreateWindowEx(0, "EDIT", "",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_TEMP_DISP_EDIT, eHInst, NULL);
	eTempDispHandle.fontType = "Normal";
	// Temperature Control Off Button
	eTempOffButton.kineticSeriesModePos = { 430, 925, 480, 950 };
	initPos = eTempOffButton.kineticSeriesModePos;
	eTempOffButton.continuousSingleScansModePos = eTempOffButton.accumulateModePos = eTempOffButton.kineticSeriesModePos;
	eTempOffButton.hwnd = CreateWindowEx(0, "BUTTON", "OFF",
		WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_TEMP_OFF_BUTTON, eHInst, NULL);
	eTempOffButton.fontType = "Normal";

	// Current Temp Message Display
	eCurrentTempDisplayHandle.kineticSeriesModePos = { 0, 950, 480, 1000 };
	eCurrentTempDisplayHandle.continuousSingleScansModePos = eCurrentTempDisplayHandle.accumulateModePos = eCurrentTempDisplayHandle.kineticSeriesModePos;
	initPos = eCurrentTempDisplayHandle.kineticSeriesModePos;
	eCurrentTempDisplayHandle.hwnd = CreateWindowEx(0, "STATIC", "Temperature control is disabled",
		WS_CHILD | WS_VISIBLE | SS_LEFT, initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_CURRENT_TEMP_DISP, eHInst, NULL);
	eCurrentTempDisplayHandle.fontType = "Normal";

	/// ERROR
	// title
	eErrorText.kineticSeriesModePos = { 480, 620, 680, 645 };
	eErrorText.continuousSingleScansModePos = eErrorText.accumulateModePos = eErrorText.kineticSeriesModePos;
	initPos = eErrorText.kineticSeriesModePos;
	eErrorText.hwnd = CreateWindowEx(0, "STATIC", "Error Messages", WS_CHILD | WS_VISIBLE | ES_CENTER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)-1, eHInst, NULL);
	eErrorText.fontType = "Normal";
	// clear button
	eErrorClear.kineticSeriesModePos = { 680, 620, 752, 645 };
	eErrorClear.continuousSingleScansModePos = eErrorClear.accumulateModePos = eErrorClear.kineticSeriesModePos;
	initPos = eErrorClear.kineticSeriesModePos;
	eErrorClear.hwnd = CreateWindowEx(0, "BUTTON", "CLEAR", WS_CHILD | WS_VISIBLE,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_ERROR_CLEAR_BUTTON, eHInst, NULL);
	eErrorClear.fontType = "Normal";
	// Error Edit Handle
	eErrorEditHandle.kineticSeriesModePos = { 480, 645, 752, 1000 };
	eErrorEditHandle.continuousSingleScansModePos = eErrorEditHandle.accumulateModePos = eErrorEditHandle.kineticSeriesModePos;
	initPos = eErrorEditHandle.kineticSeriesModePos;
	eErrorEditHandle.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_MULTILINE
		| WS_VSCROLL | ES_READONLY, initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_ERROR_EDIT, eHInst, NULL);
	eErrorEditHandle.fontType = "Normal";
	// ~2X the default text limit. As high as possible without a rich edit control.
	SendMessage(eErrorEditHandle.hwnd, EM_LIMITTEXT, 64000, 0);

	/// /// /// OTHER STUFFS
	// create pallete for drawing
	eDrawPallete = GetDC(eCameraWindowHandle);
	createIdentityPalette(eDrawPallete);

	DWORD cameraThreadID;
	eCameraThreadHandle = CreateThread(NULL, 0, cameraThread, NULL, 0, &cameraThreadID);
	// hard coded positions...
	reorganizeWindow("Kinetic Series Mode", cameraWindow);
	eImageDrawAreas = eImageBackgroundAreas;
	return false;
}
