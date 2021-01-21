// created by Mark O. Brown
#pragma once

#include "CustomQtControls/functionCombo.h"
#include "ParameterSystem/ParameterSystemStructures.h"
#include "Control.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "ParameterModel.h"
#include <vector>
#include <string>
#include <unordered_map>
#include "PrimaryWindows/IChimeraQtWindow.h"
#include <GeneralObjects/IChimeraSystem.h>
#include <qtablewidget.h>
#include <qlabel.h>

class MainWindow;
class AuxiliaryWindow;
class Script;
class AuxiliaryWindow;
class AoSystem;
class DoSystem;

enum class ParameterSysType{
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
class IChimeraSystem;

class ParameterSystem : IChimeraSystem {
	Q_OBJECT;
	public:		
		// THIS CLASS IS NOT COPYABLE.
		ParameterSystem& operator=(const ParameterSystem&) = delete;
		ParameterSystem (const ParameterSystem&) = delete;
		ParameterSystem (IChimeraQtWindow* parent, std::string configurationFileDelimiter );
		void handleContextMenu (const QPoint& pos);
		void initialize( QPoint& pos, IChimeraQtWindow* master, std::string title, ParameterSysType type, 
						 unsigned width=480, unsigned height=200 );
		void adjustVariableValue ( std::string paramName, double value );
		void addParameter( parameterType var );
		void clearParameters ( );
		void flattenScanDimensions ( );
		void saveParameter (ConfigStream& saveFile, parameterType variable );
		static void generateKey ( std::vector<parameterType>& variables, bool randomizeVariablesOption,
								  ScanRangeInfo inputRangeInfo );
		static std::vector<parameterType> combineParams ( std::vector<parameterType>& masterVars, 
														  std::vector<parameterType>& subVars );
		static std::vector<std::reference_wrapper<parameterType>> getConstParamsFromList (std::vector<parameterType>& variables);
		// getters
		parameterType getVariableInfo(int varNumber);
		std::vector<parameterType> getAllConstants();
		std::vector<parameterType> getAllVariables();
		std::vector<parameterType> getAllParams();
		unsigned int getCurrentNumberOfVariables();
		unsigned getTotalVariationNumber ( );
		double getVariableValue ( std::string paramName );
		static std::vector<double> getKeyValues ( std::vector<parameterType> variables );
		ScanRangeInfo getRangeInfo ( );
		// setters
		void setParameterControlActive(bool active);
		void setUsages(std::vector<parameterType> vars);
		void updateVariationNumber( );
		void setRangeInclusivity( unsigned rangeNum, unsigned dimNum, bool isLeft, bool inclusive);
		// file handling
		static parameterType loadParameterFromFile (ConfigStream& openFile, ScanRangeInfo inputRangeInfo );
		static std::vector<parameterType> getParametersFromFile ( ConfigStream& configFile, ScanRangeInfo inputRangeInfo );
		static ScanRangeInfo getRangeInfoFromFile (ConfigStream& configFile);
		static std::vector<parameterType> getConfigParamsFromFile ( std::string configFile );
		static ScanRangeInfo getRangeInfoFromFile ( std::string configFileName );
		// profile stuff
		void handleSaveConfig (ConfigStream& saveFile );
		void handleOpenConfig (ConfigStream& openFile );
		// public variables
		const std::string configDelim;
		void setTableviewColumnSize ();

private:
		QStringList baseLabels;
		bool controlActive = true;
		//std::vector<CDialog*> childDlgs;

		// name, constant/variable, dim, constantValue, scope
		QLabel* parametersHeader;
		//QTableWidget* parametersListview;
		QTableView* parametersView;
		// number of variations that the variables will go through.
		unsigned currentVariations;

		ParameterSysType paramSysType;
		ParameterModel paramModel;

	Q_SIGNALS:
		void paramsChanged ();
};

