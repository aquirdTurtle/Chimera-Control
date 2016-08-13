#pragma once
#include <vector>
#include <string>
#include "Control.h"
#include "Windows.h"
#include <unordered_map>
#include "afxcview.h"
#include "constants.h"

struct variationRangeInfo
{
	double initialValue;
	double finalValue;
	unsigned int variations;
};

struct variable
{
	std::string name;
	bool timelike;
	bool singleton;
	std::vector<variationRangeInfo> ranges;
};

class Script;
class MasterWindow;
class VariableSystem
{
	public:
		VariableSystem(int& startID)
		{
			this->variablesHeader.ID = startID;
			startID++;
			if (startID != LISTVIEW_CONTROL)
			{
				MessageBox(0, "ERROR: LISTVIEW_CONTROL is not equal to the actual ID.", 0, 0);
			}
			this->variablesListview.ID = startID;
			startID++;

		}
		bool updateVariableInfo(std::vector<Script*> scripts, MasterWindow* Master);
		bool deleteVariable();
		bool initialize(POINT& topLeftCorner, HWND parentWindow, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master);
		bool addVariable(variable var, int item);
		bool handleClick(NMHDR * pNotifyStruct, LRESULT * result);
		variable getVariableInfo(int varNumber);
		//std::vector<variable> getAllVariables();
		std::vector<variable> getAllSingletons();
		std::vector<variable> getAllVaryingParameters();
		std::vector<variable> getEverything();
		unsigned int getCurrentNumberOfVariables();
		bool clearVariables();
		INT_PTR handleColorMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, std::unordered_map<std::string, HBRUSH> brushes);
		bool setVariationRangeNumber(int num);

	private:
		int totalVariableNumber;
		unsigned int currentVariations;
		ClassControl<CStatic> variablesHeader;
		ClassControl<CListCtrl> variablesListview;
		std::vector<variable> currentVariables;
		HINSTANCE programInstance;
		int variableRangeSets;
		std::vector<double> key;
};

