// created by Mark O. Brown
#pragma once

#include "Control.h"
#include "CustomMfcControlWrappers/myButton.h"
#include "GeneralObjects/RunInfo.h"
#include "ParameterSystem/ParameterSystemStructures.h"
#include "AnalogOutput/AoStructures.h"
//#include "Combo.h"
#include "CustomMfcControlWrappers/functionCombo.h"
#include "ExperimentThread/Communicator.h"
#include "stdafx.h"
#include "Scripts/Script.h"

#include <string>
#include <unordered_map>
#include <string>
#include <vector>
#include <array>

class Script
{
	public:
		Script();
		void initialize( int width, int height, POINT& startingLocation, cToolTips& toolTips, CWnd* scriptWin, int& id,
						 std::string deviceTypeInput, std::string scriptHeader, std::array<UINT, 2> ids, 
						 COLORREF backgroundColor );
		bool isFunction ( );
		void handleToolTip( NMHDR * pNMHDR, LRESULT * pResult );
		std::string getScriptText();
		void setScriptText( std::string text );
		void functionChangeHandler( std::string configPath);
		void rearrange( UINT width, UINT height, fontMap fonts );
		void colorEntireScript( std::vector<parameterType> vars, Matrix<std::string> ttlNames, 
								std::array<AoInfo, 24> dacNames);
		void colorScriptSection( DWORD beginingOfChange, DWORD endOfChange, std::vector<parameterType> vars, 
								 Matrix<std::string> ttlNames,
								 std::array<AoInfo, 24> dacNames);
		COLORREF getSyntaxColor( std::string word, std::string editType, std::vector<parameterType> variables,
								 bool& colorLine, Matrix<std::string> ttlNames, std::array<AoInfo, 24> dacInfo );
		void handleEditChange();
		void handleTimerCall( std::vector<parameterType> vars, Matrix<std::string> ttlNames, 
							  std::array<AoInfo, 24> dacNames);
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
		void checkSave( std::string configPath, RunInfo info, Communicator* comm=NULL );
		void updateSavedStatus( bool isSaved );
		bool coloringIsNeeded();
		void updateScriptNameText( std::string path );
		HBRUSH handleColorMessage ( CWnd* window, CDC* cDC );
		void reset();
		bool savedStatus();

		INT_PTR handleColorMessage( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, brushMap brushes );

		void saveAsFunction( Communicator* comm );
		void loadFunctions();

		void setEnabled (bool enabled, bool functionsEnabled );

	private:
		CWnd syntaxTimer;
		Control<CRichEditCtrl> edit;
		Control<CStatic> title;
		Control<CleanCheck>  savedIndicator;
		Control<CEdit> fileNameText;
		functionCombo availableFunctionsCombo;
		Control<CEdit> help;

		std::string scriptName;
		std::string scriptPath;
		std::string scriptFullAddress;
		std::string deviceType;
		std::string extension;
		bool isLocalReference;
		bool isSaved;

		ULONG editChangeBegin;
		ULONG editChangeEnd;
		bool syntaxColoringIsCurrent;
};