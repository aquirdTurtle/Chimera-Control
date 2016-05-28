#pragma once
#include "Control.h"
#include "Windows.h"
#include <string>
#include <vector>
// This class will eventually handle all of the automatic data analysis. Right now, pretty empty.
class DataAnalysisHandler
{
	public:
		DataAnalysisHandler::DataAnalysisHandler();
		DataAnalysisHandler::~DataAnalysisHandler();
		bool reorganizeControls(RECT parentRectangle, std::string cameraMode);
		bool initializeControls(POINT& topLeftPositionKinetic, POINT& topLeftPositionAccumulate, POINT& topLeftPositionContinuous,
								HWND parentWindow, bool isTriggerModeSensitive);
		bool updateDataSetNumberEdit(int number);
		bool addNameToCombo();
		bool analyze(std::string date, long runNumber, long accumulations);
		bool onButtonPushed();
		bool setAtomLocation(std::pair<int, int> location);
		std::vector<std::pair<int, int>> getAtomLocations();
		bool clearAtomLocations();
		bool combosAreEmpty();
	private:
		Control titleBox;
		Control currentDataSetNumberText;
		Control currentDataSetNumberEdit;
		Control dataOutputNameText;
		Control dataOutputNameCombo;
		Control autoAnalyzeCheckBox;
		Control autoAnalysisText;
		Control autoAnalysisTypeCombo;
		Control setAnalysisLocationsButton;
		Control analyzeMostRecentButton;
		std::vector<std::pair<int, int>> atomLocations;
};
