// created by Mark O. Brown
#include "stdafx.h"

#include "Scripts/Script.h"

#include "PrimaryWindows/IChimeraWindowWidget.h"
#include "GeneralUtilityFunctions/cleanString.h"
#include "ParameterSystem/ParameterSystem.h"
#include "ConfigurationSystems/ProfileSystem.h"
#include "PrimaryWindows/AuxiliaryWindow.h"
#include "DigitalOutput/DoSystem.h"
#include "GeneralObjects/RunInfo.h"
#include <PrimaryWindows/QtMainWindow.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include "boost/lexical_cast.hpp"
#include <qcombobox.h>
#include <QInputDialog>

void Script::initialize (int width, int height, POINT& loc, IChimeraWindowWidget* parent,
	std::string deviceTypeInput, std::string scriptHeader){
	deviceType = deviceTypeInput;
	ScriptableDevice devenum;
	if (deviceTypeInput == "NIAWG") {
		devenum = ScriptableDevice::NIAWG;
		extension = str (".") + NIAWG_SCRIPT_EXTENSION;
	}
	else if (deviceTypeInput == "Agilent") {
		devenum = ScriptableDevice::Agilent;
		extension = str (".") + AGILENT_SCRIPT_EXTENSION;
	}
	else if (deviceTypeInput == "Master") {
		devenum = ScriptableDevice::Master;
		extension = str (".") + MASTER_SCRIPT_EXTENSION;
	}
	else {
		thrower (": Device input type not recognized during construction of script control.  (A low level bug, "
			"this shouldn't happen)");
	}
	// title
	if (scriptHeader != "")	{
		title = new QLabel (cstr (scriptHeader), parent);
		title->setGeometry (loc.x, loc.y, width, 25);
		loc.y += 25;
	}
	savedIndicator = new CQCheckBox ("Saved?", parent);
	savedIndicator->setGeometry (loc.x, loc.y, 80, 20);
	savedIndicator->setChecked (true);
	savedIndicator->setEnabled (false);
	fileNameText = new QLabel ("", parent);
	fileNameText->setGeometry (loc.x + 80, loc.y, width - 100, 20);
	isSaved = true;
	help = new QLabel ("?", parent);
	help->setGeometry (loc.x + width - 20, loc.y, 20, 20);
	// don't want this for the scripting window, hence the extra check.
	if (deviceType == "Agilent"){
		help->setToolTip (AGILENT_INFO_TEXT);
	}
	loc.y += 20;
	availableFunctionsCombo.combo = new CQComboBox (parent);
	availableFunctionsCombo.combo->setGeometry (loc.x, loc.y, width, 25);
	loadFunctions ();
	availableFunctionsCombo.combo->setCurrentIndex (0);
	loc.y += 25;

	edit = new CQTextEdit ("", parent);
	edit->setGeometry (loc.x, loc.y, width, height);
	edit->setStyleSheet ("font: bold 11pt \"Courier New\"");
	parent->connect (edit, &QTextEdit::textChanged, [parent, this]() {
			try {
				handleEditChange ();
			}
			catch (Error& err) {
				parent->reportErr (err.trace ());
			}
		});
	highlighter = new SyntaxHighlighter (devenum, edit->document ());
	loc.y += height;
}

void Script::rearrange(UINT width, UINT height, fontMap fonts)
{
}


void Script::functionChangeHandler(std::string configPath)
{
	int selection = availableFunctionsCombo.combo->currentIndex( );
	if ( selection != -1 )
	{
		CString text;
		availableFunctionsCombo.combo->currentText();
		std::string textStr( text.GetBuffer( ) );
		textStr = textStr.substr( 0, textStr.find_first_of( '(' ) );
		changeView( textStr, true, configPath );
	}
}


Script::Script()
{
	isSaved = true;
	editChangeEnd = 0;
	editChangeBegin = ULONG_MAX;
}

std::string Script::getScriptPath()
{
	return scriptPath;
}

std::string Script::getScriptText()
{
	if (!edit)
	{
		return "";
	}
	return str(edit->toPlainText());
}


COLORREF Script::getSyntaxColor( std::string word, std::string editType, std::vector<parameterType> params, 
								 std::vector<parameterType> localParams, bool& colorLine, Matrix<std::string> ttlNames,
								 std::array<AoInfo, 24> dacInfo )
{
	// convert word to lower case.
	std::transform( word.begin(), word.end(), word.begin(), ::tolower );
	// check special cases
	if (word.size() == 0)
	{
		// nothing??
		return _myRGBs["Solarized Red"];
	}
	else if (word[0] == '%')
	{
		// comments
		colorLine = true;
		if (word.size() > 1)
		{
			if (word[1] == '%')
			{
				return _myRGBs["Slate Green"];
			}
		}
		return _myRGBs["Slate Grey"];
	}
	if (word == "/*" || word == "*/")
	{
		return _myRGBs["Slate Green"];
	}
	if (word == "+" || word == "=" || word == "(" || word == ")" || word == "*" || word == "-" || word == "/" ||
		word == "sin" || word == "cos" || word == "tan" || word == "exp" || word == "ln" || word == "var")
	{
		return _myRGBs["Solarized Cyan"];
	}

	for (const auto& var : params)
	{
		if (word == var.name)
		{
			return _myRGBs["Solarized Green"];
		}
	}
	for (const auto& var : localParams)
	{
		if (word == var.name)
		{
			return _myRGBs["Solarized Blue"];
		}
	}
	// check delimiter
	if (word == "#")
	{
		return _myRGBs["Light Grey 2"];
	}
	// see if it's a double.
	try
	{
		boost::lexical_cast<double>(word);
		return _myRGBs["White"];
	}
	catch (boost::bad_lexical_cast&) {}
	// Check NIAWG-specific commands
	if ( editType == "NIAWG")
	{
		for ( auto num : range( MAX_NIAWG_SIGNALS ) )
		{
			if ( word == "gen" + str( num + 1 ) + "const" || word == "gen" + str( num + 1 ) + "ampramp"
				|| word == "gen" + str( num + 1 ) + "freqramp" || word == "gen" + str( num + 1 ) + "freq&ampramp")
			{
				return _myRGBs["Solarized Violet"];
			}
			if (word == "gen" + str (num + 1) + "const_v" || word == "gen" + str (num + 1) + "ampramp_v"
				|| word == "gen" + str (num + 1) + "freqramp_v" || word == "gen" + str (num + 1) + "freq&ampramp_v")
			{
				return _myRGBs["Solarized Violet"];
			}

		}
		if ( word == "flash" || word == "rearrange" || word == "horizontal" || word == "vertical" )
		{
			return _myRGBs["Solarized Violet"];
		}
		// check logic
		if ( word == "repeattiltrig" || word == "repeatSet#" || word == "repeattilsoftwaretrig" || word == "endrepeat" 
			 || word == "repeatforever" )
		{
			return _myRGBs["Solarized Blue"];
		}
		// check options
		if (word == "lin" || word == "nr" || word == "tanh")
		{
			return _myRGBs["Solarized Green"];
		}
		// check variable
		else if (word == "{" || word == "}" || word == "[" || word == "]" || word == "var_v")
		{
			return _myRGBs["Solarized Cyan"];
		}
		if (word.size() > 8)
		{
			if (word.substr(word.size() - 8, 8) == ".nScript")
			{
				return _myRGBs["Solarized Yellow"];
			}
		}
	}
	// Check Agilent-specific commands
	else if (editType == "Agilent")
	{
		std::transform(word.begin(), word.end(), word.begin(), ::tolower);
		if (word == "ramp" || word == "hold" || word == "pulse" )
		{
			return _myRGBs["Solarized Violet"];
		}
		else if ( word == "once" || word == "oncewaittrig" || word == "lin" || word == "tanh" 
				  || word == "repeatuntiltrig" || "repeat" || word == "sech" || word == "gaussian" || word == "lorentzian" )
		{
			return _myRGBs["Solarized Yellow"];
		}
	}
	else if (editType == "Master")
	{
		if (word == "on:" || word == "off:" || word == "pulseon:" || word == "pulseoff:")
		{
			return _myRGBs["Solarized Violet"];
		}
		if (word == "dac:" || word == "dacarange:" || word == "daclinspace:")
		{
			return _myRGBs["Solarized Yellow"];
		}
		else if (word == "call")
		{
			colorLine = true;
			return _myRGBs["Solarized Blue"];
		}
		else if (word == "def")
		{
			colorLine = true;
			return _myRGBs["Solarized Blue"];
		}
		else if ( word == "rsg:" || word == "repeat:" || word == "end" || word == "callcppcode" 
				  || word == "loadskipentrypoint!")
		{
			return _myRGBs["Solarized Yellow"];
		}
		else if (word == "t")
		{
			return _myRGBs["White"];
		}
		for (auto rowInc : range(ttlNames.getRows()))
		{
			std::string row;
			switch (rowInc)
			{
				case 0: row = "a"; break;
				case 1: row = "b"; break;
				case 2: row = "c"; break;
				case 3: row = "d"; break;
			}
			for (UINT numberInc : range(ttlNames.data[rowInc].size()))
			{				
				if (word == ttlNames(rowInc,numberInc) || word == row + str (numberInc)) 
				{ 
					return _myRGBs["Solarized Cyan"]; 
				}
			}
		}
		for (auto dacInc : range(dacInfo.size()))
		{
			if (word == dacInfo[dacInc].name || word == "dac" + str (dacInc))
			{
				return _myRGBs["Solarized Orange"];
			}
		}
	}
	return _myRGBs["Solarized Red"];
}


void Script::updateSavedStatus(bool scriptIsSaved)
{
	isSaved = scriptIsSaved;
	savedIndicator->setChecked ( scriptIsSaved );
}


bool Script::coloringIsNeeded()
{
	return !syntaxColoringIsCurrent;
}


void Script::handleTimerCall(std::vector<parameterType> vars,  Matrix<std::string> ttlNames, std::array<AoInfo, 24> dacInfo )
{
	if (!edit)
	{
		return;
	}
	if (!syntaxColoringIsCurrent)
	{
		// preserve saved state
		bool tempSaved = false;
		if (isSaved == true)
		{
			tempSaved = true;
		}
		DWORD x1 = 1, x2 = 1;
		int initScrollPos, finScrollPos;
		CHARRANGE charRange;
		//edit.GetSel(charRange);
		//initScrollPos = edit.GetScrollPos(SB_VERT);
		// color syntax
		colorEntireScript (vars, ttlNames, dacInfo);
		//colorScriptSection (editChangeBegin, editChangeEnd, vars, ttlNames, dacInfo);
		editChangeEnd = 0;
		editChangeBegin = ULONG_MAX;
		syntaxColoringIsCurrent = true;
		//edit.SetSel(charRange);
		//finScrollPos = edit.GetScrollPos(SB_VERT);
		//edit.LineScroll(-(finScrollPos - initScrollPos));
		updateSavedStatus(tempSaved);
	}
}

void Script::handleEditChange()
{
	// make sure the edit has been initialized.
	if (!edit)
	{
		return;
	}
	CHARRANGE charRange;
	//edit.GetSel(charRange);
	/*
	if (charRange.cpMin < editChangeBegin)
	{
		editChangeBegin = charRange.cpMin;
	}
	if (charRange.cpMax > editChangeEnd)
	{
		editChangeEnd = charRange.cpMax;
	}*/
	syntaxColoringIsCurrent = false;
	updateSavedStatus(false);
}


void Script::colorEntireScript( std::vector<parameterType> vars, Matrix<std::string> ttlNames, 
								std::array<AoInfo, 24> dacInfo ){
	colorScriptSection(0, ULONG_MAX, vars, ttlNames, dacInfo);
}

bool Script::positionIsInComment (DWORD position) 
{
	auto text = edit->toPlainText ();
	std::stringstream stream = std::stringstream (str (text));
	bool inComment = false;
	while (stream)
	{
		if (stream.tellg () == position)
		{
			return inComment;
		}
		char nextChar = stream.get ();
		if (nextChar == '/')
		{
			if (stream.get () == '*')
			{
				inComment = true;
			}
		}
		if (nextChar == '*')
		{
			if (stream.get () == '/')
			{
				inComment = false;
			}
		}
	}
	return inComment;
}

std::vector<parameterType> Script::getLocalParams () {
	if (!edit) { return{}; }
	auto text = edit->toPlainText ();
	std::stringstream fileTextStream = std::stringstream (str (text));
	ScriptStream ss (fileTextStream.str ());
	auto localVars = ExperimentThreadManager::getLocalParameters (ss);
	return localVars;
}

void Script::colorScriptSection( DWORD beginingOfChange, DWORD endOfChange, std::vector<parameterType> vars, 
								 Matrix<std::string> ttlNames, std::array<AoInfo, 24> dacInfo )
{
	if (!edit) { return; }
	//edit.SetRedraw( false );
	bool tempSaveStatus = isSaved;
	auto text = edit->toPlainText();
	std::string word, line;
	std::stringstream fileTextStream = std::stringstream(str(text));
	COLORREF syntaxColor, coloring;
	CHARFORMAT textFormat = { 0 };
	textFormat.cbSize = sizeof(CHARFORMAT);
	textFormat.dwMask = CFM_COLOR;
	DWORD startOfWord = 0, endOfWord = 0;
	std::size_t prev, endOfWordOffset;
	ScriptStream ss (fileTextStream.str ());
	auto localVars = ExperimentThreadManager::getLocalParameters (ss);
	bool commenting = positionIsInComment(beginingOfChange);
	if (commenting)
	{
		syntaxColor = _myRGBs["Slate Green"];
	}
	while (std::getline(fileTextStream, line))
	{
		DWORD lineStartCoordingate = startOfWord;
		int endTest = endOfWord + line.size();
		if (endTest < long long(beginingOfChange) - 15 || startOfWord > long long(endOfChange) + 15)
		{
			// not in a line close to the selected selection, so then skip to next line.
			endOfWord = endTest;
			startOfWord = endOfWord;
			continue;
		}
		prev = 0;
		coloring = 0;
		bool colorLine = false;
		std::string wordEndIndicators = " \t\r\n+=()-*/";
		while ((endOfWordOffset = line.find_first_of(wordEndIndicators, prev)) != std::string::npos)
		{
			if (endOfWordOffset == prev)
			{
				// then there was one of " \t\r\n+=" at the begging of the next string.
				endOfWordOffset++;
			}
			endOfWord = lineStartCoordingate + endOfWordOffset;
			word = line.substr(prev, endOfWordOffset - prev);
			if (word == " " || word == "\t" || word == "\r" || word == "\n")
			{
				// don't try to color whitespace.
				startOfWord = endOfWord;
				prev = endOfWordOffset;
				continue;
			}
			if (word == "/")
			{
				auto nextChar = line.substr (endOfWordOffset, 1);
				if (nextChar == "*")
				{
					word = "/*";
					endOfWordOffset += 1;
					endOfWord += 1;
					// comment start!
					commenting = true;
					syntaxColor = _myRGBs["Slate Green"];
				}
			}
			if (word == "*")
			{
				auto nextChar = line.substr (endOfWordOffset, 1);
				if (nextChar == "/")
				{
					word = "*/";
					endOfWordOffset += 1;
					endOfWord += 1;
					// comment end!
					commenting = false;
					syntaxColor = _myRGBs["Slate Green"];
				}
			}
			// if comment is found, the rest of the line is green.
			if (!colorLine)
			{
				if (!commenting)
				{
					syntaxColor = getSyntaxColor (word, deviceType, vars, localVars, colorLine, ttlNames, dacInfo);
				}
				if (syntaxColor != coloring)
				{
					// new color
					coloring = syntaxColor;
					textFormat.crTextColor = coloring;
					CHARRANGE charRange = { startOfWord, endOfWord };
					//edit.SetSel(charRange);
					//edit.SetSelectionCharFormat(textFormat);
					startOfWord = endOfWord;
				}
			}
			CHARRANGE charRange = { startOfWord, endOfWord };
			//edit.SetSel(charRange);
			//edit.SetSelectionCharFormat(textFormat);
			startOfWord = endOfWord;
			prev = endOfWordOffset;
		}
		// handle the endOfWord. above doesn't catch the endOfWord. There's probably a better way to do this.
		if (prev < std::string::npos)
		{
			bool colorLine = false;
			word = line.substr(prev, std::string::npos);
			endOfWord = lineStartCoordingate + line.length();
			// get all the params together
			if (!commenting)
			{
				syntaxColor = getSyntaxColor (word, deviceType, vars, localVars, colorLine, ttlNames, dacInfo);
			}
			if (!colorLine)
			{
				coloring = syntaxColor;
				textFormat.crTextColor = coloring;
				CHARRANGE charRange = { startOfWord, endOfWord };
				//edit.SetSel(charRange);
				//edit.SetSelectionCharFormat(textFormat);
				startOfWord = endOfWord;
			}
			CHARRANGE charRange = { startOfWord, endOfWord };
			//edit.SetSel(charRange);
			//edit.SetSelectionCharFormat(textFormat);
			startOfWord = endOfWord;
		}
	}
	//edit.SetRedraw( true );
	//edit.RedrawWindow();
	updateSavedStatus( tempSaveStatus );
}


void Script::handleToolTip( NMHDR * pNMHDR, LRESULT * pResult )
{
	/*
	TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
	UINT nID = pNMHDR->idFrom;
	if (pTTT->uFlags & TTF_IDISHWND)
	{
		nID = ::GetDlgCtrlID( (HWND)nID );
	}

	if (nID == help.GetDlgCtrlID())
	{
		// it's this window.
		// note that I don't think this is the hwnd of the help box, but rather the tooltip itself.
		::SendMessageA( pNMHDR->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 2500 );
		if (deviceType == "Master")
		{
			pTTT->lpszText = (LPSTR)MASTER_HELP;
		}
		else if( deviceType == "NIAWG")
		{
			pTTT->lpszText = (LPSTR)SCRIPT_INFO_TEXT;
		}
		else if (deviceType == "Agilent")
		{
			pTTT->lpszText = (LPSTR)AGILENT_INFO_TEXT;
		}
		else
		{
			pTTT->lpszText = "No Help available";
		}
		*pResult = 0;
		thrower ( "Worked." );
	}*/
	// else it's another window, just return and let them try.
}

void Script::changeView(std::string viewName, bool isFunction, std::string configPath)
{
	if (viewName == "Parent Script")
	{
		// load parent
		loadFile(configPath + scriptName + extension);
	}
	else if (isFunction)
	{
		loadFile(FUNCTIONS_FOLDER_LOCATION + viewName + "." + FUNCTION_EXTENSION);
	}
	else
	{
		// load child
		loadFile(configPath + viewName);
	}

	// colorEntireScript(params, rgbs, ttlNames, dacNames);
	// the view is fresh from a file, so it's saved.
	updateSavedStatus(true);
}


bool Script::isFunction ( )
{
	int sel = availableFunctionsCombo.combo->currentIndex();
	QString text;
	if ( sel != -1 )
	{
		text = availableFunctionsCombo.combo->currentText();
	}
	else
	{
		text = "";
	}
	return text != "Parent Script";// && text != "";
}

//
void Script::saveScript(std::string configPath, RunInfo info)
{
	if (configPath == "")
	{
		thrower (": Please select a configuration before trying to save a script!\r\n");
	}
	if (isSaved && scriptName != "")
	{
		// shoudln't need to do anything
		return;
	}
	if ( isFunction() )
	{
		errBox( "The current view is not the parent view. Please switch to the parent view before saving to "
				"save the script, or use the save-function option to save the current function." );
		return;
	}
	if ( scriptName == "" )
	{
		std::string newName;
		newName = str(QInputDialog::getText (edit, "New Script Name", ("Please enter new name for the " + deviceType + " script " + scriptName + ".",
											 scriptName).c_str()));
		if (newName == ""){
			// canceled
			return;
		}
		std::string path = configPath + newName + extension;
		saveScriptAs(path, info);
	}
	if (info.running)
	{
		for (UINT scriptInc = 0; scriptInc < info.currentlyRunningScripts.size(); scriptInc++)
		{
			if (scriptName == info.currentlyRunningScripts[scriptInc])
			{
				thrower ("System is currently running. You can't save over any files in use by the system while"
						 " it runs, which includes the NIAWG scripts and the intensity script.");
			}
		}
	}
	auto text = edit->toPlainText();
	std::fstream saveFile(configPath + scriptName + extension, std::fstream::out);
	if (!saveFile.is_open())
	{
		thrower ("Failed to open script file: " + configPath + scriptName + extension);
	}
	saveFile << str(text);
	saveFile.close();
	scriptFullAddress = configPath + scriptName + extension;
	scriptPath = configPath;
	updateSavedStatus(true);
}


//
void Script::saveScriptAs(std::string location, RunInfo info)
{
	if (location == "")
	{
		return;
	}
	if (info.running)
	{
		for (UINT scriptInc = 0; scriptInc < info.currentlyRunningScripts.size(); scriptInc++)
		{
			if (scriptName == info.currentlyRunningScripts[scriptInc])
			{
				thrower ("System is currently running. You can't save over any files in use by the system while "
						 "it runs, which includes the horizontal and vertical AOM scripts and the intensity script.");
			}
		}
	}
	auto text = edit->toPlainText();
	std::fstream saveFile(location, std::fstream::out);
	if (!saveFile.is_open())
	{
		thrower ("Failed to open script file: " + location);
	}
	saveFile << str(text);
	char fileChars[_MAX_FNAME];
	char dirChars[_MAX_FNAME];
	char pathChars[_MAX_FNAME];
	int myError = _splitpath_s(cstr(location), dirChars, _MAX_FNAME, pathChars, _MAX_FNAME, fileChars, _MAX_FNAME, NULL, 0);
	scriptName = str(fileChars);
	scriptPath = str(fileChars) + str(pathChars);
	saveFile.close();
	scriptFullAddress = location;
	updateScriptNameText(location);
	updateSavedStatus(true);
}


//
void Script::checkSave(std::string configPath, RunInfo info, Communicator* comm)
{
	if (isSaved)
	{
		// don't need to do anything
		return;
	}
	// test first non-commented word of text to see if this looks like a function or not.
	auto text = edit->toPlainText();
	ScriptStream tempStream;
	tempStream << str(text);
	std::string word;
	tempStream >> word;
	if (word == "def")
	{
		int answer = promptBox("Current " + deviceType + " function file is unsaved. Save it?", MB_YESNOCANCEL );
		if (answer == IDCANCEL)
		{
			thrower ("Cancel!");
		}
		else if (answer == IDNO)
		{
			return;
		}
		else if (answer == IDYES)
		{
			if ( comm == NULL )
			{
				thrower ( "ERROR: tried to save as function, but no communicator was passed to the checkSave function???" );
			}
			saveAsFunction(comm);
			return;
		}
	}
	// else it's a normal script file.
	if (scriptName == "")
	{
		int answer = promptBox("Current " + deviceType + " script file is unsaved and unnamed. Save it with a with new name?", MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			thrower ("Cancel!");
		}
		else if (answer == IDNO)
		{
			return;
		}
		else if (answer == IDYES)
		{
			std::string newName;
			newName = str (QInputDialog::getText (edit, "New Script Name", ("Please enter new name for the " + deviceType + " script " + scriptName + ".",
				scriptName).c_str ()));
			std::string path = configPath + newName + extension;
			saveScriptAs(path, info);
			return;
		}
	}
	else
	{
		int answer = promptBox("The " + deviceType + " script file is unsaved. Save it as " + scriptName 
								+ extension + "?", MB_YESNOCANCEL );
		if (answer == IDCANCEL)
		{
			thrower ("Cancel!");
		}
		else if (answer == IDNO)
		{
		}
		else if (answer == IDYES)
		{
			saveScript(configPath, info);
		}
	}
}


//
void Script::renameScript(std::string configPath)
{
	if (scriptName == "")
	{
		// ??? don't know why I need this here.
		return;
	}
	std::string newName;
	newName = str (QInputDialog::getText (edit, "New Script Name", ("Please enter new name for the " + deviceType + " script " + scriptName + ".",
		scriptName).c_str ()));
	if (newName == "")
	{
		// canceled
		return;
	}
	int result = MoveFile(cstr(configPath + scriptName + extension), cstr(configPath + newName + extension));

	if (result == 0)
	{
		thrower ("Failed to rename file. (A low level bug? this shouldn't happen)");
	}
	scriptFullAddress = configPath + scriptName + extension;
	scriptPath = configPath;
}


//
void Script::deleteScript(std::string configPath)
{
	if (scriptName == "")
	{
		return;
	}
	// check to make sure:
	int answer = promptBox("Are you sure you want to delete the script file " + scriptName + "?", MB_YESNO);
	if (answer == IDNO)
	{
		return;
	}
	int result = DeleteFile(cstr(configPath + scriptName + extension));
	if (result == 0)
	{
		thrower ("Deleting script file failed!  (A low level bug, this shouldn't happen)");
	}
	else
	{
		scriptName = "";
		scriptPath = "";
		scriptFullAddress = "";
	}
}


// the differences between this and new script are that this opens the default function instead of the default script
// and that this does not reset the script config, etc. 
void Script::newFunction()
{
	std::string tempName;
	tempName = DEFAULT_SCRIPT_FOLDER_PATH;
	if (deviceType == "Master")
	{
		tempName += "DEFAULT_FUNCTION.func";
	}
	else
	{
		thrower ("tried to load new function with non-master script? Only the master script supports functions"
				 " currently.");
	}
	loadFile(tempName);
	availableFunctionsCombo.combo->setCurrentIndex (-1);
}

//
void Script::newScript()
{
	std::string tempName;
	tempName = DEFAULT_SCRIPT_FOLDER_PATH;
	if (deviceType == "NIAWG")
	{
		tempName += "DEFAULT_SCRIPT.nScript";
	}
	else if (deviceType == "Agilent")
	{
		tempName += "DEFAULT_INTENSITY_SCRIPT.aScript";
	}
	else if (deviceType == "Master")
	{
		tempName += "DEFAULT_MASTER_SCRIPT.mScript";
	}	
	reset();
	loadFile(tempName);
}


void Script::openParentScript(std::string parentScriptFileAndPath, std::string configPath, RunInfo info)
{
	if (parentScriptFileAndPath == "" || parentScriptFileAndPath == "NONE")
	{
		return;
	}
	char fileChars[_MAX_FNAME];
	char extChars[_MAX_EXT];
	char dirChars[_MAX_FNAME];
	char pathChars[_MAX_FNAME];
	int myError = _splitpath_s(cstr(parentScriptFileAndPath), dirChars, _MAX_FNAME, pathChars, _MAX_FNAME, fileChars, 
								_MAX_FNAME, extChars, _MAX_EXT);
	std::string extStr(extChars);
	if (deviceType == "NIAWG")
	{
		if (extStr != str(".") + NIAWG_SCRIPT_EXTENSION)
		{
			thrower ("Attempted to open non-NIAWG script inside NIAWG script control.");
		}
	}
	else if (deviceType == "Agilent")
	{
		if (extStr != str( "." ) + AGILENT_SCRIPT_EXTENSION)
		{
			thrower ("Attempted to open non-agilent script from agilent script control.");
		}
	}
	else if (deviceType == "Master")
	{
		if (extStr != str( "." ) + MASTER_SCRIPT_EXTENSION)
		{
			thrower ("Attempted to open non-master script from master script control!");
		}
	}
	else
	{
		thrower ("Unrecognized device type inside script control!  (A low level bug, this shouldn't happen).");
	}
	loadFile( parentScriptFileAndPath );
	scriptName = str(fileChars);
	scriptFullAddress = parentScriptFileAndPath;
	updateSavedStatus(true);
	// Check location of NIAWG script.
	int sPos = parentScriptFileAndPath.find_last_of('\\');
	std::string scriptLocation = parentScriptFileAndPath.substr(0, sPos);
	if (scriptLocation + "\\" != configPath && configPath != "")
	{
		int answer = promptBox("The requested " + deviceType + " script: " + parentScriptFileAndPath + " is not "
								"currently located in the current configuration folder. This is recommended so that "
								"scripts related to a particular configuration are reserved to that configuration "
								"folder. Copy script to current configuration folder?", MB_YESNO);
		if (answer == IDYES)
		{
			std::string scriptName = parentScriptFileAndPath.substr(sPos+1, parentScriptFileAndPath.size());
			std::string path = configPath + scriptName;
			saveScriptAs(path, info);
		}
	}
	updateScriptNameText( configPath );
	int index = availableFunctionsCombo.combo->findData ("Parent Script");
	if (index != -1) { // -1 for not found
		availableFunctionsCombo.combo->setCurrentIndex (index);
	}
}

/*
]---	This function only puts the given file on the edit for this class, it doesn't change current settings parameters. It's used bare when just changing the
]-		view of the edit, while it's used with some surrounding changes for loading a new parent.
 */
void Script::loadFile(std::string pathToFile)
{
	std::fstream openFile;
	openFile.open(pathToFile, std::ios::in);
	if (!openFile.is_open())
	{
		reset();
		thrower ("Failed to open script file: " + pathToFile + ".");
	}
	std::string tempLine;
	std::string fileText;
	while (std::getline(openFile, tempLine))
	{
		cleanString(tempLine);
		fileText += tempLine;
	}
	// put the default into the new control.
	edit->setText(cstr(fileText));
	openFile.close();
}


void Script::reset()
{
	if (!availableFunctionsCombo.combo) {
		return;
	}
	int index = availableFunctionsCombo.combo->findData ("Parent Script");
	if (index != -1) { // -1 for not found
		availableFunctionsCombo.combo->setCurrentIndex (index);
	}
	scriptName = "";
	scriptPath = "";
	scriptFullAddress = "";
	updateSavedStatus(false);
	fileNameText->setText("");
	edit->setText("");
}


bool Script::savedStatus()
{
	return isSaved;
}

std::string Script::getScriptPathAndName()
{
	return scriptFullAddress;
}

std::string Script::getScriptName()
{
	return scriptName;
}

void Script::considerCurrentLocation(std::string configPath, RunInfo info)
{
	if (scriptFullAddress.size() > 0)
	{
		// Check location of NIAWG script.
		int sPos = scriptFullAddress.find_last_of('\\');
		std::string scriptLocation = scriptFullAddress.substr(0, sPos);
		if (scriptLocation + "\\" != configPath)
		{
			int answer = promptBox( "The requested " + deviceType + " script location: \"" + scriptLocation + "\" "
									"is not currently located in the current configuration folder. This is recommended"
									" so that scripts related to a particular configuration are reserved to that "
									"configuration folder. Copy script to current configuration folder?", MB_YESNO );
			if (answer == IDYES)
			{
				std::string scriptName = scriptFullAddress.substr(sPos, scriptFullAddress.size());
				scriptFullAddress = configPath + scriptName;
				scriptPath = configPath;
				saveScriptAs(scriptFullAddress, info);
			}
		}
	}
}

std::string Script::getExtension()
{
	return extension;
}

void Script::updateScriptNameText(std::string configPath)
{
	// there are some \\ on the endOfWord of the path by default.
	configPath = configPath.substr(0, configPath.size() - 1);
	int sPos = configPath.find_last_of('\\');
	if (sPos != -1)
	{
		std::string parentFolder = configPath.substr(sPos + 1, configPath.size());
		std::string text = parentFolder + "->" + scriptName;
		fileNameText->setText(cstr(text));
	}
	else
	{
		std::string text = "??? -> ";
		if (scriptName == "")
		{
			text += "???";
		}
		else
		{
			text += scriptName;
		}
		fileNameText->setText(cstr(text));
	}
}


void Script::setScriptText(std::string text)
{
	if (!edit) {
		return;
	}
	edit->setText( cstr( text ) );
}

void Script::saveAsFunction(Communicator* comm)
{
	// check to make sure that the current script is defined like a function
	auto text = edit->toPlainText();
	ScriptStream stream;
	stream << str(text);
	std::string word;
	stream >> word;
	if (word != "def")
	{
		thrower ("Function declarations must begin with \"def\".");
	}
	std::string line;
	line = stream.getline( '\r' );
	int pos = line.find_first_of("(");
	if (pos == std::string::npos)
	{
		thrower ("No \"(\" found in function name. If there are no arguments, use empty parenthesis \"()\"");
	}
	int initNamePos = line.find_first_not_of(" \t");
	std::string functionName = line.substr(initNamePos, line.find_first_of("("));
	if (functionName.find_first_of(" ") != std::string::npos)
	{
		thrower ("Function name included a space! Name was" + functionName);
	}
	std::string path = FUNCTIONS_FOLDER_LOCATION + functionName + "." + FUNCTION_EXTENSION;
	FILE *file;
	fopen_s( &file, cstr(path), "r" );
	if ( !file )
	{
		//
	}
	else
	{
		comm->sendStatus ( "Overwriting function definition for function at " + path + "...\r\n" );
		fclose ( file );
	}
	std::fstream functionFile(path, std::ios::out);
	if (!functionFile.is_open())
	{
		thrower ("the function file failed to open!");
	}
	functionFile << str(text);
	functionFile.close();
	// refresh this.
	loadFunctions();
	int index = availableFunctionsCombo.combo->findData (cstr (functionName));
	if (index != -1) { // -1 for not found
		availableFunctionsCombo.combo->setCurrentIndex (index);
	}
	updateSavedStatus( true );
}


void Script::setEnabled ( bool enabled, bool functionsEnabled )
{
	if (!availableFunctionsCombo.combo || !edit ) {
		return;
	}
	edit->setReadOnly (!enabled);
	availableFunctionsCombo.combo->setEnabled( functionsEnabled );
}


void Script::loadFunctions()
{
	availableFunctionsCombo.loadFunctions( );
}

