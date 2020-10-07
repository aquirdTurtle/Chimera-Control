// created by Mark O. Brown
#pragma once

#include "GeneralObjects/RunInfo.h"
#include "ParameterSystem/ParameterSystemStructures.h"
#include "AnalogOutput/AoStructures.h"
#include <Scripts/SyntaxHighlighter.h>
#include <Scripts/ScriptableDevices.h>
#include "CustomQtControls/functionCombo.h"
#include <GeneralObjects/IChimeraSystem.h>
#include <string>
#include <unordered_map>
#include <string>
#include <vector>
#include <array>

#include <qlabel.h>
#include <CustomQtControls/AutoNotifyCtrls.h>

class IChimeraQtWindow; 

class Script : public IChimeraSystem {
	public:
		static constexpr auto MASTER_SCRIPT_EXTENSION = "mScript";
		static constexpr auto NIAWG_SCRIPT_EXTENSION = "nScript";
		static constexpr auto AGILENT_SCRIPT_EXTENSION = "aScript";
		static constexpr auto FUNCTION_EXTENSION = "func";

		Script(IChimeraQtWindow* parent);
		void initialize( int width, int height, QPoint& startingLocation, IChimeraQtWindow* scriptWin,
 						 std::string deviceTypeInput, std::string scriptHeader );
		bool isFunction ( );
		std::string getScriptText();
		void setScriptText( std::string text );
		void functionChangeHandler( std::string configPath );
		void changeView( std::string viewName, bool isFunction, std::string configPath);
		void saveScript( std::string location, RunInfo info );
		void saveScriptAs( std::string location, RunInfo info );
		void renameScript( std::string configPath);
		void deleteScript( std::string configPath);
		void newScript( );
		void newFunction();

		std::string getScriptPathAndName();
		std::string getScriptPath();
		std::string getScriptName();
		std::string getExtension();

		void loadFile( std::string pathToFile );
		void openParentScript( std::string parentScriptFileAndPath, std::string configPath, RunInfo info );
		void considerCurrentLocation( std::string configPath, RunInfo info );
		void checkSave( std::string configPath, RunInfo info );
		void updateSavedStatus( bool isSaved );
		void updateScriptNameText( std::string path );
		void reset();
		bool savedStatus();

		std::vector<parameterType> getLocalParams ();

		void saveAsFunction( );
		void loadFunctions();

		void setEnabled ( bool enabled, bool functionsEnabled );
		SyntaxHighlighter* highlighter;

	private:
		CQTextEdit* edit;
		QLabel* title;
		CQCheckBox*  savedIndicator;
		QLabel* fileNameText;
		functionCombo availableFunctionsCombo;
		QLabel* help;
		std::string scriptName;
		std::string scriptPath;
		std::string scriptFullAddress;
		std::string deviceType;
		std::string extension;
		bool isLocalReference;
		bool isSaved;

		unsigned long editChangeBegin;
		unsigned long editChangeEnd;
		bool syntaxColoringIsCurrent;
};