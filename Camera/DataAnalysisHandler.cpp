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
	reorganizeControl(autoAnalyzeCheckBox, cameraMode, parentRectangle);
	reorganizeControl(setAnalysisLocationsButton, cameraMode, parentRectangle);
	reorganizeControl(analyzeMostRecentButton, cameraMode, parentRectangle);
	return false;
}

bool DataAnalysisHandler::initializeControls(POINT& topLeftPosKinetic, POINT& topLeftPosAccumulate, 
											 POINT& topLeftPosContinuous, HWND parentWindow, 
											 bool isTriggerModeSensitive)
{
	// Title for the whole control
	titleBox.kineticSeriesModePos = { topLeftPosKinetic.x, topLeftPosKinetic.y, topLeftPosKinetic.x + 480, 
									  topLeftPosKinetic.y + 25 };
	titleBox.accumulateModePos = { topLeftPosAccumulate.x, topLeftPosAccumulate.y, topLeftPosAccumulate.x + 480, 
								   topLeftPosAccumulate.y + 25 };
	titleBox.continuousSingleScansModePos = { -1,-1,-1,-1 };
	RECT initPos = titleBox.kineticSeriesModePos;
	titleBox.hwnd = CreateWindowEx(0, "STATIC", "Data Analysis", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
	titleBox.fontType = "Heading";
	topLeftPosKinetic.y += 25;
	topLeftPosAccumulate.y += 25;
	//
	currentDataSetNumberText.kineticSeriesModePos = { topLeftPosKinetic.x, topLeftPosKinetic.y, 
													  topLeftPosKinetic.x + 400, topLeftPosKinetic.y + 25 };
	currentDataSetNumberText.accumulateModePos = { topLeftPosAccumulate.x, topLeftPosAccumulate.y, 
												   topLeftPosAccumulate.x + 400, topLeftPosAccumulate.y + 25 };
	currentDataSetNumberText.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = currentDataSetNumberText.kineticSeriesModePos;
	currentDataSetNumberText.hwnd = CreateWindowEx(0, "STATIC", "Most Recent Data Set #:", 
		WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
	currentDataSetNumberText.fontType = "Normal";
	//
	currentDataSetNumberEdit.kineticSeriesModePos = { topLeftPosKinetic.x + 400, topLeftPosKinetic.y, 
													  topLeftPosKinetic.x + 480, topLeftPosKinetic.y + 25 };
	currentDataSetNumberEdit.accumulateModePos = { topLeftPosAccumulate.x + 400, topLeftPosAccumulate.y, 
												   topLeftPosAccumulate.x + 480, topLeftPosAccumulate.y + 25 };
	currentDataSetNumberEdit.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = currentDataSetNumberEdit.kineticSeriesModePos;
	currentDataSetNumberEdit.hwnd = CreateWindowEx(0, "EDIT", "?", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_MOST_RECENT_DATA_SET_NUMBER, eHInst, NULL);
	currentDataSetNumberEdit.fontType = "Normal";
	topLeftPosKinetic.y += 25;
	topLeftPosAccumulate.y += 25;

	autoAnalyzeCheckBox.kineticSeriesModePos = { topLeftPosKinetic.x, topLeftPosKinetic.y, topLeftPosKinetic.x + 480, 
												 topLeftPosKinetic.y + 20 };
	autoAnalyzeCheckBox.accumulateModePos = { topLeftPosAccumulate.x, topLeftPosAccumulate.y, 
											  topLeftPosAccumulate.x + 480, topLeftPosAccumulate.y + 20 };
	autoAnalyzeCheckBox.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = autoAnalyzeCheckBox.kineticSeriesModePos;
	autoAnalyzeCheckBox.hwnd = CreateWindowEx(0, "BUTTON", "Automatically Analyze Data at Finish?", 
		WS_CHILD | WS_VISIBLE | ES_RIGHT | ES_READONLY | BS_CHECKBOX,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_AUTOANALYZE_CHECKBOX, eHInst, NULL);
	autoAnalyzeCheckBox.fontType = "Normal";
	topLeftPosKinetic.y += 20;
	topLeftPosAccumulate.y += 20;

	// 
	setAnalysisLocationsButton.kineticSeriesModePos = { topLeftPosKinetic.x, topLeftPosKinetic.y, 
														topLeftPosKinetic.x + 480, topLeftPosKinetic.y + 25 };
	setAnalysisLocationsButton.accumulateModePos = { topLeftPosAccumulate.x, topLeftPosAccumulate.y, 
													 topLeftPosAccumulate.x + 480, topLeftPosAccumulate.y + 25 };
	setAnalysisLocationsButton.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = setAnalysisLocationsButton.kineticSeriesModePos;
	setAnalysisLocationsButton.hwnd = CreateWindowEx(0, "BUTTON", "Set AutoAnalysis Points",
		WS_CHILD | WS_VISIBLE | BS_PUSHLIKE | BS_CHECKBOX,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_SET_ANALYSIS_LOCATION, eHInst, NULL);
	setAnalysisLocationsButton.fontType = "Normal";
	topLeftPosKinetic.y += 25;
	topLeftPosAccumulate.y += 25;

	//
	analyzeMostRecentButton.kineticSeriesModePos = { topLeftPosKinetic.x, topLeftPosKinetic.y, 
													 topLeftPosKinetic.x + 480, topLeftPosKinetic.y + 25 };
	analyzeMostRecentButton.accumulateModePos = { topLeftPosAccumulate.x, topLeftPosAccumulate.y, 
												  topLeftPosAccumulate.x + 480, topLeftPosAccumulate.y + 25 };
	analyzeMostRecentButton.continuousSingleScansModePos = { -1, -1, -1, -1 };
	initPos = analyzeMostRecentButton.kineticSeriesModePos;
	analyzeMostRecentButton.hwnd = CreateWindowEx(0, "BUTTON", "Analyze Most Recent Data", 
		WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_ANALYZE_MOST_RECENT, eHInst, NULL);
	analyzeMostRecentButton.fontType = "Normal";
	topLeftPosKinetic.y += 25;
	topLeftPosAccumulate.y += 25;
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

bool DataAnalysisHandler::analyze(std::string date, long runNumber, long accumulations, EmbeddedPythonHandler* pyHandler)
{
	std::string analysisFunctionName;
	std::string details1, details2;
	// python is initialized in the constructor for the data handler object. 
	appendText("Beginning Data Analysis...\r\n", IDC_STATUS_EDIT);
	// Get information to send to the python script from inputParam
	//std::string moduleName = "SingleAtomAnalysisFunction";

	if (pyHandler->runDataAnalysis(date, runNumber, accumulations, this->atomLocations))
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
