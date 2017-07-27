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
	bool leftInclusive;
	bool rightInclusive;
};


struct variable
{
	std::string name;
	bool timelike;
	// whether this variable is constant or varies.
	bool constant;	
	bool active = false;
	bool overwritten = false;
	// records which scan dimension the variable is in.
	USHORT scanDimension;
	std::vector<variationRangeInfo> ranges;
};


class Script;
class MasterWindow;


class VariableSystem
{
	public:
		void handleDraw(NMHDR* pNMHDR, LRESULT* pResult);
		void updateVariableInfo(std::vector<Script*> scripts, MasterWindow* Master);
		void deleteVariable();
		void initialize(POINT& pos, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master, int& id, std::string title);
		void addConfigVariable(variable var, int item);
		void addGlobalVariable( variable var, int item );
		void handleColumnClick(NMHDR * pNotifyStruct, LRESULT * result);
		void addVariableDimension();
		void removeVariableDimension();
		variable getVariableInfo(int varNumber);
		std::vector<variable> getAllConstants();
		std::vector<variable> getAllVariables();
		std::vector<variable> getEverything();
		unsigned int getCurrentNumberOfVariables();
		void clearVariables();
		INT_PTR handleColorMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, std::unordered_map<std::string, HBRUSH> brushes);
		void setVariationRangeNumber(int num, USHORT dimNumber);
		void rearrange(UINT width, UINT height, fontMap fonts);
		void setActive(bool active);
		void setUsages(std::vector<variable> vars);

	private:
		// Only 2 gui elements.
		Control<CStatic> variablesHeader;
		Control<CListCtrl> variablesListview;
		// most important member, holds the settings for all current variables. Might change to have an outer vector 
		// for each scan dimension, like so?
		//std::vector<std::vector<variable>> currentVariables;
		std::vector<variable> currentVariables;
		// Number of variables in the control.
		UINT totalVariableNumber;
		// number of variations that the variables will go through.
		UINT currentVariations;
		// holds the number of variable ranges. Not sure why this is necessary, could probablty get this info from 
		// currentVariables member.
		std::vector<USHORT> variableRanges;
		// tells the class whether it was initialized as the global variable control or the configuration control.
		bool isGlobal;
		// number of dimensions to the variable scans. Unusual to do more than 2.
		USHORT scanDimensions;
};

