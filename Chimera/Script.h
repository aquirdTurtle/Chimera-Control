#pragma once

#include "Control.h"
#include "myButton.h"
#include "RunInfo.h"
#include "VariableStructures.h"
#include "AoStructures.h"
#include "Combo.h"
#include "functionCombo.h"
#include <string>
#include <unordered_map>
#include <string>
#include <vector>
#include <array>

#include "stdafx.h"

#include "Script.h"

class Script
{
	public:
		Script();
		void initialize( int width, int height, POINT& startingLocation, cToolTips& toolTips, CWnd* scriptWin, int& id,
						 std::string deviceTypeInput, std::string scriptHeader, std::array<UINT, 2> ids, 
						 COLORREF backgroundColor );
		void handleToolTip( NMHDR * pNMHDR, LRESULT * pResult );
		std::string getScriptText();
		void setScriptText( std::string text );
		void functionChangeHandler( std::string categoryPath );
		void rearrange( UINT width, UINT height, fontMap fonts );
		void colorEntireScript( std::vector<parameterType> vars, 
								std::array<std::array<std::string, 16>, 4> ttlNames, std::array<AoInfo, 24> dacNames);
		void colorScriptSection( DWORD beginingOfChange, DWORD endOfChange, std::vector<parameterType> vars, 
								 std::array<std::array<std::string, 16>, 4> ttlNames,
								 std::array<AoInfo, 24> dacNames);
		COLORREF getSyntaxColor( std::string word, std::string editType, std::vector<parameterType> variables,
								 bool& colorLine,
								 std::array<std::array<std::string, 16>, 4> ttlNames,
								 std::array<AoInfo, 24> dacInfo );

		INT_PTR colorControl( LPARAM lParam, WPARAM wParam );
		void handleEditChange();
		void handleTimerCall( std::vector<parameterType> vars, 
							  std::array<std::array<std::string, 16>, 4> ttlNames, std::array<AoInfo, 24> dacNames);
		void changeView( std::string viewName, bool isFunction, std::string categoryPath );
		void saveScript( std::string location, RunInfo info );
		void saveScriptAs( std::string location, RunInfo info );
		void renameScript( std::string categoryPath );
		void deleteScript( std::string categoryPath );
		void newScript( );
		void newFunction();

		std::string getScriptPathAndName();
		std::string getScriptPath();
		std::string getScriptName();
		std::string getExtension();

		void loadFile( std::string pathToFile );
		void openParentScript( std::string parentScriptFileAndPath, std::string categoryPath, RunInfo info );
		void considerCurrentLocation( std::string categoryPath, RunInfo info );
		void checkSave( std::string categoryPath, RunInfo info );
		void updateSavedStatus( bool isSaved );
		bool coloringIsNeeded();
		void updateScriptNameText( std::string path );
		HBRUSH handleColorMessage ( CWnd* window, CDC* cDC );
		void reset();
		bool savedStatus();

		INT_PTR handleColorMessage( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, brushMap brushes );

		void saveAsFunction();
		void loadFunctions();

		void setEnabled (bool enabled );

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
		std::string scriptCategory;
		std::string deviceType;
		std::string extension;
		bool isLocalReference;
		bool isSaved;

		ULONG editChangeBegin;
		ULONG editChangeEnd;
		bool syntaxColoringIsCurrent;
};