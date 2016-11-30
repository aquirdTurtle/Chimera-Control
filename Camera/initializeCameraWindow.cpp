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
	POINT secondColumnPointKinetic = { 480, 0 }, secondColumnPointAccumulate = { 480, 0 }, 
		secondColumnPointContinuous = { 480, 0 };

	///  /// Initialize all controls and HWNDs for this window. ///  ///
	InitCommonControlsEx(0);
	ePicStats.initialize(secondColumnPointKinetic, cameraWindow);
	eTextingHandler.initializeControls(secondColumnPointKinetic, secondColumnPointAccumulate, 
		secondColumnPointContinuous, cameraWindow, false);
	/// Image stuffs
	eImageControl.initiateControls(secondColumnPointKinetic, secondColumnPointAccumulate, secondColumnPointContinuous, 
		cameraWindow, false);
	eAlerts.initialize(secondColumnPointKinetic, secondColumnPointAccumulate, secondColumnPointContinuous, 
		cameraWindow, false);

	/// Filesystem Controls
	eCameraFileSystem.initializeControls(collumnPosKin, collumnPosAccum, collumnPosCont, cameraWindow, false);
	//
	RECT initPos;
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

	collumnPosKin.y += 20;
	collumnPosAccum.y += 20;
	collumnPosCont.y += 20;

	/// Most picture options
	int id_in = 7165;
	ePictureOptionsControl.initialize(collumnPosKin, collumnPosAccum, collumnPosCont, cameraWindow, id_in);

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

	collumnPosKin.y += 25;

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
	collumnPosKin.y += 25;

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
	eTriggerComboHandle.kineticSeriesModePos = { 245, collumnPosKin.y, 480, collumnPosKin.y + 800 };
	eTriggerComboHandle.continuousSingleScansModePos = { 245, collumnPosCont.y, 480, collumnPosCont.y + 800 };
	eTriggerComboHandle.accumulateModePos = { 245, collumnPosAccum.y, 480, collumnPosAccum.y + 800 };
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
	collumnPosKin.y += 25;
	collumnPosAccum.y += 25;
	collumnPosCont.y += 25;
	eCurrentTriggerMode = "External";

	/// autoanalysis
	eAutoAnalysisHandler.initializeControls(collumnPosKin, collumnPosAccum, collumnPosCont, cameraWindow, true);

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
	eKineticCycleTimeEditHandle.kineticSeriesModePos = { 220, collumnPosKin.y, 320, collumnPosKin.y + 25 };
	eKineticCycleTimeEditHandle.continuousSingleScansModePos = { -1,-1,-1,-1 };
	eKineticCycleTimeEditHandle.accumulateModePos = { -1,-1,-1,-1 };
	initPos = eKineticCycleTimeEditHandle.kineticSeriesModePos;
	eKineticCycleTimeEditHandle.triggerModeSensitive = -1;
	eKineticCycleTimeEditHandle.hwnd = CreateWindowEx(0, "EDIT", "500",
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_KINETIC_CYCLE_TIME_EDIT, eHInst, NULL);
	eKineticCycleTimeEditHandle.fontType = "Normal";
	// Kinetic Cycle Time Display
	eKineticCycleTimeDispHandle.kineticSeriesModePos = { 320, collumnPosKin.y, 480, collumnPosKin.y + 25 };
	eKineticCycleTimeDispHandle.continuousSingleScansModePos = { -1,-1,-1,-1 };
	eKineticCycleTimeDispHandle.accumulateModePos = { -1,-1,-1,-1 };
	initPos = eKineticCycleTimeDispHandle.kineticSeriesModePos;
	eKineticCycleTimeDispHandle.triggerModeSensitive = -1;
	eKineticCycleTimeDispHandle.hwnd = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_KINETIC_CYCLE_TIME_DISP, eHInst, NULL);
	eKineticCycleTimeDispHandle.fontType = "Normal";
	collumnPosKin.y += 25;

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
	collumnPosAccum.y += 25;
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
	collumnPosAccum.y += 25;

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
	ePlottingFrequencyEdit.kineticSeriesModePos = { 220, collumnPosKin.y, 320, collumnPosKin.y + 25 };
	ePlottingFrequencyEdit.continuousSingleScansModePos = { 220, collumnPosCont.y, 320, collumnPosCont.y + 25 };
	ePlottingFrequencyEdit.accumulateModePos = { 220, collumnPosAccum.y, 320, collumnPosAccum.y + 25 };
	initPos = ePlottingFrequencyEdit.kineticSeriesModePos;
	ePlottingFrequencyEdit.triggerModeSensitive = 1;
	ePlottingFrequencyEdit.hwnd = CreateWindowEx(0, "EDIT", "1",
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_PLOTTING_FREQUENCY_EDIT, eHInst, NULL);
	ePlottingFrequencyEdit.fontType = "Normal";
	// Plotting Frequency Display
	ePlottingFrequencyDisp.kineticSeriesModePos = { 320, collumnPosKin.y, 480, collumnPosKin.y + 25 };
	ePlottingFrequencyDisp.continuousSingleScansModePos = { 320, collumnPosCont.y, 480, collumnPosCont.y + 25 };
	ePlottingFrequencyDisp.accumulateModePos = { 320, collumnPosAccum.y, 480, collumnPosAccum.y + 25 };
	initPos = ePlottingFrequencyDisp.kineticSeriesModePos;
	ePlottingFrequencyDisp.triggerModeSensitive = 1;
	ePlottingFrequencyDisp.hwnd = CreateWindowEx(0, "EDIT", "1",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		cameraWindow, (HMENU)IDC_PLOTTING_FREQUENCY_DISP, eHInst, NULL);
	ePlottingFrequencyDisp.fontType = "Normal";
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
	eAllPlotsCombo.kineticSeriesModePos = { 100, collumnPosKin.y, 480, collumnPosKin.y + 800 };
	eAllPlotsCombo.continuousSingleScansModePos = { 100, collumnPosCont.y, 480, collumnPosCont.y + 25 };
	eAllPlotsCombo.accumulateModePos = { 100, collumnPosAccum.y, 480, collumnPosAccum.y + 800 };
	initPos = eAllPlotsCombo.kineticSeriesModePos;
	eAllPlotsCombo.triggerModeSensitive = 1;
	eAllPlotsCombo.hwnd = CreateWindowEx(0, "COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_PLOTTING_COMBO, eHInst, NULL);
	// set options for the combo
	eAllPlotsCombo.fontType = "Normal";
	fileManage::reloadCombo(eAllPlotsCombo.hwnd, PLOT_FILES_SAVE_LOCATION, "*.plot", "__NONE__");
	collumnPosKin.y += 25;
	collumnPosAccum.y += 25;
	collumnPosCont.y += 25;

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
	eCurrentPlotsCombo.kineticSeriesModePos = { 100, collumnPosKin.y, 480, collumnPosKin.y + 800 };
	eCurrentPlotsCombo.continuousSingleScansModePos = { 100, collumnPosCont.y, 480, collumnPosCont.y + 800 };
	eCurrentPlotsCombo.accumulateModePos = { 100, collumnPosAccum.y, 480, collumnPosAccum.y + 800 };
	initPos = eCurrentPlotsCombo.kineticSeriesModePos;
	eCurrentPlotsCombo.triggerModeSensitive = 1;
	eCurrentPlotsCombo.hwnd = CreateWindowEx(0, "COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, cameraWindow, (HMENU)IDC_CURRENT_PLOTS_COMBO, eHInst, NULL);
	// set options for the combo
	eCurrentPlotsCombo.fontType = "Normal";
	collumnPosKin.y += 25;
	collumnPosAccum.y += 25;
	collumnPosCont.y += 25;

	
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
