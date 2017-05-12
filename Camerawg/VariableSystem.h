#pragma once
#include <vector>
#include <string>
#include "Control.h"
#include "Windows.h"

class MainWindow;
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
		bool updateVariableInfo(MainWindow* comm, ScriptingWindow* scriptWin);
		bool deleteVariable();
		bool initializeControls(POINT &topLeftCorner, CWnd* parent, int& id, 
			std::unordered_map<std::string, CFont*> fonts, std::vector<CToolTipCtrl*>& tooltips);
		bool addVariable(std::string name, bool timelike, bool singleton, double value, int item);
		variable getVariableInfo(int varNumber);
		//std::vector<variable> getAllVariables();
		std::vector<variable> getAllSingletons();
		std::vector<variable> getAllVaryingParameters();
		std::vector<variable> getAllVariables();
		unsigned long long getCurrentNumberOfVariables();
		bool clearVariables();
		void rearrange(int width, int height, std::unordered_map<std::string, CFont*> fonts);
	private:
		int totalVariableNumber;
		Control<CStatic> header;
		Control<CListCtrl> listview;
		std::vector<variable> currentVariables;
};

