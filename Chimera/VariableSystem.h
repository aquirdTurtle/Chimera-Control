#pragma once
#include "functionCombo.h"
#include "VariableStructures.h"
#include "Control.h"
#include "afxwin.h"
#include "afxcview.h"
#include <vector>
#include <string>
#include <unordered_map>

class MainWindow;
class AuxiliaryWindow;
class Script;
class AuxiliaryWindow;
class AoSystem;
class DioSystem;

enum class VariableSysType
{
	global,
	config,
	function
};

/*
 * The VariableSystem class is a big class. Most of what it does is simply managing the different types of 
 * variable gui elements, i.e. managing all the different things the user can do to a variable. It is big mostly 
 * because there are simply a lot of different settings that need to be handled, and as well that the listview handling
 * in mfc is not particularly elegant so a lot of the handling ends up fairly verbose.
 *
 * This is not a singleton class, as it is re-used with slight variations for the different types of gui elements:
 * - Global Variables
 * - Config Variables
 * - Function Variables
 * That being said, there should only ever be 3 instances of this class active, one for each gui element.
 */
class VariableSystem
{
	public:
		UINT getTotalVariationNumber();
		void handleNewConfig( std::ofstream& newFile );
		void handleSaveConfig(std::ofstream& saveFile);
		void normHandleOpenConfig(std::ifstream& openFile, int versionMajor, int versionMinor );
		void handleDraw(NMHDR* pNMHDR, LRESULT* pResult, rgbMap rgbs);
		void updateVariableInfo( std::vector<Script*> scripts, MainWindow* mainWin, AuxiliaryWindow* auxWin,
								 DioSystem* ttls, AoSystem* aoSys );
		void deleteVariable();
		void initialize( POINT& pos, cToolTips& toolTips, AuxiliaryWindow* master, int& id, std::string title,
						 rgbMap rgbs, UINT listviewId, VariableSysType type );
		void addConfigVariable( variableType var, UINT item );
		void addGlobalVariable( variableType var, UINT item );
		void handleColumnClick( NMHDR * pNotifyStruct, LRESULT* result );
		void removeVariableDimension();
		variableType getVariableInfo(int varNumber);
		std::vector<variableType> getAllConstants();
		std::vector<variableType> getAllVariables();
		std::vector<variableType> getEverything();
		unsigned int getCurrentNumberOfVariables();
		void clearVariables();
		INT_PTR handleColorMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, brushMap brushes);
		void setVariationRangeNumber(int num, USHORT dimNumber);
		void rearrange(UINT width, UINT height, fontMap fonts);
		void setActive(bool active);
		void setUsages(std::vector<std::vector<variableType>> vars);
		void updateVariationNumber( );
		void setRangeInclusivity( UINT rangeNum, bool leftBorder, bool inclusive, UINT column );
		/// used to be in KeyHandler
		static void generateKey( std::vector<std::vector<variableType>>& variables, bool randomizeVariablesOption );
		static std::vector<double> getKeyValues( std::vector<variableType> variables );
		void reorderVariableDimensions( );
		static std::vector<variableType> getConfigVariablesFromFile( std::string configFile );
		void handleFuncCombo( );
		void loadVariablesFromFunc( std::string funcName );
		void saveVariable( std::ofstream& saveFile, variableType variable );
		variableType loadVariableFromFile( std::ifstream& openFile, UINT versionMajor, UINT versionMinor );
		void funcHandleOpenConfig( std::ifstream& configFile, int versionMajor, int versionMinor );
		std::vector<variableType> getVariablesFromFile( std::ifstream& configFile, int versionMajor, int versionMinor );
	private:
		// name, constant/variable, dim, constantValue.
		USHORT preRangeColumns = 4;
		// Only 2 gui elements.
		Control<CStatic> variablesHeader;
		Control<CListCtrl> variablesListview;
		functionCombo funcCombo;
		// most important member, holds the settings for all current variables. Might change to have an outer vector 
		// for each scan dimension, like so?
		std::vector<variableType> currentVariables;
		// number of variations that the variables will go through.
		UINT currentVariations;
		// holds the number of variable ranges. Not sure why this is necessary, could probablty get this info from 
		// currentVariables member.
		USHORT variableRanges;
		VariableSysType varSysType;
		// number of dimensions to the variable scans. Unusual to do more than 2.
		USHORT scanDimensions;
		// only for the function control. Keeps a (potentially long) list of all variables from functions that have 
		// previously had values set for variations, used to smart recall these names even within functions.  The 
		// index off the map is the function name.
		std::unordered_map<std::string, std::vector<variableType>> funcVariables;
		std::string currentFuncName;
};

