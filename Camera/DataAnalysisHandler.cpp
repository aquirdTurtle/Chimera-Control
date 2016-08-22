#include "stdafx.h"
#include "DataAnalysisHandler.h"
#include "constants.h"
#include "reorganizeControl.h"
#include "dialogProcedures.h"
#include "Python.h"
#include <process.h>
#include "appendText.h"
#include "myAndor.h"
#include "frameobject.h"
#include "EmbeddedPythonHandler.h"

bool DataAnalysisHandler::reorganizeControls(RECT parentRectangle, std::string cameraMode)
{
	reorganizeControl(titleBox, cameraMode, parentRectangle);
	reorganizeControl(currentDataSetNumberText, cameraMode, parentRectangle);
	reorganizeControl(currentDataSetNumberEdit, cameraMode, parentRectangle);
	reorganizeControl(dataOutputNameText, cameraMode, parentRectangle);
	reorganizeControl(dataOutputNameCombo, cameraMode, parentRectangle);
	reorganizeControl(autoAnalyzeCheckBox, cameraMode, parentRectangle);
	reorganizeControl(autoAnalysisText, cameraMode, parentRectangle);
	reorganizeControl(autoAnalysisTypeCombo, cameraMode, parentRectangle);
	reorganizeControl(setAnalysisLocationsButton, cameraMode, parentRectangle);
	reorganizeControl(analyzeMostRecentButton, cameraMode, parentRectangle);
	reorganizeControl(dataOutputNameDetailCombo1, cameraMode, parentRectangle);
	reorganizeControl(dataOutputNameDetailCombo2, cameraMode, parentRectangle);
	return false;
}

bool DataAnalysisHandler::initializeControls(POINT& topLeftPositionKinetic, POINT& topLeftPositionAccumulate, POINT& topLeftPositionContinuous,
	HWND parentWindow, bool isTriggerModeSensitive)
{
	// Title for the whole control
	titleBox.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 480, topLeftPositionKinetic.y + 25 };
	titleBox.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 480, topLeftPositionAccumulate.y + 25 };
	titleBox.continuousSingleScansModePos = { -1,-1,-1,-1 };
	RECT initPos = titleBox.kineticSeriesModePos;
	titleBox.hwnd = CreateWindowEx(0, "STATIC", "Data Analysis", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
	titleBox.fontType = "Heading";
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;
	//
	currentDataSetNumberText.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 400, topLeftPositionKinetic.y + 25 };
	currentDataSetNumberText.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 400, topLeftPositionAccumulate.y + 25 };
	currentDataSetNumberText.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = currentDataSetNumberText.kineticSeriesModePos;
	currentDataSetNumberText.hwnd = CreateWindowEx(0, "STATIC", "Most Recent Data Set #:", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
	currentDataSetNumberText.fontType = "Normal";
	//
	currentDataSetNumberEdit.kineticSeriesModePos = { topLeftPositionKinetic.x + 400, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 480, topLeftPositionKinetic.y + 25 };
	currentDataSetNumberEdit.accumulateModePos = { topLeftPositionAccumulate.x + 400, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 480, topLeftPositionAccumulate.y + 25 };
	currentDataSetNumberEdit.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = currentDataSetNumberEdit.kineticSeriesModePos;
	currentDataSetNumberEdit.hwnd = CreateWindowEx(0, "EDIT", "?", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_MOST_RECENT_DATA_SET_NUMBER, eHInst, NULL);
	currentDataSetNumberEdit.fontType = "Normal";
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;

	autoAnalyzeCheckBox.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 480, topLeftPositionKinetic.y + 20 };
	autoAnalyzeCheckBox.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 480, topLeftPositionAccumulate.y + 20 };
	autoAnalyzeCheckBox.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = autoAnalyzeCheckBox.kineticSeriesModePos;
	autoAnalyzeCheckBox.hwnd = CreateWindowEx(0, "BUTTON", "Automatically Analyze Data at Finish?", WS_CHILD | WS_VISIBLE | ES_RIGHT | ES_READONLY | BS_CHECKBOX,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_AUTOANALYZE_CHECKBOX, eHInst, NULL);
	autoAnalyzeCheckBox.fontType = "Normal";
	topLeftPositionKinetic.y += 20;
	topLeftPositionAccumulate.y += 20;

	dataOutputNameText.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 100, topLeftPositionKinetic.y + 25 };
	dataOutputNameText.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 100, topLeftPositionAccumulate.y + 25 };
	dataOutputNameText.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = dataOutputNameText.kineticSeriesModePos;
	dataOutputNameText.hwnd = CreateWindowEx(0, "EDIT", "Filename:", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
	dataOutputNameText.fontType = "Normal";

	dataOutputNameCombo.kineticSeriesModePos = { topLeftPositionKinetic.x + 100, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 300, topLeftPositionKinetic.y + 800 };
	dataOutputNameCombo.accumulateModePos = { topLeftPositionAccumulate.x + 100, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 300, topLeftPositionAccumulate.y + 800 };
	dataOutputNameCombo.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = dataOutputNameCombo.kineticSeriesModePos;
	dataOutputNameCombo.hwnd = CreateWindowEx(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_DATA_OUTPUT_NAME_COMBO, eHInst, NULL);
	dataOutputNameCombo.fontType = "Small";
	SendMessage(dataOutputNameCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Carrier_Calibration");
	SendMessage(dataOutputNameCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Sideband_Spectrum");
	SendMessage(dataOutputNameCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Global_Microwave");
	SendMessage(dataOutputNameCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Microwave_Rabi");
	SendMessage(dataOutputNameCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Ramsey_Time_Scan");
	SendMessage(dataOutputNameCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Tunneling_Bias_Scan");
	SendMessage(dataOutputNameCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Tunneling_Time_Scan");
	SendMessage(dataOutputNameCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Piezo_Scan");
	// TODO: add more.
	dataOutputNameDetailCombo1.kineticSeriesModePos = { topLeftPositionKinetic.x + 300, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 400, topLeftPositionKinetic.y + 800 };
	dataOutputNameDetailCombo1.accumulateModePos = { topLeftPositionAccumulate.x + 300, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 400, topLeftPositionAccumulate.y + 800 };
	dataOutputNameDetailCombo1.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = dataOutputNameDetailCombo1.kineticSeriesModePos;
	dataOutputNameDetailCombo1.hwnd = CreateWindowEx(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_DATA_OUTPUT_NAME_DETAILS_COMBO, eHInst, NULL);
	dataOutputNameDetailCombo1.fontType = "Normal";
	SendMessage(dataOutputNameDetailCombo1.hwnd, CB_ADDSTRING, 0, (LPARAM)"");
	SendMessage(dataOutputNameDetailCombo1.hwnd, CB_ADDSTRING, 0, (LPARAM)"Top");
	SendMessage(dataOutputNameDetailCombo1.hwnd, CB_ADDSTRING, 0, (LPARAM)"Bottom");
	SendMessage(dataOutputNameDetailCombo1.hwnd, CB_ADDSTRING, 0, (LPARAM)"Axial");
	SendMessage(dataOutputNameDetailCombo1.hwnd, CB_ADDSTRING, 0, (LPARAM)"Radial");
	SendMessage(dataOutputNameDetailCombo1.hwnd, CB_ADDSTRING, 0, (LPARAM)"LS");
	SendMessage(dataOutputNameDetailCombo1.hwnd, CB_ADDSTRING, 0, (LPARAM)"EO");
	dataOutputNameDetailCombo2.kineticSeriesModePos = { topLeftPositionKinetic.x + 400, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 480, topLeftPositionKinetic.y + 800 };
	dataOutputNameDetailCombo2.accumulateModePos = { topLeftPositionAccumulate.x + 400, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 480, topLeftPositionAccumulate.y + 800 };
	dataOutputNameDetailCombo2.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = dataOutputNameDetailCombo2.kineticSeriesModePos;
	dataOutputNameDetailCombo2.hwnd = CreateWindowEx(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_DATA_OUTPUT_NAME_DETAILS_COMBO, eHInst, NULL);
	dataOutputNameDetailCombo2.fontType = "Normal";
	SendMessage(dataOutputNameDetailCombo2.hwnd, CB_ADDSTRING, 0, (LPARAM)"");
	SendMessage(dataOutputNameDetailCombo2.hwnd, CB_ADDSTRING, 0, (LPARAM)"Vert");
	SendMessage(dataOutputNameDetailCombo2.hwnd, CB_ADDSTRING, 0, (LPARAM)"Hor");
	SendMessage(dataOutputNameDetailCombo2.hwnd, CB_ADDSTRING, 0, (LPARAM)"X");
	SendMessage(dataOutputNameDetailCombo2.hwnd, CB_ADDSTRING, 0, (LPARAM)"Y");

	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;

	autoAnalysisText.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 200, topLeftPositionKinetic.y + 25 };
	autoAnalysisText.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 200, topLeftPositionAccumulate.y + 25 };
	autoAnalysisText.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = autoAnalysisText.kineticSeriesModePos;
	autoAnalysisText.hwnd = CreateWindowEx(0, "EDIT", "Autoanalysis Type:", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
	autoAnalysisText.fontType = "Normal";

	autoAnalysisTypeCombo.kineticSeriesModePos = { topLeftPositionKinetic.x + 200, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 480, topLeftPositionKinetic.y + 800 };
	autoAnalysisTypeCombo.accumulateModePos = { topLeftPositionAccumulate.x + 200, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 480, topLeftPositionAccumulate.y + 800};
	autoAnalysisTypeCombo.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = autoAnalysisTypeCombo.kineticSeriesModePos;
	autoAnalysisTypeCombo.hwnd = CreateWindowEx(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_DATA_AUTOANALYSIS_COMBO, eHInst, NULL);
	autoAnalysisTypeCombo.fontType = "Normal";
	SendMessage(autoAnalysisTypeCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Single Point Analysis");
	SendMessage(autoAnalysisTypeCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Pair Analysis");
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;
	// 
	setAnalysisLocationsButton.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 480, topLeftPositionKinetic.y + 25 };
	setAnalysisLocationsButton.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 480, topLeftPositionAccumulate.y + 25 };
	setAnalysisLocationsButton.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = setAnalysisLocationsButton.kineticSeriesModePos;
	setAnalysisLocationsButton.hwnd = CreateWindowEx(0, "BUTTON", "Set AutoAnalysis Points", WS_CHILD | WS_VISIBLE | BS_PUSHLIKE | BS_CHECKBOX,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_SET_ANALYSIS_LOCATION, eHInst, NULL);
	setAnalysisLocationsButton.fontType = "Normal";
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;

	//
	analyzeMostRecentButton.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 480, topLeftPositionKinetic.y + 25 };
	analyzeMostRecentButton.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 480, topLeftPositionAccumulate.y + 25 };
	analyzeMostRecentButton.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = analyzeMostRecentButton.kineticSeriesModePos;
	analyzeMostRecentButton.hwnd = CreateWindowEx(0, "BUTTON", "Analyze Most Recent Data", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_ANALYZE_MOST_RECENT, eHInst, NULL);
	analyzeMostRecentButton.fontType = "Normal";
	topLeftPositionKinetic.y += 25;
	topLeftPositionAccumulate.y += 25;
	return false;
}

bool DataAnalysisHandler::updateDataSetNumberEdit(int number)
{
	if (number > 0)
	{
		SendMessage(currentDataSetNumberEdit.hwnd, WM_SETTEXT, 0, (LPARAM)std::to_string(number).c_str());
	}
	else
	{
		SendMessage(currentDataSetNumberEdit.hwnd, WM_SETTEXT, 0, (LPARAM)"None");
	}
	return false;
}

// adds an output name to the output filename combo.
bool DataAnalysisHandler::addNameToCombo()
{
	std::string newOutputName = (const char*)DialogBoxParam(eHInst, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)dialogProcedures::textPromptDialogProcedure, (LPARAM)"Please enter a new name to call the auto-analyzed data files.\r\nThis name will be temporary (removed upon program restart), please ask Mark to make any permantent changes.");
	SendMessage(dataOutputNameCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)newOutputName.c_str());
	return false;
}

bool DataAnalysisHandler::analyze(std::string date, long runNumber, long accumulations, EmbeddedPythonHandler* pyHandler)
{
	std::string analysisFunctionName, analysisType;
	std::string outputName, details1, details2;
	// get analysis type
	int selectedNumber = SendMessage(autoAnalysisTypeCombo.hwnd, CB_GETCURSEL, 0, 0);
	TCHAR text[256];
	SendMessage(autoAnalysisTypeCombo.hwnd, CB_GETLBTEXT, selectedNumber, (LPARAM)text);
	analysisType = std::string(text);
	// interpret the text here to get the actual function name.
	if (analysisType == "Single Point Analysis")
	{
		analysisFunctionName = "singlePointAnalysis";
	}
	else if (analysisType == "Pair Analysis")
	{
		analysisFunctionName = "pairAnalysis";
	}
	else
	{
		MessageBox(0, "ERROR: unrecognized analysis type while trying to figure out the analysis function name! Ask Mark about bugs.", 0, 0);
		return true;
	}
	// get output name
	selectedNumber = SendMessage(dataOutputNameCombo.hwnd, CB_GETCURSEL, 0, 0);
	SendMessage(dataOutputNameCombo.hwnd, CB_GETLBTEXT, selectedNumber, (LPARAM)text);
	outputName = std::string(text);
	if (outputName == "")
	{
		MessageBox(0, "ERROR: Please select an output name if you are doing autoanalysis!", 0, 0);
		return true;
	}
	selectedNumber = SendMessage(dataOutputNameDetailCombo1.hwnd, CB_GETCURSEL, 0, 0);
	SendMessage(dataOutputNameDetailCombo1.hwnd, CB_GETLBTEXT, selectedNumber, (LPARAM)text);
	details1 = std::string(text);
	selectedNumber = SendMessage(dataOutputNameDetailCombo2.hwnd, CB_GETCURSEL, 0, 0);
	SendMessage(dataOutputNameDetailCombo2.hwnd, CB_GETLBTEXT, selectedNumber, (LPARAM)text);
	details2 = std::string(text);

	// python is initialized in the constructor for the data handler object. 
	appendText("Beginning Data Analysis...\r\n", IDC_STATUS_EDIT);
	// Get information to send to the python script from inputParam
	//std::string moduleName = "SingleAtomAnalysisFunction";
	std::string completeName = outputName;
	if (details1 != "")
	{
		completeName += "_" + details1;
	}
	if (details2 != "")
	{
		completeName += "_" + details2;
	}

	if (pyHandler->runDataAnalysis(analysisType, date, runNumber, accumulations, completeName, this->atomLocations))
	{
		appendText("Data Analysis Failed!\r\n", IDC_STATUS_EDIT);
		appendText("Data Analysis Failed!\r\n", IDC_ERROR_EDIT);
	}
	else
	{
		appendText("Finished Data Analysis!\r\n", IDC_STATUS_EDIT);
	}
	return false;
}

// handles the pressing of the analysis points button.
// TODO: handle different cases where single locations or pairs of locations are being analyzed. 
bool DataAnalysisHandler::onButtonPushed()
{
	BOOL checked = SendMessage(setAnalysisLocationsButton.hwnd, BM_GETCHECK, 0, 0);
	if (checked)
	{
		SendMessage(setAnalysisLocationsButton.hwnd, BM_SETCHECK, BST_UNCHECKED, 0);
		SendMessage(setAnalysisLocationsButton.hwnd, WM_SETTEXT, 0, (LPARAM)"Set Analysis Points");
		eSettingAnalysisLocations = false;
	}
	else
	{
		atomLocations.clear();
		SendMessage(setAnalysisLocationsButton.hwnd, BM_SETCHECK, BST_CHECKED, 0);
		SendMessage(setAnalysisLocationsButton.hwnd, WM_SETTEXT, 0, (LPARAM)"Right-Click Relevant Points and Reclick");
		eSettingAnalysisLocations = true;
	}			
	int experimentPictureNumber;
	if (eRealTimePictures)
	{
		experimentPictureNumber = 0;
	}
	else
	{
		experimentPictureNumber = (((eCurrentAccumulationNumber - 1) % ePicturesPerVariation) % ePicturesPerRepetition);
	}
	if ((experimentPictureNumber == ePicturesPerRepetition - 1 || eRealTimePictures) && eDataExists)
	{
		myAndor::drawDataWindow();
	}
	return false;
}

bool DataAnalysisHandler::setAtomLocation(std::pair<int, int> location)
{
	bool exists = false;
	for (int locInc = 0; locInc < this->atomLocations.size(); locInc++)
	{
		if (location == atomLocations[locInc])
		{
			exists = true;
		}
	}
	if (!exists)
	{
		atomLocations.push_back(location);
	}
	return false;
}
std::vector<std::pair<int, int>> DataAnalysisHandler::getAtomLocations()
{
	return atomLocations;
}
bool DataAnalysisHandler::clearAtomLocations()
{
	atomLocations.clear();
	return false;
}

bool DataAnalysisHandler::combosAreEmpty()
{
	if (SendMessage(autoAnalysisTypeCombo.hwnd, CB_GETCURSEL, 0, 0) == -1 || SendMessage(dataOutputNameCombo.hwnd, CB_GETCURSEL, 0, 0) == -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

std::string DataAnalysisHandler::getSelectedAnalysisType()
{
	int selection = SendMessage(this->autoAnalysisTypeCombo.hwnd, CB_GETCURSEL, 0, 0);
	if (selection == -1)
	{
		return "";
	}
	TCHAR text[256];
	SendMessage(this->autoAnalysisTypeCombo.hwnd, CB_GETLBTEXT, selection, (LPARAM)text);
	return text;
}