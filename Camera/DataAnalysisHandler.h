#pragma once
#include "Control.h"
#include "Windows.h"
#include <string>
#include <vector>
#include "Python.h"
#include "EmbeddedPythonHandler.h"
// This class will eventually handle all of the automatic data analysis. Right now, pretty empty.
class DataAnalysisHandler
{
	public:
		bool reorganizeControls(RECT parentRectangle, std::string cameraMode);
		bool initializeControls(POINT& topLeftPositionKinetic, POINT& topLeftPositionAccumulate, POINT& topLeftPositionContinuous,
								HWND parentWindow, bool isTriggerModeSensitive);
		bool updateDataSetNumberEdit(int number);
		bool addNameToCombo();
		bool analyze(std::string date, long runNumber, long accumulations, EmbeddedPythonHandler* pyHandler);
		bool onButtonPushed();
		bool setAtomLocation(std::pair<int, int> location);
		std::vector<std::pair<int, int>> getAtomLocations();
		bool clearAtomLocations();
		bool combosAreEmpty();
		std::string getSelectedAnalysisType();
	private:
		HwndControl titleBox;
		HwndControl currentDataSetNumberText;
		HwndControl currentDataSetNumberEdit;
		HwndControl dataOutputNameText;
		HwndControl dataOutputNameCombo;
		HwndControl dataOutputNameDetailCombo1;
		HwndControl dataOutputNameDetailCombo2;
		HwndControl autoAnalyzeCheckBox;
		HwndControl autoAnalysisText;
		HwndControl autoAnalysisTypeCombo;
		HwndControl setAnalysisLocationsButton;
		HwndControl analyzeMostRecentButton;
		std::vector<std::pair<int, int>> atomLocations;
};
