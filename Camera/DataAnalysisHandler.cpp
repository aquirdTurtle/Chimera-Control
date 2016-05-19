#include "stdafx.h"
#include "DataAnalysisHandler.h";
#include "constants.h"
#include "reorganizeControl.h"

bool DataAnalysisHandler::reorganizeControls(RECT parentRectangle, std::string cameraMode)
{
	reorganizeControl(titleBox, cameraMode, parentRectangle);
	reorganizeControl(currentDataSetNumberTitleBox, cameraMode, parentRectangle);
	reorganizeControl(currentDataSetNumberEdit, cameraMode, parentRectangle);
	return false;
}

bool DataAnalysisHandler::initializeControls(POINT& topLeftPositionKinetic, POINT& topLeftPositionAccumulate, POINT& topLeftPositionContinuous,
	HWND parentWindow, bool isTriggerModeSensitive)
{
	// just initialize two of them right now. More to come.
	titleBox.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y, topLeftPositionKinetic.x + 272, topLeftPositionKinetic.y + 25 };
	titleBox.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y, topLeftPositionAccumulate.x + 272, topLeftPositionAccumulate.y + 25 };
	titleBox.continuousSingleScansModePos = { -1,-1,-1,-1 };
	RECT initPos = titleBox.kineticSeriesModePos;
	titleBox.hwnd = CreateWindowEx(0, "STATIC", "Data Analysis", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
	titleBox.fontType = "Heading";
	//
	currentDataSetNumberTitleBox.kineticSeriesModePos = { topLeftPositionKinetic.x, topLeftPositionKinetic.y + 25, topLeftPositionKinetic.x + 200, topLeftPositionKinetic.y + 50 };
	currentDataSetNumberTitleBox.accumulateModePos = { topLeftPositionAccumulate.x, topLeftPositionAccumulate.y + 25, topLeftPositionAccumulate.x + 200, topLeftPositionAccumulate.y + 50 };
	currentDataSetNumberTitleBox.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = currentDataSetNumberTitleBox.kineticSeriesModePos;
	currentDataSetNumberTitleBox.hwnd = CreateWindowEx(0, "STATIC", "Most Recent Data Set #:", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)-1, eHInst, NULL);
	currentDataSetNumberTitleBox.fontType = "Normal";
	//
	currentDataSetNumberEdit.kineticSeriesModePos = { topLeftPositionKinetic.x + 200, topLeftPositionKinetic.y + 25, topLeftPositionKinetic.x + 272, topLeftPositionKinetic.y + 50 };
	currentDataSetNumberEdit.accumulateModePos = { topLeftPositionAccumulate.x + 200, topLeftPositionAccumulate.y + 25, topLeftPositionAccumulate.x + 272, topLeftPositionAccumulate.y + 50 };
	currentDataSetNumberEdit.continuousSingleScansModePos = { -1,-1,-1,-1 };
	initPos = currentDataSetNumberEdit.kineticSeriesModePos;
	currentDataSetNumberEdit.hwnd = CreateWindowEx(0, "EDIT", "?", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
		initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top,
		parentWindow, (HMENU)IDC_MOST_RECENT_DATA_SET_NUMBER, eHInst, NULL);
	currentDataSetNumberEdit.fontType = "Normal";
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
