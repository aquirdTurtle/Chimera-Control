// created by Mark O. Brown
#pragma once

#include "CustomMfcControlWrappers/functionCombo.h"
#include "ParameterSystem/ParameterSystemStructures.h"
#include "Control.h"
#include "afxwin.h"
#include "afxcview.h"
#include "CustomMfcControlWrappers/MyListCtrl.h"
#include "ConfigurationSystems/Version.h"
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
		ParameterSystem ( std::string configurationFileDelimiter );
		BOOL handleAccelerators( HACCEL m_haccel, LPMSG lpMsg );
		void handleDraw(NMHDR* pNMHDR, LRESULT* pResult );
		void initialize( POINT& pos, cToolTips& toolTips, CWnd* master, int& id, std::string title, UINT listviewId, 
						 ParameterSysType type );
		void handleSingleClick (  );
		void handleDblClick ( std::vector<Script*> scripts, MainWindow* mainWin, AuxiliaryWindow* auxWin,
								   DioSystem* ttls, AoSystem* aoSys );
		void adjustVariableValue ( std::string paramName, double value );
		void deleteVariable ( );
		void addParameter( parameterType var );
		void reorderVariableDimensions ( );
		void removeVariableDimension();
		void clearParameters ( );
		void flattenScanDimensions ( );
		void checkScanDimensionConsistency ( );
		void checkVariationRangeConsistency ( );
		void saveParameter ( std::ofstream& saveFile, parameterType variable );
		static void generateKey ( std::vector<std::vector<parameterType>>& variables, bool randomizeVariablesOption,
								  ScanRangeInfo inputRangeInfo );
		static std::vector<parameterType> combineParamsForExpThread ( std::vector<parameterType>& masterVars,
																				 std::vector<parameterType>& subVars );

		void handleColumnClick ( NMHDR * pNotifyStruct, LRESULT* result );
		void redrawListview ( );
		// getters
		parameterType getVariableInfo(int varNumber);
		std::vector<parameterType> getAllConstants();
		std::vector<parameterType> getAllVariables();
		std::vector<parameterType> getAllParams();
		unsigned int getCurrentNumberOfVariables();
		UINT getTotalVariationNumber ( );
		double getVariableValue ( std::string paramName );
		static std::vector<double> getKeyValues ( std::vector<parameterType> variables );
		ScanRangeInfo getRangeInfo ( );
		// setters
		INT_PTR handleColorMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, brushMap brushes);
		void setVariationRangeNumber(int num, USHORT dimNumber);
		void rearrange(UINT width, UINT height, fontMap fonts);
		void setParameterControlActive(bool active);
		void setUsages(std::vector<std::vector<parameterType>> vars);
		void updateVariationNumber( );
		void setRangeInclusivity( UINT rangeNum, UINT dimNum, bool isLeft, bool inclusive);
		// file handling
		static parameterType loadParameterFromFile ( std::ifstream& openFile, Version ver, ScanRangeInfo inputRangeInfo );
		static std::vector<parameterType> getParametersFromFile ( std::ifstream& configFile, Version ver, ScanRangeInfo inputRangeInfo );
		static ScanRangeInfo getRangeInfoFromFile ( std::ifstream& configFile, Version ver );
		static std::vector<parameterType> getConfigParamsFromFile ( std::string configFile );
		static ScanRangeInfo getRangeInfoFromFile ( std::string configFileName );
		// profile stuff
		void handleNewConfig ( std::ofstream& newFile );
		void handleSaveConfig ( std::ofstream& saveFile );
		void handleOpenConfig ( std::ifstream& openFile, Version ver );		
		
		// public variables
		const IndvRangeInfo defaultRangeInfo = { 2,false,true };
		const std::string configDelim;

	private:
		void addParamToListview ( parameterType param, UINT item );
		void setVariationRangeColumns ( int num = -1, int width = -1 );
		bool controlActive = true;
		std::vector<CDialog*> childDlgs;
		int mostRecentlySelectedParam = -1;
		// name, constant/variable, dim, constantValue, scope
		USHORT preRangeColumns = 5;
		// Only 2 gui elements.
		Control<CStatic> parametersHeader;
		Control<MyListCtrl> parametersListview;
		// most important member, holds the settings for all current variables. 
		std::vector<parameterType> currentParameters;
		// number of variations that the variables will go through.
		UINT currentVariations;
		// A global parameter, the "official" version. 
		ScanRangeInfo rangeInfo;
		ParameterSysType paramSysType;
		// number of dimensions to the variable scans. Unusual to do more than 2.
		USHORT scanDimensions;
};

