#pragma once
#include "functionCombo.h"
#include "VariableStructures.h"
#include "Control.h"
#include "afxwin.h"
#include "afxcview.h"
#include "Version.h"
#include <vector>
#include <string>
#include <unordered_map>

class MainWindow;
class AuxiliaryWindow;
class Script;
class AuxiliaryWindow;
class AoSystem;
class DioSystem;

enum class ParameterSysType
{
	global,
	config
};

/*
 * The ParameterSystem class is a big class. Most of what it does is simply managing the different types of 
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
class ParameterSystem
{
	public:		
		BOOL handleAccelerators( HACCEL m_haccel, LPMSG lpMsg );
		UINT getTotalVariationNumber();
		void handleNewConfig( std::ofstream& newFile );
		void handleSaveConfig(std::ofstream& saveFile);
		void normHandleOpenConfig(std::ifstream& openFile, Version ver );
		void handleDraw(NMHDR* pNMHDR, LRESULT* pResult, rgbMap rgbs);
		void updateParameterInfo( std::vector<Script*> scripts, MainWindow* mainWin, AuxiliaryWindow* auxWin,
								 DioSystem* ttls, AoSystem* aoSys );
		void adjustVariableValue( std::string paramName, double value );
		void deleteVariable();
		void initialize( POINT& pos, cToolTips& toolTips, CWnd* master, int& id, std::string title, rgbMap rgbs, 
						 UINT listviewId, ParameterSysType type );
		void addConfigParameter( parameterType var, UINT item );
		void addGlobalParameter( parameterType var, UINT item );
		void handleColumnClick( NMHDR * pNotifyStruct, LRESULT* result );
		void removeVariableDimension();
		parameterType getVariableInfo(int varNumber);
		std::vector<parameterType> getAllConstants();
		std::vector<parameterType> getAllVariables();
		std::vector<parameterType> getEverything();
		unsigned int getCurrentNumberOfVariables();
		void clearVariables();
		INT_PTR handleColorMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, brushMap brushes);
		void setVariationRangeNumber(int num, USHORT dimNumber);
		void rearrange(UINT width, UINT height, fontMap fonts);
		void setParameterControlActive(bool active);
		void setUsages(std::vector<std::vector<parameterType>> vars);
		void updateVariationNumber( );
		void setRangeInclusivity( UINT rangeNum, bool leftBorder, bool inclusive, UINT column );
		// used to be in KeyHandler
		static void generateKey( std::vector<std::vector<parameterType>>& variables, bool randomizeVariablesOption );
		static std::vector<double> getKeyValues( std::vector<parameterType> variables );
		void reorderVariableDimensions( );
		static std::vector<parameterType> getConfigVariablesFromFile( std::string configFile );
		void saveVariable( std::ofstream& saveFile, parameterType variable );
		static parameterType loadVariableFromFile( std::ifstream& openFile, Version ver );
		static std::vector<parameterType> getVariablesFromFile( std::ifstream& configFile, Version ver );
		void updateCurrentVariationsNum( );
		static std::vector<parameterType> combineParametersForExperimentThread( std::vector<parameterType>& masterVars,
														   std::vector<parameterType>& subVars );
	private:
		bool controlActive = true;
		std::vector<CDialog*> childDlgs;
		// name, constant/variable, dim, constantValue, scope
		USHORT preRangeColumns = 5;
		// Only 2 gui elements.
		Control<CStatic> parametersHeader;
		Control<CListCtrl> parametersListview;
		// most important member, holds the settings for all current variables. Might change to have an outer vector 
		// for each scan dimension, like so?
		std::vector<parameterType> currentParameters;
		// number of variations that the variables will go through.
		UINT currentVariations;
		// holds the number of variable ranges. Not sure why this is necessary, could probablty get this info from 
		// currentParameters member.
		USHORT variableRanges;
		ParameterSysType paramSysType;
		// number of dimensions to the variable scans. Unusual to do more than 2.
		USHORT scanDimensions;
};

