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
	int collumnPosKin = 0, collumnPosAccum = 0, collumnPosCont = 0;

	///  /// Initialize all controls and HWNDs for this window. ///  ///
	InitCommonControlsEx(0);
	RECT initPos;
	/// CAMERA MODE
	eCameraModeComboHandle.kineticSeriesModePos = { 0, collumnPosKin, 480, collumnPosKin + 800 };
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
	collumnPosKin += 30;
	collumnPosAccum += 30;
	collumnPosCont += 30;
	/// EM GAIN
	
	eSetEMGain.kineticSeriesModePos = { 0, collumnPosKin, 120, collumnPosKin + 20 };
	eSetEMGain.continuousSingleScansModePos = eSetEMGain.accumulateModePos = eSetEMGain.kineticSeriesModePos;
	initPos = eSetEMGain.kineticSeriesModePos;
	eSetEMGain.hwnd = CreateWindowEx(0, "BUTTON", "Set EM Gain",
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow,
		(HMENU)IDC_SET_EM_GAIN_MODE, eHInst, NULL);
	eSetEMGain.fontType = "Normal";

	eEMGainText.kineticSeriesModePos = { 120, collumnPosKin, 280, collumnPosKin + 20 };
	eEMGainText.continuousSingleScansModePos = eEMGainText.accumulateModePos = eEMGainText.kineticSeriesModePos;
	initPos = eEMGainText.kineticSeriesModePos;
	eEMGainText.hwnd = CreateWindowEx(0, "STATIC", "EM Gain Setting:",
		WS_CHILD | WS_VISIBLE | BS_RIGHT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow,
		(HMENU)-1, eHInst, NULL);
	eEMGainText.fontType = "Normal";
	//
	eEMGainEdit.kineticSeriesModePos = { 280, collumnPosKin, 380, collumnPosKin + 20 };
	eEMGainEdit.continuousSingleScansModePos = eEMGainEdit.accumulateModePos = eEMGainEdit.kineticSeriesModePos;
	initPos = eEMGainEdit.kineticSeriesModePos;
	eEMGainEdit.hwnd = CreateWindowEx(0, "EDIT", "",
		WS_CHILD | WS_VISIBLE | BS_RIGHT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow,
		(HMENU)IDC_EM_GAIN_MODE_EDIT, eHInst, NULL);
	eEMGainText.fontType = "Normal";
	
	eEMGainDisplay.kineticSeriesModePos = { 380, collumnPosKin, 480, collumnPosKin + 20 };
	eEMGainDisplay.continuousSingleScansModePos = eEMGainDisplay.accumulateModePos = eEMGainDisplay.kineticSeriesModePos;
	initPos = eEMGainDisplay.kineticSeriesModePos;
	eEMGainDisplay.hwnd = CreateWindowEx(0, "EDIT", "OFF",
		WS_CHILD | WS_VISIBLE | BS_RIGHT | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow,
		(HMENU)IDC_EM_GAIN_MODE_DISP, eHInst, NULL);
	eEMGainDisplay.fontType = "Normal";
	eEMGainMode = false;

	collumnPosKin += 25;
	collumnPosAccum += 25;
	collumnPosCont += 25;

	/// TEMPERATURE
	// Set temperature Button
	eSetTemperatureButtonHandle.kineticSeriesModePos = { 0, collumnPosKin, 250, collumnPosKin + 25 };
	initPos = eSetTemperatureButtonHandle.kineticSeriesModePos;
	eSetTemperatureButtonHandle.continuousSingleScansModePos = eSetTemperatureButtonHandle.accumulateModePos = eSetTemperatureButtonHandle.kineticSeriesModePos;
	eSetTemperatureButtonHandle.hwnd = CreateWindowEx(0, "BUTTON", "Set Camera Temperature (C)",
													  WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
													  initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, 
													  cameraWindow, (HMENU)IDC_SET_TEMP_BUTTON, eHInst, NULL);
	eSetTemperatureButtonHandle.fontType = "Normal";

	// Temperature Edit
	eTempEditHandle.kineticSeriesModePos = { 255, collumnPosKin, 335, collumnPosKin + 25 };
	eTempEditHandle.continuousSingleScansModePos = eTempEditHandle.accumulateModePos = eTempEditHandle.kineticSeriesModePos;
	initPos = eTempEditHandle.kineticSeriesModePos;
	eTempEditHandle.hwnd = CreateWindowEx(0, "EDIT", "0",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_TEMP_EDIT, eHInst, NULL);
	eTempEditHandle.fontType = "Normal";


	// Temperature Setting Display
	eTempDispHandle.kineticSeriesModePos = { 340, collumnPosKin, 480, collumnPosKin + 25 };
	eTempDispHandle.continuousSingleScansModePos = eTempDispHandle.accumulateModePos = eTempDispHandle.kineticSeriesModePos;
	initPos = eTempDispHandle.kineticSeriesModePos;
	eTempDispHandle.hwnd = CreateWindowEx(0, "EDIT", "",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_TEMP_DISP_EDIT, eHInst, NULL);
	eTempDispHandle.fontType = "Normal";
	collumnPosKin += 30;
	collumnPosAccum += 30;
	collumnPosCont += 30;

	// Current Temp Message Display
	eCurrentTempDisplayHandle.kineticSeriesModePos = { 0, collumnPosKin, 480, collumnPosKin + 50 };
	eCurrentTempDisplayHandle.continuousSingleScansModePos = eCurrentTempDisplayHandle.accumulateModePos = eCurrentTempDisplayHandle.kineticSeriesModePos;
	initPos = eCurrentTempDisplayHandle.kineticSeriesModePos;
	eCurrentTempDisplayHandle.hwnd = CreateWindowEx(0, "STATIC", "Temperature control is disabled",
		WS_CHILD | WS_VISIBLE | SS_LEFT, initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_CURRENT_TEMP_DISP, eHInst, NULL);
	eCurrentTempDisplayHandle.fontType = "Normal";
	collumnPosKin += 50;
	collumnPosAccum += 50;
	collumnPosCont += 50;

	/// EXPOSURE
	// Set Exposure Button
	eSetExposureButtonHandle.kineticSeriesModePos = { 0, collumnPosKin, 480, collumnPosKin + 25 };
	eSetExposureButtonHandle.continuousSingleScansModePos = eSetExposureButtonHandle.accumulateModePos = eSetExposureButtonHandle.kineticSeriesModePos;
	initPos = eSetExposureButtonHandle.kineticSeriesModePos;
	eSetExposureButtonHandle.hwnd = CreateWindowEx(0, "BUTTON", "Set Exposure Time (ms)",
											 WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
											initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, 
											cameraWindow, (HMENU)IDC_SET_EXPOSURE_BUTTON, eHInst, NULL);
	eSetExposureButtonHandle.fontType = "Normal";
	collumnPosKin += 25;
	collumnPosAccum += 25;
	collumnPosCont += 25;
	// Exposure Edits
	// #1
	eExposure1EditHandle.kineticSeriesModePos = { 0, collumnPosKin, 160, collumnPosKin + 25 };
	eExposure1EditHandle.continuousSingleScansModePos = eExposure1EditHandle.accumulateModePos = eExposure1EditHandle.kineticSeriesModePos;
	initPos = eExposure1EditHandle.kineticSeriesModePos;
	eExposure1EditHandle.hwnd = CreateWindowEx(0, "EDIT", "10",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_EXPOSURE_1_EDIT, eHInst, NULL);
	eExposure1EditHandle.fontType = "Normal";
	// #2
	eExposure2EditHandle.kineticSeriesModePos = { 160, collumnPosKin, 320, collumnPosKin + 25 };
	eExposure2EditHandle.continuousSingleScansModePos = eExposure2EditHandle.accumulateModePos = eExposure2EditHandle.kineticSeriesModePos;
	initPos = eExposure2EditHandle.kineticSeriesModePos;
	eExposure2EditHandle.hwnd = CreateWindowEx(0, "EDIT", "10",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_EXPOSURE_2_EDIT, eHInst, NULL);
	eExposure2EditHandle.fontType = "Normal";
	// #3
	eExposure3EditHandle.kineticSeriesModePos = { 320, collumnPosKin, 480, collumnPosKin + 25 };
	eExposure3EditHandle.continuousSingleScansModePos = eExposure3EditHandle.accumulateModePos = eExposure3EditHandle.kineticSeriesModePos;
	initPos = eExposure3EditHandle.kineticSeriesModePos;
	eExposure3EditHandle.hwnd = CreateWindowEx(0, "EDIT", "10",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_EXPOSURE_3_EDIT, eHInst, NULL);
	eExposure3EditHandle.fontType = "Normal";
	// Fill in default exposure time
	
	collumnPosKin += 25;
	collumnPosAccum += 25;
	collumnPosCont += 25;
	// Exposure Display
	eExposureDispHandle.kineticSeriesModePos = { 0, collumnPosKin, 480, collumnPosKin + 25 };
	eExposureDispHandle.continuousSingleScansModePos = eExposureDispHandle.accumulateModePos = eExposureDispHandle.kineticSeriesModePos;
	initPos = eExposureDispHandle.kineticSeriesModePos;
	eExposureDispHandle.hwnd = CreateWindowEx(0, "EDIT", "",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_EXPOSURE_DISP, eHInst, NULL);
	eExposureDispHandle.fontType = "Normal";
	collumnPosKin += 30;
	collumnPosAccum += 30;
	collumnPosCont += 30;

	/// TRIGGER
	// Trigger Text

	eTriggerTextDisplayHandle.kineticSeriesModePos = { 0, collumnPosKin, 240, collumnPosKin + 25 };
	eTriggerTextDisplayHandle.continuousSingleScansModePos = eTriggerTextDisplayHandle.accumulateModePos = eTriggerTextDisplayHandle.kineticSeriesModePos;
	initPos = eTriggerTextDisplayHandle.kineticSeriesModePos;
	eTriggerTextDisplayHandle.hwnd = CreateWindowEx(0, "STATIC", "Trigger Mode:", WS_CHILD | WS_VISIBLE | SS_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_TRIGGER_DISP, eHInst, NULL);
	eTriggerTextDisplayHandle.fontType = "Normal";
	// trigger combo
	eTriggerComboHandle.kineticSeriesModePos = { 245, collumnPosKin, 420, collumnPosKin + 800 };
	eTriggerComboHandle.continuousSingleScansModePos = eTriggerComboHandle.accumulateModePos = eTriggerComboHandle.kineticSeriesModePos;
	initPos = eTriggerComboHandle.kineticSeriesModePos;
	eTriggerComboHandle.hwnd = CreateWindowEx(0, "COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_TRIGGER_COMBO, eHInst, NULL);
	// set options for the combo
	eTriggerComboHandle.fontType = "Normal";
	SendMessage(eTriggerComboHandle.hwnd, CB_ADDSTRING, 0, (LPARAM)"Internal");
	SendMessage(eTriggerComboHandle.hwnd, CB_ADDSTRING, 0, (LPARAM)"External");
	// Select default trigger
	SendMessage(eTriggerComboHandle.hwnd, CB_SELECTSTRING, 0, (LPARAM)"Internal");
	collumnPosKin += 30;
	collumnPosAccum += 30;
	collumnPosCont += 30;

	/// IMAGE PARAMETERS
	// Set Image Parameters Button
	eSetImageParametersButtonHandle.kineticSeriesModePos = { 0, collumnPosKin, 420, collumnPosKin + 25 };
	eSetImageParametersButtonHandle.continuousSingleScansModePos = eSetImageParametersButtonHandle.accumulateModePos = eSetImageParametersButtonHandle.kineticSeriesModePos;
	initPos = eSetImageParametersButtonHandle.kineticSeriesModePos;
	eSetImageParametersButtonHandle.hwnd = CreateWindowEx(0, "BUTTON", "Set Camera Image Dimensions and Binning",
													WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_SET_IMAGE_PARAMS_BUTTON, eHInst, NULL);
	eSetImageParametersButtonHandle.fontType = "Normal";
	collumnPosKin += 30;
	collumnPosAccum += 30;
	collumnPosCont += 30;

	/// Image Horizontal Texts
	// Image Left Side Text
	eImgLeftSideTextHandle.kineticSeriesModePos = { 0, collumnPosKin, 140, collumnPosKin + 20 };
	eImgLeftSideTextHandle.continuousSingleScansModePos = eImgLeftSideTextHandle.accumulateModePos = eImgLeftSideTextHandle.kineticSeriesModePos;
	initPos = eImgLeftSideTextHandle.kineticSeriesModePos;
	eImgLeftSideTextHandle.hwnd = CreateWindowEx(0, "STATIC", "Leftmost", WS_CHILD | WS_VISIBLE | SS_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_IMG_LEFT_TEXT, eHInst, NULL);
	eImgLeftSideTextHandle.fontType = "Normal";
	// Image Right Side Text
	eImgRightSideTextHandle.kineticSeriesModePos = { 140, collumnPosKin, 280, collumnPosKin + 20 };
	eImgRightSideTextHandle.continuousSingleScansModePos = eImgRightSideTextHandle.accumulateModePos = eImgRightSideTextHandle.kineticSeriesModePos;
	initPos = eImgRightSideTextHandle.kineticSeriesModePos;
	eImgRightSideTextHandle.hwnd = CreateWindowEx(0, "STATIC", "Rightmost", WS_CHILD | WS_VISIBLE | SS_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_IMG_RIGHT_TEXT, eHInst, NULL);
	eImgRightSideTextHandle.fontType = "Normal";
	// Horizontal Binning Text
	eHorizontalBinningTextHandle.kineticSeriesModePos = { 280, collumnPosKin, 420, collumnPosKin + 20 };
	eHorizontalBinningTextHandle.continuousSingleScansModePos = eHorizontalBinningTextHandle.accumulateModePos = eHorizontalBinningTextHandle.kineticSeriesModePos;
	initPos = eHorizontalBinningTextHandle.kineticSeriesModePos;
	eHorizontalBinningTextHandle.hwnd = CreateWindowEx(0, "STATIC", "Horizontal Bin Size", WS_CHILD | WS_VISIBLE | SS_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_HOR_BIN_TEXT, eHInst, NULL);
	eHorizontalBinningTextHandle.fontType = "Normal";
	collumnPosKin += 25;
	collumnPosAccum += 25;
	collumnPosCont += 25;

	/// Image Horizontal Edits
	// Image Left Side Edit
	eImgLeftSideEditHandle.kineticSeriesModePos = { 0, collumnPosKin, 140, collumnPosKin + 20 };
	eImgLeftSideEditHandle.continuousSingleScansModePos = eImgLeftSideEditHandle.accumulateModePos = eImgLeftSideEditHandle.kineticSeriesModePos;
	initPos = eImgLeftSideEditHandle.kineticSeriesModePos;
	eImgLeftSideEditHandle.hwnd = CreateWindowEx(0, "EDIT", "1",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_IMG_LEFT_EDIT, eHInst, NULL);
	eImgLeftSideEditHandle.fontType = "Normal";
	// Image Right Side Edit
	eImgRightSideEditHandle.kineticSeriesModePos = { 140, collumnPosKin, 280, collumnPosKin + 20 };
	eImgRightSideEditHandle.continuousSingleScansModePos = eImgRightSideEditHandle.accumulateModePos = eImgRightSideEditHandle.kineticSeriesModePos;
	initPos = eImgRightSideEditHandle.kineticSeriesModePos;
	eImgRightSideEditHandle.hwnd = CreateWindowEx(0, "EDIT", "5",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_IMG_RIGHT_EDIT, eHInst, NULL);
	eImgRightSideEditHandle.fontType = "Normal";
	// Horizontal Binning Edit
	eHorizontalBinningEditHandle.kineticSeriesModePos = { 280, collumnPosKin, 420, collumnPosKin + 20 };
	eHorizontalBinningEditHandle.continuousSingleScansModePos = eHorizontalBinningEditHandle.accumulateModePos = eHorizontalBinningEditHandle.kineticSeriesModePos;
	initPos = eHorizontalBinningEditHandle.kineticSeriesModePos;
	eHorizontalBinningEditHandle.hwnd = CreateWindowEx(0, "EDIT", "1",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_HOR_BIN_EDIT, eHInst, NULL);
	eHorizontalBinningEditHandle.fontType = "Normal";
	collumnPosKin += 25;
	collumnPosAccum += 25;
	collumnPosCont += 25;
	/// Image Horizontal Displays
	// Image Left Side Display
	eImgLeftSideDispHandle.kineticSeriesModePos = { 0, collumnPosKin, 140, collumnPosKin + 20 };
	eImgLeftSideDispHandle.continuousSingleScansModePos = eImgLeftSideDispHandle.accumulateModePos = eImgLeftSideDispHandle.kineticSeriesModePos;
	initPos = eImgLeftSideDispHandle.kineticSeriesModePos;
	eImgLeftSideDispHandle.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_IMG_LEFT_DISP, eHInst, NULL);
	eImgLeftSideDispHandle.fontType = "Normal";
	// Image Right Side Display
	eImgRightSideDispHandle.kineticSeriesModePos = { 140, collumnPosKin, 280, collumnPosKin + 20 };
	eImgRightSideDispHandle.continuousSingleScansModePos = eImgRightSideDispHandle.accumulateModePos = eImgRightSideDispHandle.kineticSeriesModePos;
	initPos = eImgRightSideDispHandle.kineticSeriesModePos;
	eImgRightSideDispHandle.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_IMG_RIGHT_DISP, eHInst, NULL);
	eImgRightSideDispHandle.fontType = "Normal";
	// Horizontal Binning Display
	eHorizontalBinningDispHandle.kineticSeriesModePos = { 280, collumnPosKin, 420, collumnPosKin + 20 };
	eHorizontalBinningDispHandle.continuousSingleScansModePos = eHorizontalBinningDispHandle.accumulateModePos = eHorizontalBinningDispHandle.kineticSeriesModePos;
	initPos = eHorizontalBinningDispHandle.kineticSeriesModePos;
	eHorizontalBinningDispHandle.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_HOR_BIN_DISP, eHInst, NULL);
	eHorizontalBinningDispHandle.fontType = "Normal";
	collumnPosKin += 25;
	collumnPosAccum += 25;
	collumnPosCont += 25;
	/// Image Veritcal Texts
	// Image Top Side Text
	eImageBottomSideTextHandle.kineticSeriesModePos = { 0, collumnPosKin, 140, collumnPosKin + 20 };
	eImageBottomSideTextHandle.continuousSingleScansModePos = eImageBottomSideTextHandle.accumulateModePos = eImageBottomSideTextHandle.kineticSeriesModePos;
	initPos = eImageBottomSideTextHandle.kineticSeriesModePos;
	eImageBottomSideTextHandle.hwnd = CreateWindowEx(0, "STATIC", "Bottommost", WS_CHILD | WS_VISIBLE | SS_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_IMG_TOP_TEXT, eHInst, NULL);
	 eImageBottomSideTextHandle.fontType = "Normal";
	// Image Bottom Side Text
	eImageTopSideTextHandle.kineticSeriesModePos = { 140, collumnPosKin, 280, collumnPosKin + 20 };
	eImageTopSideTextHandle.continuousSingleScansModePos = eImageTopSideTextHandle.accumulateModePos = eImageTopSideTextHandle.kineticSeriesModePos;
	initPos = eImageTopSideTextHandle.kineticSeriesModePos;
	eImageTopSideTextHandle.hwnd = CreateWindowEx(0, "STATIC", "Topmost", WS_CHILD | WS_VISIBLE | SS_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_IMG_BOTTOM_TEXT, eHInst, NULL);
	eImageTopSideTextHandle.fontType = "Normal";
	// Vertical Binning Text
	eVerticalBinningTextHandle.kineticSeriesModePos = { 280, collumnPosKin, 420, collumnPosKin + 20 };
	eVerticalBinningTextHandle.continuousSingleScansModePos = eVerticalBinningTextHandle.accumulateModePos = eVerticalBinningTextHandle.kineticSeriesModePos;
	initPos = eVerticalBinningTextHandle.kineticSeriesModePos;
	eVerticalBinningTextHandle.hwnd = CreateWindowEx(0, "STATIC", "Vertical Bin Size", WS_CHILD | WS_VISIBLE | SS_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_VERTICAL_BIN_TEXT, eHInst, NULL);
	eVerticalBinningTextHandle.fontType = "Normal";
	collumnPosKin += 25;
	collumnPosAccum += 25;
	collumnPosCont += 25;
	/// Image Vertical Edits
	// Image Top Side Edit
	eImageBottomSideEditHandle.kineticSeriesModePos = { 0, collumnPosKin, 140, collumnPosKin + 20 };
	eImageBottomSideEditHandle.continuousSingleScansModePos = eImageBottomSideEditHandle.accumulateModePos = eImageBottomSideEditHandle.kineticSeriesModePos;
	initPos = eImageBottomSideEditHandle.kineticSeriesModePos;
	eImageBottomSideEditHandle.hwnd = CreateWindowEx(0, "EDIT", "1",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_IMG_TOP_EDIT, eHInst, NULL);
	eImageBottomSideEditHandle.fontType = "Normal";
	// Image Bottom Side Edit
	eImageTopSideEditHandle.kineticSeriesModePos = { 140, collumnPosKin, 280, collumnPosKin + 20 };
	eImageTopSideEditHandle.continuousSingleScansModePos = eImageTopSideEditHandle.accumulateModePos = eImageTopSideEditHandle.kineticSeriesModePos;
	initPos = eImageTopSideEditHandle.kineticSeriesModePos;
	eImageTopSideEditHandle.hwnd = CreateWindowEx(0, "EDIT", "5",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_IMG_BOTTOM_EDIT, eHInst, NULL);
	eImageTopSideEditHandle.fontType = "Normal";
	// Vertical Binning Edit
	eVerticalBinningEditHandle.kineticSeriesModePos = { 280, collumnPosKin, 420, collumnPosKin + 20 };
	eVerticalBinningEditHandle.continuousSingleScansModePos = eVerticalBinningEditHandle.accumulateModePos = eVerticalBinningEditHandle.kineticSeriesModePos;
	initPos = eVerticalBinningEditHandle.kineticSeriesModePos;
	eVerticalBinningEditHandle.hwnd = CreateWindowEx(0, "EDIT", "1",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_VERTICAL_BIN_EDIT, eHInst, NULL);
	eVerticalBinningEditHandle.fontType = "Normal";
	collumnPosKin += 25;
	collumnPosAccum += 25;
	collumnPosCont += 25;
	/// Image Veritcal Displays
	// Image Top Side Display
	eImageBottomSideDispHandle.kineticSeriesModePos = { 0, collumnPosKin, 140, collumnPosKin + 20 };
	eImageBottomSideDispHandle.continuousSingleScansModePos = eImageBottomSideDispHandle.accumulateModePos = eImageBottomSideDispHandle.kineticSeriesModePos;
	initPos = eImageBottomSideDispHandle.kineticSeriesModePos;
	eImageBottomSideDispHandle.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_IMG_TOP_DISP, eHInst, NULL);
	eImageBottomSideDispHandle.fontType = "Normal";
	// Image Bottom Side Display
	eImageTopSideDispHandle.kineticSeriesModePos = { 140, collumnPosKin, 280, collumnPosKin + 20 };
	eImageTopSideDispHandle.continuousSingleScansModePos = eImageTopSideDispHandle.accumulateModePos = eImageTopSideDispHandle.kineticSeriesModePos;
	initPos = eImageTopSideDispHandle.kineticSeriesModePos;
	eImageTopSideDispHandle.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_IMG_BOTTOM_DISP, eHInst, NULL);
	eImageTopSideDispHandle.fontType = "Normal";
	// Vertical Binning Display
	eVerticalBinningDispHandle.kineticSeriesModePos = { 280, collumnPosKin, 420, collumnPosKin + 20 };
	eVerticalBinningDispHandle.continuousSingleScansModePos = eVerticalBinningDispHandle.accumulateModePos = eVerticalBinningDispHandle.kineticSeriesModePos;
	initPos = eVerticalBinningDispHandle.kineticSeriesModePos;
	eVerticalBinningDispHandle.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_VERTICAL_BIN_DISP, eHInst, NULL);
	eVerticalBinningDispHandle.fontType = "Normal";
	collumnPosKin += 25;
	collumnPosAccum += 25;
	collumnPosCont += 25;

	/// KINETIC CYCLE TIME
	// Set Kinetic Series Cycle Time Button
	eSetKineticSeriesCycleTimeButtonHandle.kineticSeriesModePos = { 0, collumnPosKin, 220, collumnPosKin + 25 };
	eSetKineticSeriesCycleTimeButtonHandle.continuousSingleScansModePos = { -1,-1,-1,-1 };
	eSetKineticSeriesCycleTimeButtonHandle.accumulateModePos = { -1,-1,-1,-1 };
	initPos = eSetKineticSeriesCycleTimeButtonHandle.kineticSeriesModePos;
	eSetKineticSeriesCycleTimeButtonHandle.hwnd = CreateWindowEx(0, "BUTTON", "Set Kinetic Cycle Time (ms)",
														   WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_SET_KINETIC_CYCLE_TIME_BUTTON, eHInst, NULL);
	eSetKineticSeriesCycleTimeButtonHandle.fontType = "Normal";
	// Kinetic Cycle Time Edit
	eKineticCycleTimeEditHandle.kineticSeriesModePos = { 225, collumnPosKin, 320, collumnPosKin + 25 };
	eKineticCycleTimeEditHandle.continuousSingleScansModePos = { -1,-1,-1,-1 };
	eKineticCycleTimeEditHandle.accumulateModePos = { -1,-1,-1,-1 };
	initPos = eKineticCycleTimeEditHandle.kineticSeriesModePos;
	eKineticCycleTimeEditHandle.hwnd = CreateWindowEx(0, "EDIT", "500",
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_KINETIC_CYCLE_TIME_EDIT, eHInst, NULL);
	eKineticCycleTimeEditHandle.fontType = "Normal";
	// Kinetic Cycle Time Display
	eKineticCycleTimeDispHandle.kineticSeriesModePos = { 325, collumnPosKin, 420, collumnPosKin + 25 };
	eKineticCycleTimeDispHandle.continuousSingleScansModePos = { -1,-1,-1,-1 };
	eKineticCycleTimeDispHandle.accumulateModePos = { -1,-1,-1,-1 };
	initPos = eKineticCycleTimeDispHandle.kineticSeriesModePos;
	eKineticCycleTimeDispHandle.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_KINETIC_CYCLE_TIME_DISP, eHInst, NULL);
	eKineticCycleTimeDispHandle.fontType = "Normal";
	collumnPosKin += 30;

	/// PICTURES PER STACK
	// Set Pictures Per Stack Button
	eSetNumberOfExperimentsPerStackButtonHandle.kineticSeriesModePos = { 0, collumnPosKin, 220, collumnPosKin + 25 };
	eSetNumberOfExperimentsPerStackButtonHandle.continuousSingleScansModePos = { -1,-1,-1,-1 };
	eSetNumberOfExperimentsPerStackButtonHandle.accumulateModePos = { -1,-1,-1,-1 };
	initPos = eSetNumberOfExperimentsPerStackButtonHandle.kineticSeriesModePos;
	eSetNumberOfExperimentsPerStackButtonHandle.hwnd = CreateWindowEx(0, "BUTTON", "Set Experiments / Stack",
													   WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_SET_EXPERIMENTS_PER_STACK_BUTTON, eHInst, NULL);
	eSetNumberOfExperimentsPerStackButtonHandle.fontType = "Normal";
	// Pictures Per STack Edit
	eExperimentsPerStackEditHandle.kineticSeriesModePos = { 225, collumnPosKin, 320, collumnPosKin + 25 };
	eExperimentsPerStackEditHandle.continuousSingleScansModePos = { -1,-1,-1,-1 };
	eExperimentsPerStackEditHandle.accumulateModePos = { -1,-1,-1,-1 };
	initPos = eExperimentsPerStackEditHandle.kineticSeriesModePos;
	eExperimentsPerStackEditHandle.hwnd = CreateWindowEx(0, "EDIT", "10",
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_SCAN_NUM_EDIT, eHInst, NULL);
	eExperimentsPerStackEditHandle.fontType = "Normal";
	// Pictures Per Stack Display
	eExperimentsPerStackDispHandle.kineticSeriesModePos = { 325, collumnPosKin, 420, collumnPosKin + 25 };
	eExperimentsPerStackDispHandle.continuousSingleScansModePos = { -1,-1,-1,-1 };
	eExperimentsPerStackDispHandle.accumulateModePos = { -1,-1,-1,-1 };
	initPos = eExperimentsPerStackDispHandle.kineticSeriesModePos;
	eExperimentsPerStackDispHandle.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_SCAN_NUM_DISP, eHInst, NULL);
	eExperimentsPerStackDispHandle.fontType = "Normal";
	
	collumnPosKin += 30;

	/// ATOM THRESHOLD
	// Set Atom Threshold Button
	eSetAtomThresholdButtonHandle.kineticSeriesModePos = { 0, collumnPosKin, 220, collumnPosKin + 25 };
	eSetAtomThresholdButtonHandle.continuousSingleScansModePos = { 0, collumnPosCont, 220, collumnPosCont + 25 };
	eSetAtomThresholdButtonHandle.accumulateModePos = { 0, collumnPosAccum, 220, collumnPosAccum + 25 };
	initPos = eSetAtomThresholdButtonHandle.kineticSeriesModePos;
	eSetAtomThresholdButtonHandle.hwnd = CreateWindowEx(0, "BUTTON", "Set Atom Threshold",
													   WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_SET_DETECTION_THRESHOLD_BUTTON, eHInst, NULL);
	eSetAtomThresholdButtonHandle.fontType = "Normal";
	// Atom Threshold Edit
	eAtomThresholdEditHandle.kineticSeriesModePos = { 225, collumnPosKin, 320, collumnPosKin + 25 };
	eAtomThresholdEditHandle.continuousSingleScansModePos = { 225, collumnPosCont, 320, collumnPosCont + 25 };
	eAtomThresholdEditHandle.accumulateModePos = { 225, collumnPosAccum, 320, collumnPosAccum + 25 };
	initPos = eAtomThresholdEditHandle.kineticSeriesModePos;
	eAtomThresholdEditHandle.hwnd = CreateWindowEx(0, "EDIT", "100",
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_DETECTION_THRESHOLD_EDIT_HANDLE, eHInst, NULL);
	eAtomThresholdEditHandle.fontType = "Normal";
	// Atom Threshold Display
	eAtomThresholdDispHandle.kineticSeriesModePos = { 325, collumnPosKin, 420, collumnPosKin + 25 };
	eAtomThresholdDispHandle.continuousSingleScansModePos = { 325, collumnPosCont, 420, collumnPosCont + 25 };
	eAtomThresholdDispHandle.accumulateModePos = { 325, collumnPosAccum, 420, collumnPosAccum + 25 };
	initPos = eAtomThresholdDispHandle.kineticSeriesModePos;
	eAtomThresholdDispHandle.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_DETECTION_THRESHOLD_DISP_HANDLE, eHInst, NULL);
	eAtomThresholdDispHandle.fontType = "Normal";
	collumnPosKin += 30;
	collumnPosAccum += 30;
	collumnPosCont += 30;

	/// ACCUMULATION STACK NUMBER
	// Set Accumulation Stack Number Button
	eSetAccumulationStackNumberButtonHandle.kineticSeriesModePos = { 0, collumnPosKin, 220, collumnPosKin + 25 };
	eSetAccumulationStackNumberButtonHandle.continuousSingleScansModePos = { -1,-1,-1,-1 };
	eSetAccumulationStackNumberButtonHandle.accumulateModePos = { -1,-1,-1,-1 };
	initPos = eSetAccumulationStackNumberButtonHandle.kineticSeriesModePos;
	eSetAccumulationStackNumberButtonHandle.hwnd = CreateWindowEx(0, "BUTTON", "Set # Accumulation Stacks",
															WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_SET_PICTURE_SUB_SERIES_NUMBER_BUTTON, eHInst, NULL);
	eSetAccumulationStackNumberButtonHandle.fontType = "Normal";
	// Accumulation Stack # Edit
	ePictureSubSeriesNumberEditHandle.kineticSeriesModePos = { 225, collumnPosKin, 320, collumnPosKin + 25 };
	ePictureSubSeriesNumberEditHandle.continuousSingleScansModePos = { -1,-1,-1,-1 };
	ePictureSubSeriesNumberEditHandle.accumulateModePos = { -1,-1,-1,-1 };
	initPos = ePictureSubSeriesNumberEditHandle.kineticSeriesModePos;
	ePictureSubSeriesNumberEditHandle.hwnd = CreateWindowEx(0, "EDIT", "1",
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_ACCUMULATION_STACK_EDIT_HANDLE, eHInst, NULL);
	ePictureSubSeriesNumberEditHandle.fontType = "Normal";
	// Accumulation Stack Number Display
	ePictureSubSeriesNumberDispHandle.kineticSeriesModePos = { 325, collumnPosKin, 420, collumnPosKin + 25 };
	ePictureSubSeriesNumberDispHandle.continuousSingleScansModePos = { -1,-1,-1,-1 };
	ePictureSubSeriesNumberDispHandle.accumulateModePos = { -1,-1,-1,-1 };
	initPos = ePictureSubSeriesNumberDispHandle.kineticSeriesModePos;
	ePictureSubSeriesNumberDispHandle.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_ACCUMULATION_STACK_DISP_HANDLE, eHInst, NULL);
	ePictureSubSeriesNumberDispHandle.fontType = "Normal";
	collumnPosKin += 30;

	/// PICTURES PER EXPERIMENT
	// Set Pictures Per Experiment Button
	eSetPicturesPerExperimentButtonHandle.kineticSeriesModePos = { 0, collumnPosKin, 220, collumnPosKin + 25 };
	eSetPicturesPerExperimentButtonHandle.continuousSingleScansModePos = { 0, collumnPosCont, 220, collumnPosCont + 25 };
	eSetPicturesPerExperimentButtonHandle.accumulateModePos = { 0, collumnPosAccum, 220, collumnPosAccum + 25 };
	initPos = eSetPicturesPerExperimentButtonHandle.kineticSeriesModePos;
	eSetPicturesPerExperimentButtonHandle.hwnd = CreateWindowEx(0, "BUTTON", "Set Pictures / Experiment",
														  WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_SET_PICTURES_PER_EXPERIMENT_BUTTON, eHInst, NULL);
	eSetPicturesPerExperimentButtonHandle.fontType = "Normal";
	// Pictures Per Experiment Edit
	ePicturesPerExperimentEditHandle.kineticSeriesModePos = { 225, collumnPosKin, 320, collumnPosKin + 25 };
	ePicturesPerExperimentEditHandle.continuousSingleScansModePos = { 225, collumnPosCont, 320, collumnPosCont + 25 };
	ePicturesPerExperimentEditHandle.accumulateModePos = { 225, collumnPosAccum, 320, collumnPosAccum + 25 };
	initPos = ePicturesPerExperimentEditHandle.kineticSeriesModePos;
	ePicturesPerExperimentEditHandle.hwnd = CreateWindowEx(0, "EDIT", "2",
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_PICTURES_PER_EXPERIMENT_EDIT, eHInst, NULL);
	ePicturesPerExperimentEditHandle.fontType = "Normal";
	// Pictures Per Experiment Display
	ePicturesPerExperimentDispHandle.kineticSeriesModePos = { 325, collumnPosKin, 420, collumnPosKin + 25 };
	ePicturesPerExperimentDispHandle.continuousSingleScansModePos = { 325, collumnPosCont, 420, collumnPosCont + 25 };
ePicturesPerExperimentDispHandle.accumulateModePos = { 325, collumnPosAccum, 420, collumnPosAccum + 25 };
	initPos = ePicturesPerExperimentDispHandle.kineticSeriesModePos;
	ePicturesPerExperimentDispHandle.hwnd = CreateWindowEx(0, "EDIT", "",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_PICTURES_PER_EXPERIMENT_DISP, eHInst, NULL);
	ePicturesPerExperimentDispHandle.fontType = "Normal";
	collumnPosKin += 30;
	collumnPosAccum += 30;
	collumnPosCont += 30;
	/// ACCUMULATION TIME
	// Set Accumulation Time Button
	eSetAccumulationTimeButton.kineticSeriesModePos = { -1, -1, -1, -1 };
	eSetAccumulationTimeButton.continuousSingleScansModePos = { -1, -1, -1, -1 };
	eSetAccumulationTimeButton.accumulateModePos = { 0, collumnPosAccum, 220, collumnPosAccum + 25 };
	initPos = eSetAccumulationTimeButton.kineticSeriesModePos;
	eSetAccumulationTimeButton.hwnd = CreateWindowEx(0, "BUTTON", "Set Accumulation Time",
		WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_SET_ACCUMULATION_TIME_BUTTON, eHInst, NULL);
	ShowWindow(eSetAccumulationTimeButton.hwnd, SW_HIDE);
	eSetAccumulationTimeButton.fontType = "Normal";
	// Accumulation Time Edit
	eAccumulationTimeEdit.kineticSeriesModePos = { -1, -1, -1, -1 };
	eAccumulationTimeEdit.continuousSingleScansModePos = { -1, -1, -1, -1 };
	eAccumulationTimeEdit.accumulateModePos = { 225, collumnPosAccum, 320, collumnPosAccum + 25 };
	initPos = eAccumulationTimeEdit.kineticSeriesModePos;
	eAccumulationTimeEdit.hwnd = CreateWindowEx(0, "EDIT", "1",
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, 
		cameraWindow, (HMENU)IDC_ACCUMULATION_TIME_EDIT, eHInst, NULL);
	ShowWindow(eSetAccumulationTimeButton.hwnd, SW_HIDE);
	eAccumulationTimeEdit.fontType = "Normal";
	// Accumulation Time Display
	eAccumulationTimeDisp.kineticSeriesModePos = { -1, -1, -1, -1 };
	eAccumulationTimeDisp.continuousSingleScansModePos = { -1, -1, -1, -1 };
	eAccumulationTimeDisp.accumulateModePos = { 325, collumnPosAccum, 420, collumnPosAccum + 25 };
	initPos = eAccumulationTimeDisp.kineticSeriesModePos;
	eAccumulationTimeDisp.hwnd = CreateWindowEx(0, "EDIT", "",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, 
		cameraWindow, (HMENU)IDC_ACCUMULATION_TIME_DISP, eHInst, NULL);
	ShowWindow(eAccumulationTimeDisp.hwnd, SW_HIDE);
	eAccumulationTimeDisp.fontType = "Normal";
	collumnPosAccum += 30;
	/// ACCUMULATION NUMBER
	// Set Accumulation Number Button
	eSetAccumulationNumberButton.kineticSeriesModePos = { -1, -1, -1, -1 };
	eSetAccumulationNumberButton.continuousSingleScansModePos = { -1, -1, -1, -1 };
	eSetAccumulationNumberButton.accumulateModePos = { 0, collumnPosAccum, 220, collumnPosAccum + 25 };
	initPos = eSetAccumulationNumberButton.kineticSeriesModePos;
	eSetAccumulationNumberButton.hwnd = CreateWindowEx(0, "BUTTON", "Set # of Accumulations",
		WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_SET_ACCUMULATION_NUMBER_BUTTON, eHInst, NULL);
	ShowWindow(eSetAccumulationNumberButton.hwnd, SW_HIDE);
	eSetAccumulationNumberButton.fontType = "Normal";
	// Accumulation Time Edit
	eSetAccumulationNumberEdit.kineticSeriesModePos = { -1, -1, -1, -1 };
	eSetAccumulationNumberEdit.continuousSingleScansModePos = { -1, -1, -1, -1 };
	eSetAccumulationNumberEdit.accumulateModePos = { 225, collumnPosAccum, 320, collumnPosAccum + 25 };
	initPos = eSetAccumulationNumberEdit.kineticSeriesModePos;
	eSetAccumulationNumberEdit.hwnd = CreateWindowEx(0, "EDIT", "1",
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_ACCUMULATION_NUMBER_EDIT, eHInst, NULL);
	ShowWindow(eSetAccumulationNumberEdit.hwnd, SW_HIDE);
	eSetAccumulationNumberEdit.fontType = "Normal";
	// Accumulation Time Display
	eSetAccumulationNumberDisp.kineticSeriesModePos = { -1, -1, -1, -1 };
	eSetAccumulationNumberDisp.continuousSingleScansModePos = { -1, -1, -1, -1 };
	eSetAccumulationNumberDisp.accumulateModePos = { 325, collumnPosAccum, 420, collumnPosAccum + 25 };
	initPos = eSetAccumulationNumberDisp.kineticSeriesModePos;
	eSetAccumulationNumberDisp.hwnd = CreateWindowEx(0, "EDIT", "",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_ACCUMULATION_NUMBER_DISP, eHInst, NULL);
	ShowWindow(eSetAccumulationNumberDisp.hwnd, SW_HIDE);
	eSetAccumulationNumberDisp.fontType = "Normal";
	collumnPosAccum += 30;
	/// RUNNING AVERAGE OPTIONS
	// Set Number of Runs to Average
	eSetRunningAverageNumberToAverageButton.kineticSeriesModePos = { -1, -1, -1, -1 };
	eSetRunningAverageNumberToAverageButton.continuousSingleScansModePos = { 0, collumnPosCont, 220, collumnPosCont + 25 };
	eSetRunningAverageNumberToAverageButton.accumulateModePos = { -1, -1, -1, -1 };
	initPos = eSetRunningAverageNumberToAverageButton.kineticSeriesModePos;
	eSetRunningAverageNumberToAverageButton.hwnd = CreateWindowEx(0, "BUTTON", "Set # of Pictures to Average",
		WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_RUNNING_AVERAGE_BUTTON, eHInst, NULL);
	ShowWindow(eSetRunningAverageNumberToAverageButton.hwnd, SW_HIDE);
	eSetRunningAverageNumberToAverageButton.fontType = "Normal";
	// Running Average Edit
	eRunningAverageEdit.kineticSeriesModePos = { -1, -1, -1, -1 };
	eRunningAverageEdit.continuousSingleScansModePos = { 225, collumnPosCont, 320, collumnPosCont + 25 };
	eRunningAverageEdit.accumulateModePos = { -1, -1, -1, -1 };
	initPos = eRunningAverageEdit.kineticSeriesModePos;
	eRunningAverageEdit.hwnd = CreateWindowEx(0, "EDIT", "5",
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_RUNNING_AVERAGE_EDIT, eHInst, NULL);
	ShowWindow(eRunningAverageEdit.hwnd, SW_HIDE);
	eRunningAverageEdit.fontType = "Normal";
	// Running Average Edit Display
	eRunningAverageDisp.kineticSeriesModePos = { -1, -1, -1, -1 };
	eRunningAverageDisp.continuousSingleScansModePos = { 325, collumnPosCont, 420, collumnPosCont + 25 };
	eRunningAverageDisp.accumulateModePos = { -1, -1, -1, -1 };
	initPos = eRunningAverageDisp.kineticSeriesModePos;
	eRunningAverageDisp.hwnd = CreateWindowEx(0, "EDIT", "",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_RUNNING_AVERAGE_DISP, eHInst, NULL);
	ShowWindow(eRunningAverageDisp.hwnd, SW_HIDE);
	eRunningAverageDisp.fontType = "Normal";



	/// PLOTTING FREQUENCY CONTROLS
	// Set Plotting Frequency
	eSetPlottingFrequencyButton.kineticSeriesModePos = { 0, collumnPosKin, 220, collumnPosKin + 25 };
	eSetPlottingFrequencyButton.continuousSingleScansModePos = { 0, collumnPosCont, 220, collumnPosCont + 25 };
	eSetPlottingFrequencyButton.accumulateModePos = { 0, collumnPosAccum, 220, collumnPosAccum + 25 };
	initPos = eSetPlottingFrequencyButton.kineticSeriesModePos;
	eSetPlottingFrequencyButton.hwnd = CreateWindowEx(0, "BUTTON", "Set Plotting Frequency",
		WS_CHILD | WS_VISIBLE | WS_BORDER | BS_PUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_PLOTTING_FREQUENCY_BUTTON, eHInst, NULL);
	eSetPlottingFrequencyButton.fontType = "Normal";
	// Plotting Frequency Edit
	ePlottingFrequencyEdit.kineticSeriesModePos = { 225, collumnPosKin, 320, collumnPosKin + 25 };
	ePlottingFrequencyEdit.continuousSingleScansModePos = { 225, collumnPosCont, 320, collumnPosCont + 25 };
	ePlottingFrequencyEdit.accumulateModePos = { 225, collumnPosAccum, 320, collumnPosAccum + 25 };
	initPos = ePlottingFrequencyEdit.kineticSeriesModePos;
	ePlottingFrequencyEdit.hwnd = CreateWindowEx(0, "EDIT", "1",
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_PLOTTING_FREQUENCY_EDIT, eHInst, NULL);
	ePlottingFrequencyEdit.fontType = "Normal";
	// Plotting Frequency Display
	ePlottingFrequencyDisp.kineticSeriesModePos = { 325, collumnPosKin, 420, collumnPosKin + 25 };
	ePlottingFrequencyDisp.continuousSingleScansModePos = { 325, collumnPosCont, 420, collumnPosCont + 25 };
	ePlottingFrequencyDisp.accumulateModePos = { 325, collumnPosAccum, 420, collumnPosAccum + 25 };
	initPos = ePlottingFrequencyDisp.kineticSeriesModePos;
	ePlottingFrequencyDisp.hwnd = CreateWindowEx(0, "EDIT", "1",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_PLOTTING_FREQUENCY_DISP, eHInst, NULL);
	ePlottingFrequencyDisp.fontType = "Normal";
	collumnPosKin += 30;
	collumnPosAccum += 30;
	collumnPosCont += 30;

	/// Increment Data File Names Box
	eIncDataFileOptionBoxHandle.kineticSeriesModePos = { 0, collumnPosKin, 420, collumnPosKin + 20 };
	eIncDataFileOptionBoxHandle.continuousSingleScansModePos = { 0, collumnPosCont, 420, collumnPosCont + 20 };
	eIncDataFileOptionBoxHandle.accumulateModePos = { 0, collumnPosAccum, 420, collumnPosAccum + 20 };
	initPos = eIncDataFileOptionBoxHandle.kineticSeriesModePos;
	eIncDataFileOptionBoxHandle.hwnd = CreateWindowEx(0, "BUTTON", "Increment Data File Name",
												WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, 
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow,
												(HMENU)IDC_INCREMENT_FILE_OPTION_BUTTON, eHInst, NULL);
	eIncDataFileOptionBoxHandle.fontType = "Normal";
	SendMessage(eIncDataFileOptionBoxHandle.hwnd, BM_SETCHECK, BST_UNCHECKED, NULL);
	eIncSaveFileNameOption = false;
	collumnPosKin += 25;
	collumnPosAccum += 25;
	collumnPosCont += 25;
	/// PLOTTING 2.0 ////////////////////
	/// All plots
	// all plots Text
	eAllPlotsText.kineticSeriesModePos = { 0, collumnPosKin, 100, collumnPosKin + 25 };
	eAllPlotsText.continuousSingleScansModePos = { 0, collumnPosCont, 100, collumnPosCont + 25 };
	eAllPlotsText.accumulateModePos = { 0, collumnPosAccum, 100, collumnPosAccum + 25 };
	initPos = eAllPlotsText.kineticSeriesModePos;
	eAllPlotsText.hwnd = CreateWindowEx(0, "STATIC", "All Plots:", WS_CHILD | WS_VISIBLE | SS_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_PLOTTING_TEXT, eHInst, NULL);
	eAllPlotsText.fontType = "Normal";
	// all plots combo
	eAllPlotsCombo.kineticSeriesModePos = { 100, collumnPosKin, 420, collumnPosKin + 800 };
	eAllPlotsCombo.continuousSingleScansModePos = { 100, collumnPosCont, 420, collumnPosCont + 25 };
	eAllPlotsCombo.accumulateModePos = { 100, collumnPosAccum, 420, collumnPosAccum + 800 };
	initPos = eAllPlotsCombo.kineticSeriesModePos;
	eAllPlotsCombo.hwnd = CreateWindowEx(0, "COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_PLOTTING_COMBO, eHInst, NULL);
	// set options for the combo
	eAllPlotsCombo.fontType = "Normal";
	fileManage::reloadCombo(eAllPlotsCombo.hwnd, PLOT_FILES_SAVE_LOCATION, "*.plot", "__NONE__");
	collumnPosKin += 30;
	collumnPosAccum += 30;
	collumnPosCont += 30;

	/// Current Plots
	// current plots Text
	eCurrentPlotsText.kineticSeriesModePos = { 0, collumnPosKin, 100, collumnPosKin + 25 };
	eCurrentPlotsText.continuousSingleScansModePos = { 0, collumnPosCont, 100, collumnPosCont + 25 };
	eCurrentPlotsText.accumulateModePos = { 0, collumnPosAccum, 100, collumnPosAccum + 25 };
	initPos = eCurrentPlotsText.kineticSeriesModePos;
	eCurrentPlotsText.hwnd = CreateWindowEx(0, "STATIC", "Current Plots:", WS_CHILD | WS_VISIBLE | SS_LEFT,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_CURRENT_PLOTS_TEXT, eHInst, NULL);
	eCurrentPlotsText.fontType = "Normal";
	// current Plots Combo
	eCurrentPlotsCombo.kineticSeriesModePos = { 100, collumnPosKin, 420, collumnPosKin + 800 };
	eCurrentPlotsCombo.continuousSingleScansModePos = { 100, collumnPosCont, 420, collumnPosCont + 800 };
	eCurrentPlotsCombo.accumulateModePos = { 100, collumnPosAccum, 420, collumnPosAccum + 800 };
	initPos = eCurrentPlotsCombo.kineticSeriesModePos;
	eCurrentPlotsCombo.hwnd = CreateWindowEx(0, "COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_CURRENT_PLOTS_COMBO, eHInst, NULL);
	// set options for the combo
	eCurrentPlotsCombo.fontType = "Normal";
	collumnPosKin += 30;
	collumnPosAccum += 30;
	collumnPosCont += 30;

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

	/// PROGRESS BARS
	// subseries progress bar
	eSubSeriesProgressBar.accumulateModePos = eSubSeriesProgressBar.continuousSingleScansModePos = eSubSeriesProgressBar.kineticSeriesModePos = { 752, 480, 1920, 495 };
	initPos = eSubSeriesProgressBar.kineticSeriesModePos;

	eSubSeriesProgressBar.hwnd = CreateWindowEx(0, PROGRESS_CLASS, 0, WS_CHILD | WS_VISIBLE,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_SUBSERIES_PROGRESS_BAR, eHInst, NULL);
	SetWindowTheme(eSubSeriesProgressBar.hwnd, 0, 0);
	SendMessage(eSubSeriesProgressBar.hwnd, (UINT)PBM_SETBKCOLOR, 0, RGB(100, 110, 100));
	SendMessage(eSubSeriesProgressBar.hwnd, (UINT)PBM_SETBARCOLOR, 0, (LPARAM)RGB(0, 200, 0));
	// series progress bar display
	eSeriesProgressBar.accumulateModePos = eSeriesProgressBar.continuousSingleScansModePos = eSeriesProgressBar.kineticSeriesModePos = { 752, 495, 1920, 520 };
	initPos = eSeriesProgressBar.kineticSeriesModePos;
	eSeriesProgressBar.hwnd = CreateWindowEx(0, PROGRESS_CLASS, 0, WS_CHILD | WS_VISIBLE,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_SERIES_PROGRESS_BAR, eHInst, NULL);
	SetWindowTheme(eSeriesProgressBar.hwnd, 0, 0);
	SendMessage(eSeriesProgressBar.hwnd, (UINT)PBM_SETBKCOLOR, 0, RGB(100, 110, 100));
	SendMessage(eSeriesProgressBar.hwnd, (UINT)PBM_SETBARCOLOR, 0, (LPARAM)RGB(255, 255, 255));
	/// STATUS
	// Clear Status Button
	eClearStatusButtonHandle.kineticSeriesModePos = { 0, 720, 50, 745 };
	eClearStatusButtonHandle.continuousSingleScansModePos = eClearStatusButtonHandle.accumulateModePos = eClearStatusButtonHandle.kineticSeriesModePos;
	initPos = eClearStatusButtonHandle.kineticSeriesModePos;
	eClearStatusButtonHandle.hwnd = CreateWindowEx(0, "BUTTON", "Clear", WS_CHILD | WS_VISIBLE,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_CLEAR_STATUS_BUTTON, eHInst, NULL);
	eClearStatusButtonHandle.fontType = "Normal";
	// Status Edit Handle
	eStatusEditHandle.kineticSeriesModePos = { 0, 745, 480, 1000 };
	eStatusEditHandle.continuousSingleScansModePos = eStatusEditHandle.accumulateModePos = eStatusEditHandle.kineticSeriesModePos;
	initPos = eStatusEditHandle.kineticSeriesModePos;
	eStatusEditHandle.hwnd = CreateWindowEx(0, "EDIT", "",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_MULTILINE | WS_VSCROLL | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_STATUS_EDIT, eHInst, NULL);
	eStatusEditHandle.fontType = "Normal";

	// ~2X the default text limit. As high as possible without a rich edit control.
	SendMessage(eStatusEditHandle.hwnd, EM_LIMITTEXT, 64000, 0);
	/// ERROR
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
