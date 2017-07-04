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
	// whether this variable is constant or varies.
	bool constant;
	std::vector<variationRangeInfo> ranges;
};

class Script;
class MasterWindow;

class VariableSystem
{
	public:
		void updateVariableInfo(std::vector<Script*> scripts, MasterWindow* Master);
		void deleteVariable();
		void initialize(POINT& pos, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master, int& id, std::string title);
		void addConfigVariable(variable var, int item);
		void addGlobalVariable( variable var, int item );
		void handleColumnClick(NMHDR * pNotifyStruct, LRESULT * result);
		variable getVariableInfo(int varNumber);
		//std::vector<variable> getAllVariables();
		std::vector<variable> getAllConstants();
		std::vector<variable> getAllVariables();
		std::vector<variable> getEverything();
		unsigned int getCurrentNumberOfVariables();
		void clearVariables();
		INT_PTR handleColorMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, std::unordered_map<std::string, HBRUSH> brushes);
		void setVariationRangeNumber(int num);
		void rearrange(UINT width, UINT height, fontMap fonts);
	private:
		int totalVariableNumber;
		unsigned int currentVariations;
		Control<CStatic> variablesHeader;
		Control<CListCtrl> variablesListview;
		std::vector<variable> currentVariables;
		HINSTANCE programInstance;
		int variableRangeSets;
		std::vector<double> key;
		bool isGlobal;
};

