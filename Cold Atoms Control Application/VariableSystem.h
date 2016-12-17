#pragma once
#include <vector>
#include <string>
#include "Control.h"
#include "Windows.h"

class ScriptingWindow;

struct variable
{
	std::string name;
	bool timelike;
	bool singleton;
	double value;
	double initialValue;
	double finalValue;
	int variations;
};

class VariableSystem
{
	public:
		bool updateVariableInfo(LPARAM lParamOfMessage, ScriptingWindow* scriptWin);
		bool deleteVariable(LPARAM lParamOfMessage);
		bool initializeControls(POINT topLeftCorner, CWnd* parent, int& id);
		bool addVariable(std::string name, bool timelike, bool singleton, double value, int item);
		variable getVariableInfo(int varNumber);
		//std::vector<variable> getAllVariables();
		std::vector<variable> getAllSingletons();
		std::vector<variable> getAllVaryingParameters();
		std::vector<variable> getAllVariables();
		unsigned int getCurrentNumberOfVariables();
		bool clearVariables();

	private:
		int totalVariableNumber;
		Control<CStatic> header;
		Control<CListCtrl> listview;
		//HwndControl variablesHeader;
		//HwndControl variablesListview;
		std::vector<variable> currentVariables;
};

