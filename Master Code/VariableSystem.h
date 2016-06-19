#pragma once
#include <vector>
#include <string>
#include "Control.h"
#include "Windows.h"

struct variable
{
	std::string name;
	bool timelike;
	bool singleton;
	double initialValue;
	double finalValue;
	int variations;
};

class Script;
class MasterWindow;
class VariableSystem
{
	public:
		bool updateVariableInfo(LPARAM lParamOfMessage, std::vector<Script*> scripts, MasterWindow* Master);
		bool deleteVariable(LPARAM lParamOfMessage);
		bool initialize(POINT topLeftCorner, HWND parentWindow);
		bool addVariable(std::string name, bool timelike, bool singleton, double value, int item);
		variable getVariableInfo(int varNumber);
		//std::vector<variable> getAllVariables();
		std::vector<variable> getAllSingletons();
		std::vector<variable> getAllVaryingParameters();
		unsigned int getCurrentNumberOfVariables();
		bool clearVariables();

	private:
		int totalVariableNumber;
		Control variablesHeader;
		Control variablesListview;
		std::vector<variable> currentVariables;
		HINSTANCE programInstance;
};

