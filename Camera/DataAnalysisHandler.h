#pragma once
#include "Control.h"
#include "Windows.h"
#include <string>
// This class will eventually handle all of the automatic data analysis. Right now, pretty empty.
class DataAnalysisHandler
{
	public:
		bool reorganizeControls(RECT parentRectangle, std::string cameraMode);
		bool initializeControls(POINT& topLeftPositionKinetic, POINT& topLeftPositionAccumulate, POINT& topLeftPositionContinuous,
								HWND parentWindow, bool isTriggerModeSensitive);
		bool updateDataSetNumberEdit(int number);
	private:
		Control titleBox;
		Control currentDataSetNumberTitleBox;
		Control currentDataSetNumberEdit;
		Control dataOutputNameCombo;
		Control autoAnalyzeCheckBox;
		Control autoAnalysisTypeCombo;
};