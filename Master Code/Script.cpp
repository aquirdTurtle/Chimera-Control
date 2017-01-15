#include "stdafx.h"
#include "Script.h"
#include "constants.h"
#include "fonts.h"
#include <sstream>
#include <algorithm>
#include "boost/lexical_cast.hpp"
#include "cleanString.h"
#include "textPromptDialogProcedure.h"
#include "Windows.h"
#include "Richedit.h"
#include "VariableSystem.h"
#include "ConfigurationFileSystem.h"
#include <iostream>
#include <fstream>
#include <string>
#include "MasterWindow.h"
#include <unordered_map>
#include "ExperimentManager.h"
#include "TTL_System.h"
#include "VariableSystem.h"
#include "MasterWindow.h"

bool Script::functionChangeHandler(MasterWindow* master)
{
	int selection = this->availableFunctionsCombo.GetCurSel();
	if (selection != -1)
	{
		CString text;
		this->availableFunctionsCombo.GetLBText(selection, text);
		std::string textStr(text.GetBuffer());
		textStr = textStr.substr(0, textStr.find_first_of('('));
		this->changeView(textStr, master, true);
	}
	return true;
}

Script::Script(std::string functionsLoc, std::string deviceTypeInput, int& idStart) : deviceType{ deviceTypeInput }, idStart{ idStart }, idEnd{ idStart + 4 }, functionLocation{ functionsLoc }
{
	this->savedIndicator.ID = idStart;
	this->title.ID = idStart + 1;
	this->childCombo.ID = idStart + 2;
	this->fileNameText.ID = idStart + 3;
	this->edit.ID = idStart + 4;
	if (this->edit.ID != MASTER_RICH_EDIT)
	{
		MessageBox(0, "ERROR: MASTER_RICH_EDIT doens't match actual ID", 0, 0);
	}
	this->availableFunctionsCombo.ID = idStart + 5;
	if (this->availableFunctionsCombo.ID != FUNCTION_COMBO_ID)
	{
		errBox("ERROG: functionsCombo.ID " + std::to_string(availableFunctionsCombo.ID) + " did not match FUNCTIONS_COMBO ID: " + std::to_string(FUNCTION_COMBO_ID));

	}
	this->help.ID = idStart + 6;
	idStart += 7;
	if (deviceTypeInput == "Horizontal NIAWG" || deviceTypeInput == "Vertical NIAWG")
	{
		extension = NIAWG_SCRIPT_EXTENSION;
	}
	else if (deviceTypeInput == "Agilent")
	{
		extension = AGILENT_SCRIPT_EXTENSION;
	}
	else if (deviceTypeInput == "Master")
	{
		extension = MASTER_SCRIPT_EXTENSION;
	}
	else
	{
		MessageBox(0, "ERROR: Bad device input type argument for scritp constructor.", 0, 0);
		extension = "Sadness :(";
	}
	isSaved = true;
	editChangeEnd = 0;
	editChangeBegin = ULONG_MAX;
}

Script::~Script(){}

std::string Script::getScriptPath()
{
	return this->scriptPath;
}

std::string Script::getScriptText()
{
	CString text;
	this->edit.GetWindowText(text);
	return text;
}

COLORREF Script::getSyntaxColor(std::string word, std::string editType, std::vector<variable> variables, std::unordered_map<std::string, COLORREF> rgbs, bool& colorLine, std::array<std::array<std::string, 16>, 4> ttlNames, std::array<std::string, 24> dacNames)
{
	// check special cases
	if (word.size() == 0)
	{
		return rgbs["Red"];
	}
	else if (word[0] == '%')
	{
		colorLine = true;
		if (word.size() > 1)
		{
			if (word[1] == '%')
			{
				return rgbs["Forest Green"];
			}
		}
		return rgbs["Dull Red"];
	}

	// Check NIAWG-specific commands
	if (editType == "Horizontal NIAWG" || editType == "Vertical NIAWG")
	{
		if (word == "gen" || word == "1," || word == "2," || word == "3," || word == "4," || word == "5," || word == "freq" || word == "amp" || word == "const"
			|| word == "&" || word == "ramp")
		{
			return rgbs["Dark Lavendar"];
		}
		// check logic
		if (word == "repeat" || word == "until" || word == "trigger" || word == "end" || word == "forever")
		{
			return rgbs["Teal"];
		}
		// check options
		if (word == "lin" || word == "nr" || word == "tanh")
		{
			return rgbs["Tan"];
		}
		// check variable

		if (word == "predefined" || word == "script")
		{
			return rgbs["Purple"];
		}
		if (word.size() > 8)
		{
			if (word.substr(word.size() - 8, 8) == ".nScript")
			{
				return rgbs["Purple"];
			}
		}
	}
	// check Agilent-specific commands
	else if (editType == "Agilent")
	{
		std::transform(word.begin(), word.end(), word.begin(), ::tolower);
		if (word == "agilent" || word == "ramp" || word == "hold" || word == "predefined" || word == "script")
		{
			return rgbs["Dark Lavender"];
		}
		if (word == "once" || word == "oncewaittrig" || word == "lin" || word == "tanh" || word == "repeatuntiltrig")
		{
			return rgbs["Tan"];
		}
		if (word.size() > 8)
		{
			if (word.substr(word.size() - 8, 8) == ".aScript")
			{
				return rgbs["Purple"];
			}
		}
	}
	else if (editType == "Master")
	{
		if (word == "on:" || word == "dac:")
		{
			return rgbs["Dark Lavender"];
		}
		if (word == "off:")
		{
			return rgbs["Tan"];
		}
		else if (word == "call")
		{
			colorLine = true;
			return rgbs["Purple"];
		}
		else if (word == "t" || word == "t++" || word == "++" || word == "t+=" || word == "+=" || word == "t=" || word == "=")
		{
			return rgbs["Orange"];
		}
		else if (word == "def")
		{
			colorLine = true;
			return rgbs["Light Blue"];
		}
		for (int rowInc = 0; rowInc < ttlNames.size(); rowInc++)
		{
			std::string row;
			switch (rowInc)
			{
				case 0: row = "A"; break;
				case 1: row = "B"; break;
				case 2: row = "C"; break;
				case 3: row = "D"; break;
			}
			for (int numberInc = 0; numberInc < ttlNames[rowInc].size(); numberInc++)
			{
				
				if (word == ttlNames[rowInc][numberInc])
				{
					return rgbs["Gold"];
				}
				if (word == row + std::to_string(numberInc + 1))
				{
					return rgbs["Gold"];
				}
			}
		}
		for (int dacInc = 0; dacInc < dacNames.size(); dacInc++)
		{
			if (word == dacNames[dacInc])
			{
				return rgbs["Brown"];
			}
			if (word == "dac" + std::to_string(dacInc + 1))
			{
				return rgbs["Brown"];
			}
		}
	}
	for (int varInc = 0; varInc < variables.size(); varInc++)
	{
		if (word == variables[varInc].name)
		{
			return rgbs["Teal"];
		}
	}
	// check delimiter
	if (word == "#")
	{
		return rgbs["Light Grey"];
	}
	// see if it's a double.
	try
	{
		boost::lexical_cast<double>(word);
		return rgbs["White"];
	}
	catch (boost::bad_lexical_cast &)
	{
		return rgbs["Red"];
	}
}

bool Script::updateSavedStatus(bool scriptIsSaved)
{
	this->isSaved = scriptIsSaved;
	if (scriptIsSaved)
	{
		savedIndicator.SetCheck( true );
	}
	else
	{
		savedIndicator.SetCheck( false );
	}
	return false;
}

bool Script::coloringIsNeeded()
{
	return !this->syntaxColoringIsCurrent;
}

bool Script::handleTimerCall(MasterWindow* Master)
{
	if (!this->syntaxColoringIsCurrent)
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
		this->edit.GetSel(charRange);
		initScrollPos = this->edit.GetScrollPos(SB_VERT);
		// color syntax
		this->colorScriptSection(editChangeBegin, editChangeEnd, Master);
		editChangeEnd = 0;
		editChangeBegin = ULONG_MAX;
		syntaxColoringIsCurrent = true;
		this->edit.SetSel(charRange);
		finScrollPos = this->edit.GetScrollPos(SB_VERT);
		edit.LineScroll(-(finScrollPos - initScrollPos));
		//SendMessage(edit.hwnd, EM_LINESCROLL, 0, -(finScrollPos - initScrollPos));
		this->updateSavedStatus(tempSaved);
	}
	return true;
}

bool Script::handleEditChange(MasterWindow* master)
{
		DWORD begin, end;
		CHARRANGE charRange;
		edit.GetSel(charRange);
		if (charRange.cpMin < editChangeBegin)
		{
			editChangeBegin = charRange.cpMin;
		}
		if (charRange.cpMax > editChangeEnd)
		{
			editChangeEnd = charRange.cpMax;
		}
		master->SetTimer(SYNTAX_TIMER_ID, SYNTAX_TIMER_LENGTH, NULL);
		//this->syntaxTimer.SetTimer(SYNTAX_TIMER_ID, SYNTAX_TIMER_LENGTH, NULL);
		syntaxColoringIsCurrent = false;
		this->updateSavedStatus(false);
		return true;
}

bool Script::colorEntireScript(MasterWindow* Master)
{
	return (this->colorScriptSection(0, ULONG_MAX, Master));
}

bool Script::colorScriptSection(DWORD beginingOfChange, DWORD endOfChange, MasterWindow* Master)
{
	long long beginingSigned = beginingOfChange;
	long long endSigned = endOfChange;
	CString text;
	edit.GetWindowTextA(text);
	std::string script(text);
	std::vector<std::string> predefinedScripts;
	COLORREF coloring;
	std::string word;
	std::stringstream fileTextStream(script);
	std::string line;
	std::string currentTextToAdd;
	std::string analysisWord;
	std::string tempColor;
	COLORREF syntaxColor;
	CHARFORMAT syntaxFormat;
	memset(&syntaxFormat, 0, sizeof(CHARFORMAT));
	syntaxFormat.cbSize = sizeof(CHARFORMAT);
	syntaxFormat.dwMask = CFM_COLOR;
	//int relevantID = GetDlgCtrlID(edit.hwnd);
	DWORD start = 0, end = 0;
	std::size_t prev, pos;

	while (std::getline(fileTextStream, line))
	{
		DWORD lineStartCoordingate = start;
		int endTest = end + line.size();
		if (endTest < beginingSigned - 5 || start > endSigned)
		{
			// then skip to next line.
			end = endTest;
			start = end;
			continue;
		}
		prev = 0;
		coloring = 0;
		bool colorLine = false;
		while ((pos = line.find_first_of(" \t\r\n", prev)) != std::string::npos)
		{
			end = lineStartCoordingate + pos;
			word = line.substr(prev, pos - prev + 1);
			// kill whatever is on the end of it.
			analysisWord = word.substr(0, word.length() - 1);
			// if comment is found, the rest of the line is green.
			if (!colorLine)
			{
				std::vector<variable> vars = Master->variables.getEverything();
				syntaxColor = this->getSyntaxColor(analysisWord, deviceType, vars, Master->getRGBs(), colorLine, Master->ttlBoard.getAllNames(), Master->dacBoards.getAllNames());
				if (syntaxColor != coloring)
				{
					coloring = syntaxColor;
					syntaxFormat.crTextColor = coloring;
					CHARRANGE charRange;
					charRange.cpMin = start;
					charRange.cpMax = end;
					edit.SetSel(charRange);
					edit.SetSelectionCharFormat(syntaxFormat);
					start = end;
				}
			}
			CHARRANGE charRange;
			charRange.cpMin = start;
			charRange.cpMax = end;
			edit.SetSel(charRange);
			edit.SetSelectionCharFormat(syntaxFormat);
			end++;
			start = end;
			prev = pos + 1;
		}
		if (prev < std::string::npos)
		{
			bool colorLine = false;
			word = line.substr(prev, std::string::npos);
			end = lineStartCoordingate + line.length();
			std::vector<variable> vars = Master->variables.getEverything();
			syntaxColor = this->getSyntaxColor(analysisWord, deviceType, vars, Master->getRGBs(), colorLine, Master->ttlBoard.getAllNames(), 
				Master->dacBoards.getAllNames());
			if (!colorLine)
			{
				coloring = syntaxColor;
				syntaxFormat.crTextColor = coloring;
				CHARRANGE charRange;
				charRange.cpMin = start;
				charRange.cpMax = end;
				edit.SetSel(charRange);
				edit.SetSelectionCharFormat(syntaxFormat);
				start = end;
			}
			CHARRANGE charRange;
			charRange.cpMin = start;
			charRange.cpMax = end;
			edit.SetSel(charRange);
			edit.SetSelectionCharFormat(syntaxFormat);
			start = end;
		}
	}
	this->updateChildCombo(Master);
	return false;
}

bool Script::updateChildCombo(MasterWindow* Master)
{
	// check the current setting.
	int selection = childCombo.GetCurSel();
	TCHAR text[256];
	childCombo.GetLBText( selection, text );
	std::string textStr(text);
	if (textStr != "Parent Script")
	{
		// if not parent, don't update the child combo.
		return false;
	}
	CString myText;
	edit.GetWindowTextA(myText);
	// get script
	// get the name that's currently set.
	selection = childCombo.GetCurSel();
	TCHAR name[256];
	childCombo.GetLBText( selection, name );
	std::string currentName(name);
	// I'll use this later to reset the combo.
	// reset the combo and name vector.
	childCombo.ResetContent();

	childrenNames.clear();
	std::string script(myText);
	// look for predefined scripts.
	std::stringstream scriptStream(script);
	while (scriptStream)
	{
		std::string line;
		std::getline(scriptStream, line);
		// remove trailing '\r' from line.
		line = line.substr(0, line.size() - 1);
		std::transform(line.begin(), line.end(), line.begin(), ::tolower);
		if (line == "predefined script")
		{
			std::string scriptName;
			std::getline(scriptStream, scriptName);
			// remove trailing '\r'
			scriptName = scriptName.substr(0, scriptName.size() - 1);
			// test if script exists in nearby folder.
			struct stat buffer;
			std::string path = Master->profile.getCurrentPathIncludingCategory() + scriptName;
			if (stat(path.c_str(), &buffer) == 0)
			{
				// add to combo normally.
				childCombo.AddString( scriptName.c_str() );
				childrenNames.push_back(scriptName);
			}
			else
			{
				childCombo.AddString( (scriptName + " (File Not Found!)").c_str() );
			}
		}
	}
	// add the parent string message
	childCombo.AddString( "Parent Script" );
	// reset the child window.
	childCombo.SelectString( 0, currentName.c_str() );
	return false;
}

bool Script::getControlIDRange(int& start, int& fin)
{
	start = idStart;
	fin = idEnd;
	return false;
}

INT_PTR Script::colorControl(LPARAM lParam, WPARAM wParam)
{
	int controlID = GetDlgCtrlID((HWND)lParam);
	HDC hdcStatic = (HDC)wParam;
	if (controlID == edit.ID)
	{
		SetTextColor(hdcStatic, RGB(255, 255, 255));
		SetBkColor(hdcStatic, RGB(50, 45, 45));
		//return (INT_PTR)eGreyRedBrush;
	}
	else if (controlID == title.ID)
	{
		SetTextColor(hdcStatic, RGB(255, 255, 255));
		SetBkMode(hdcStatic, TRANSPARENT);
		SetBkColor(hdcStatic, RGB(75, 0, 0));
		//return (INT_PTR)eDarkRedBrush;
	}
	else if (controlID == savedIndicator.ID)
	{
		SetTextColor(hdcStatic, RGB(255, 255, 255));
		SetBkMode(hdcStatic, TRANSPARENT);
		SetBkColor(hdcStatic, RGB(50, 45, 45));
		//return (INT_PTR)eGreyRedBrush;
	}
	else if (controlID == childCombo.ID)
	{
		return false;
	}
	else if (controlID == fileNameText.ID)
	{
		SetTextColor(hdcStatic, RGB(255, 255, 255));
		SetBkMode(hdcStatic, TRANSPARENT);
		SetBkColor(hdcStatic, RGB(25, 0, 0));
		//return (INT_PTR)eDullRedBrush;
	}
	else
	{
		return false;
	}
	return true;
}

bool Script::initialize(int width, int height, POINT& startingLocation, std::vector<CToolTipCtrl*>& toolTips, HWND parentWindow, MasterWindow* master)
{
	AfxInitRichEdit();
	InitCommonControls();
	LoadLibrary(TEXT("Msftedit.dll"));
	CHARFORMAT myCharFormat;
	memset(&myCharFormat, 0, sizeof(CHARFORMAT));
	myCharFormat.cbSize = sizeof(CHARFORMAT);
	myCharFormat.dwMask = CFM_COLOR;
	myCharFormat.crTextColor = RGB(255, 255, 255);

	std::string titleText;
	if (deviceType == "Horizontal NIAWG")
	{
		titleText = "HORIZONTAL NIAWG SCRIPT";
	}
	else if (deviceType == "Vertical NIAWG")
	{
		titleText = "VERTICAL NIAWG SCRIPT";
	}
	else if (deviceType == "Agilent")
	{
		titleText = "AGILENT INTENSITY SCRIPT";
	}
	else if (deviceType == "Master")
	{
		titleText = "MASTER SCRIPT";
	}
	// title
	title.position = { startingLocation.x, startingLocation.y, startingLocation.x + width, startingLocation.y + 20 };
	title.Create( titleText.c_str(), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, title.position, master, title.ID );
	title.SetFont( CFont::FromHandle( sHeadingFont ) );
	startingLocation.y += 20;
	// saved indicator
	savedIndicator.position = { startingLocation.x, startingLocation.y, startingLocation.x + 80, startingLocation.y + 20 };
	savedIndicator.Create( "Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT, savedIndicator.position, master, savedIndicator.ID );
	savedIndicator.SetFont( CFont::FromHandle( sNormalFont ) );
	savedIndicator.SetCheck( true );	
	// filename
	fileNameText.position = { startingLocation.x + 80, startingLocation.y, startingLocation.x + width - 20, startingLocation.y + 20 };
	fileNameText.Create(WS_CHILD | WS_VISIBLE | SS_ENDELLIPSIS, fileNameText.position, master, fileNameText.ID);
	fileNameText.SetFont(CFont::FromHandle(sHeadingFont));
	isSaved = true;
	// help
	this->help.position = { startingLocation.x + width - 20, startingLocation.y, startingLocation.x + width, startingLocation.y + 20 };
	//this->help.Create("?", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, itemBox, CWnd::FromHandle(parentWindow), this->help.ID);
	this->help.Create(WS_CHILD | WS_VISIBLE | ES_READONLY, help.position, master, this->help.ID);
	help.SetWindowTextA("?");
	help.setToolTip("This is a script for programming master timing for TTLs, DACs, the RSG, and the raman outputs.\n"
					"Acceptable Commands:\n"
					"-      t++\n"
					"-      t+= [number] (space between = and number required)\n"
					"-      t= [number] (space between = and number required)\n"
					"-      on: [ttlName]\n"
					"-      off: [ttlName]\n"
					"-      dac: [dacName] [voltage]\n"
					"-      dacramp: [dacName] [initValue] [finalValue] [rampTime] [rampInc]"
					"-      rsg: [frequency to add]"
					"-      raman: [topFreq] [bottomFreq] [axialFreq]"
					"-      call [functionName(argument1, argument2, etc...)]\n"
					"-      % marks a line as a comment. %% does the same and gives you a different color."
					"-      extra white-space is generally fine and doesn't screw up analysis of the script. Format as you like.", 
					toolTips, master);
	//this->help.setToolTip();
	startingLocation.y += 20;
	// available functions combo
	availableFunctionsCombo.position = { startingLocation.x, startingLocation.y, startingLocation.x + width, startingLocation.y + 800 };
	availableFunctionsCombo.Create(CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, availableFunctionsCombo.position, master, availableFunctionsCombo.ID);
	availableFunctionsCombo.SetFont(CFont::FromHandle(sNormalFont));
	availableFunctionsCombo.AddString("Available Functions List:");
	availableFunctionsCombo.SetCurSel(0);
	this->loadFunctions();
	
	startingLocation.y += 25;
	// children combo
	childCombo.position = { startingLocation.x, startingLocation.y, startingLocation.x + width, startingLocation.y + 800 };
	childCombo.Create( CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, childCombo.position, master, childCombo.ID );
	childCombo.SetFont( CFont::FromHandle( sNormalFont ) );
	childCombo.AddString( "Parent Script" );
	childCombo.SetCurSel( 0 );
	startingLocation.y += 25;
	// Edit
	edit.position = { startingLocation.x, startingLocation.y, startingLocation.x + width, height};
	edit.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | WS_HSCROLL | ES_WANTRETURN | WS_BORDER,
				 edit.position, master, edit.ID);
	edit.SetFont(CFont::FromHandle(sCodeFont));
	edit.SetBackgroundColor(FALSE, RGB(15, 15, 15));
	edit.SetEventMask(ENM_CHANGE);
	edit.SetDefaultCharFormat(myCharFormat);

	// timer
	this->syntaxTimer.Create(0, NULL, 0, { 0,0,0,0 }, CWnd::FromHandle(parentWindow), 0);

	return false;
}

bool Script::reorganizeControls()
{
	return false;
}

bool Script::childComboChangeHandler(WPARAM messageWParam, LPARAM messageLParam, MasterWindow* Master)
{
	int controlID = GetDlgCtrlID((HWND)messageLParam);
	if (controlID != this->childCombo.ID)
	{
		return true;
	}
	if (HIWORD(messageWParam) != CBN_SELCHANGE)
	{
		return true;
	}
	// prompt for save
	this->checkSave(Master);
	// get text from child

	// check what was selected.
	int selection = childCombo.GetCurSel();
	TCHAR selectedText[256];
	childCombo.GetLBText( selection, selectedText );
	std::string viewName(selectedText);
	this->changeView(viewName, Master, false);
	this->colorEntireScript(Master);
	this->updateSavedStatus(true);
	return false;
}

bool Script::changeView(std::string viewName, MasterWindow* Master, bool isFunction)
{
	if (viewName == "Parent Script")
	{
		// load parent
		this->loadFile(Master->profile.getCurrentPathIncludingCategory() + scriptName + extension);
	}
	else if (isFunction)
	{
		this->loadFile(FUNCTIONS_FOLDER_LOCATION + viewName + FUNCTION_EXTENSION);
	}
	else
	{
		// load child
		this->loadFile(Master->profile.getCurrentPathIncludingCategory() + viewName);
	}
	return false;
}
//
bool Script::saveScript(MasterWindow* Master)
{
	if (Master->profile.getCurrentPathIncludingCategory() == "")
	{
		MessageBox(0, "ERROR: Please select a category before trying to save a script!", 0, 0);
		return false;
	}
	if (isSaved && scriptName != "")
	{
		// shoudln't need to do anything
		return false;
	}
	if (scriptName == "")
	{
		std::string newName = (const char*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure,
			(LPARAM)("The " + deviceType + " script is unnamed Please enter new name for the script:").c_str());
		if (newName == "")
		{
			// canceled
			return true;
		}
		std::string path = Master->profile.getCurrentPathIncludingCategory() + newName + extension;
		this->saveScriptAs(path, Master);
	}
	if (Master->systemRunningInfo.running)
	{
		for (int scriptInc = 0; scriptInc < Master->systemRunningInfo.currentlyRunningScripts.size(); scriptInc++)
		{
			if (scriptName == Master->systemRunningInfo.currentlyRunningScripts[scriptInc])
			{
				MessageBox(0, "ERROR: System is currently running. You can't save over any files in use by the system while it runs, which includes the "
					"horizontal and vertical AOM scripts and the intensity script.", 0, 0);
				return true;
			}
		}
	}
	CString text;
	this->edit.GetWindowTextA(text);
	std::fstream saveFile(Master->profile.getCurrentPathIncludingCategory() + scriptName + extension, std::fstream::out);
	if (!saveFile.is_open())
	{
		MessageBox(0, ("ERROR: Failed to open script file: " + Master->profile.getCurrentPathIncludingCategory() + scriptName + extension).c_str(), 0, 0);
		return true;
	}
	saveFile << text;
	saveFile.close();
	this->scriptFullAddress = Master->profile.getCurrentPathIncludingCategory() + scriptName + extension;
	this->scriptPath = Master->profile.getCurrentPathIncludingCategory();
	this->updateSavedStatus(true);
	return true;
}
//
bool Script::saveScriptAs(std::string location, MasterWindow* Master)
{
	if (location == "")
	{
		return false;
	}
	if (Master->systemRunningInfo.running)
	{
		for (int scriptInc = 0; scriptInc < Master->systemRunningInfo.currentlyRunningScripts.size(); scriptInc++)
		{
			if (scriptName == Master->systemRunningInfo.currentlyRunningScripts[scriptInc])
			{
				MessageBox(0, "ERROR: System is currently running. You can't save over any files in use by the system while it runs, which includes the "
					"horizontal and vertical AOM scripts and the intensity script.", 0, 0);
				return false;
			}
		}
	}
	CString text;
	this->edit.GetWindowTextA(text);
	std::fstream saveFile(location, std::fstream::out);
	if (!saveFile.is_open())
	{
		MessageBox(0, ("ERROR: Failed to open script file: " + location).c_str(), 0, 0);
		return true;
	}
	saveFile << text;
	char fileChars[_MAX_FNAME];
	char dirChars[_MAX_FNAME];
	char pathChars[_MAX_FNAME];
	int myError = _splitpath_s(location.c_str(), dirChars, _MAX_FNAME, pathChars, _MAX_FNAME, fileChars, _MAX_FNAME, NULL, 0);
	scriptName = std::string(fileChars);
	this->scriptPath = std::string(fileChars) + std::string(pathChars);
	saveFile.close();
	this->scriptFullAddress = location;
	this->updateScriptNameText(location);
	this->updateSavedStatus(true);
	return true;
}
bool Script::checkChildSave(MasterWindow* Master)
{
	// get the 
	if (isSaved)
	{
		// don't need to do anything
		return false;
	}
	int selection = childCombo.GetCurSel();
	TCHAR name[256];
	childCombo.GetLBText( selection, name );
	std::string nameStr(name);
	if (nameStr == "")
	{
		int answer = MessageBox(0, ("Current " + deviceType + " script file is unsaved and unnamed. Save it with a with new name?").c_str(), 0, MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			return true;
		}
		else if (answer == IDNO)
		{
			return false;
		}
		else if (answer == IDYES)
		{
			std::string newName = (const char*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure,
				(LPARAM)("Please enter new name for the script " + scriptName + ".").c_str());
			std::string path = Master->profile.getCurrentPathIncludingCategory() + newName + this->extension;
			this->saveScriptAs(path, Master);
			return false;
		}
		else
		{
			MessageBox(0, "WTF", 0, 0);
			return true;
		}
	}
	else
	{
		int answer = MessageBox(0, ("Save " + deviceType + " script file as " + nameStr + "?").c_str(), 0, MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			return true;
		}
		else if (answer == IDNO)
		{
			return false;
		}
		else if (answer == IDYES)
		{
			this->saveScriptAs(nameStr, Master);
			return false;
		}
		else
		{
			MessageBox(0, "WTF", 0, 0);
			return true;
		}
	}
	return false;
}
//
bool Script::checkSave(MasterWindow* Master)
{
	if (isSaved)
	{
		// don't need to do anything
		return true;
	}
	if (scriptName == "")
	{
		int answer = MessageBox(0, ("Current " + deviceType + " script file is unsaved and unnamed. Save it with a with new name?").c_str(), 0, MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			return false;
		}
		else if (answer == IDNO)
		{
			return true;
		}
		else if (answer == IDYES)
		{
			std::string newName = (const char*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure,
				(LPARAM)("Please enter new name for the script " + scriptName + ".").c_str());
			std::string path = Master->profile.getCurrentPathIncludingCategory() + newName + this->extension;
			this->saveScriptAs(path, Master);
			return true;
		}
		else
		{
			MessageBox(0, "WTF", 0, 0);
			return false;
		}
	}
	else
	{
		int answer = MessageBox(0, ("The " + deviceType + " script file is unsaved. Save it as " + scriptName + extension + "?").c_str(), 0, MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			return false;
		}
		else if (answer == IDNO)
		{
			return true;
		}
		else if (answer == IDYES)
		{
			this->saveScript(Master);
			return true;
		}
		else
		{
			MessageBox(0, "WTF", 0, 0);
			return false;
		}
	}
	return true;
}
//
bool Script::renameScript(MasterWindow* Master)
{
	if (this->scriptName == "")
	{
		return false;
	}
	std::string newName = (const char*)DialogBoxParam(Master->programInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure,
		(LPARAM)("Please enter new name for the script " + scriptName + ".").c_str());
	if (newName == "")
	{
		// canceled
		return false;
	}
	int result = MoveFile((Master->profile.getCurrentPathIncludingCategory() + scriptName + extension).c_str(),
		(Master->profile.getCurrentPathIncludingCategory() + newName + extension).c_str());
	if (result == 0)
	{
		MessageBox(0, "ERROR: Failed to move file.", 0, 0);
		return true;
	}
	this->scriptFullAddress = Master->profile.getCurrentPathIncludingCategory() + scriptName + extension;
	this->scriptPath = Master->profile.getCurrentPathIncludingCategory();
	return false;
}
//
bool Script::deleteScript(MasterWindow* Master)
{
	if (this->scriptName == "")
	{
		return false;
	}
	// check to make sure:
	int answer = MessageBox(0, ("Are you sure you want to delete the script file " + this->scriptName + "?").c_str(), 0, MB_YESNO);
	if (answer == IDNO)
	{
		return false;
	}

	int result = DeleteFile((Master->profile.getCurrentPathIncludingCategory() + scriptName + extension).c_str());
	if (result == 0)
	{
		MessageBox(0, "ERROR: Deleting script file failed!", 0, 0);
		return true;
	}
	else
	{
		this->scriptName = "";
		this->scriptPath = "";
		this->scriptFullAddress = "";
	}
	return false;
}
//
bool Script::newScript(MasterWindow* Master)
{
	std::string tempName;
	tempName = DEFAULT_SCRIPT_FOLDER_PATH;
	if (deviceType == "Horizontal NIAWG")
	{
		if (Master->profile.getOrientation() == HORIZONTAL_ORIENTATION)
		{
			tempName += "DEFAULT_HCONFIG_HORIZONTAL_SCRIPT.nScript";
			//"DEFAULT_HORIZONTAL_SCRIPT.script"
		}
		else if (Master->profile.getOrientation() == VERTICAL_ORIENTATION)
		{
			tempName += "DEFAULT_VCONFIG_HORIZONTAL_SCRIPT.nScript";
		}
		else
		{
			MessageBox(0, "ERROR: Unrecognized orientation!", 0, 0);
			return true;
		}
	}
	else if (deviceType == "Vertical NIAWG")
	{
		if (Master->profile.getOrientation() == HORIZONTAL_ORIENTATION)
		{
			tempName += "DEFAULT_HCONFIG_VERTICAL_SCRIPT.nScript";
			//"DEFAULT_HORIZONTAL_SCRIPT.script"
		}
		else if (Master->profile.getOrientation() == VERTICAL_ORIENTATION)
		{
			tempName += "DEFAULT_VCONFIG_VERTICAL_SCRIPT.nScript";
		}
		else
		{
			MessageBox(0, "ERROR: Unrecognized orientation!", 0, 0);
			return true;
		}
	}
	else if (deviceType == "Agilent")
	{
		tempName += "DEFAULT_INTENSITY_SCRIPT.aScript";
	}
	else if (deviceType == "Master")
	{
		tempName += "DEFAULT_MASTER_SCRIPT.mScript";
	}
	this->reset();
	this->loadFile(tempName);
	this->colorEntireScript(Master);
	return false;
}
//
bool Script::openParentScript(std::string parentScriptFileAndPath, MasterWindow* Master)
{
	if (parentScriptFileAndPath == "")
	{
		return false;
	}
	char fileChars[_MAX_FNAME];
	char extChars[_MAX_EXT];
	char dirChars[_MAX_FNAME];
	char pathChars[_MAX_FNAME];
	int myError = _splitpath_s(parentScriptFileAndPath.c_str(), dirChars, _MAX_FNAME, pathChars, _MAX_FNAME, fileChars, _MAX_FNAME, extChars, _MAX_EXT);
	std::string extStr(extChars);
	if (deviceType == "Horizontal NIAWG" || deviceType == "Vertical NIAWG")
	{
		if (extStr != NIAWG_SCRIPT_EXTENSION)
		{
			MessageBox(0, "ERROR: Attempted to open non-NIAWG script inside NIAWG script control.", 0, 0);
			return false;
		}
	}
	else if (deviceType == "Agilent")
	{
		if (extStr != AGILENT_SCRIPT_EXTENSION)
		{
			MessageBox(0, "ERROR: Attempted to open non-agilent script from agilent script control.", 0, 0);
			return false;
		}
	}
	else if (deviceType == "Master")
	{
		if (extStr != MASTER_SCRIPT_EXTENSION)
		{
			errBox("ERROR: Attempted to open non-master script from master script control!");
			return false;
		}
	}
	else
	{
		MessageBox(0, "ERROR: Unrecognized device type inside script control! Ask Mark about Bugs.", 0, 0);
		return false;
	}
	if (!this->loadFile(parentScriptFileAndPath))
	{
		return false;
	}
	this->scriptName = std::string(fileChars);
	this->scriptFullAddress = parentScriptFileAndPath;
	this->updateSavedStatus(true);
	// Check location of vertical script.
	int position = parentScriptFileAndPath.find_last_of('\\');
	std::string scriptLocation = parentScriptFileAndPath.substr(0, position);
	if (scriptLocation + "\\" != (Master->profile.getCurrentPathIncludingCategory()) && Master->profile.getCurrentPathIncludingCategory() != "")
	{
		int answer = MessageBox(0, "The requested script is not currently located in the current configuration folder. This is recommended so that scripts "
			"related to a particular configuration are reserved to that configuration folder. Copy script to current configuration folder?", 0, MB_YESNO);
		if (answer == IDYES)
		{
			std::string scriptName = parentScriptFileAndPath.substr(position, parentScriptFileAndPath.size());
			std::string path = (Master->profile.getCurrentPathIncludingCategory()) + scriptName;
			this->saveScriptAs(path, Master);
			//fileManage::saveScript(relevantEdit, filePathway, savedInd, savedVar);
		}
	}
	this->updateScriptNameText(parentScriptFileAndPath);
	this->colorEntireScript(Master);
	return true;
}

/*
]---	This function only puts the given file on the edit for this class, it doesn't change current settings parameters. It's used bare when just changing the
]-		view of the edit, while it's used with some surrounding changes for loading a new parent.
 */
bool Script::loadFile(std::string pathToFile)
{
	std::fstream openFile;
	openFile.open(pathToFile.c_str(), std::ios::in);
	if (!openFile.is_open())
	{
		MessageBox(0, ("ERROR: Failed to open script file: " + pathToFile + ".").c_str(), 0, 0);
		this->reset();
		return false;
	}
	std::string tempLine;
	std::string fileText;
	while (std::getline(openFile, tempLine))
	{
		cleanString(tempLine);

		fileText += tempLine;
		// Append the line to the edit control here (use c_str() ).
	}
	// put the default into the new control.
	this->edit.SetWindowTextA(fileText.c_str());
	openFile.close();
	return true;
}

bool Script::reset()
{
	this->scriptName = "";
	this->scriptPath = "";
	this->scriptFullAddress = "";
	this->updateSavedStatus(false);
	this->fileNameText.SetWindowTextA("");
	this->edit.SetWindowTextA("");
	return false;
}

bool Script::savedStatus()
{
	return isSaved;
}

std::string Script::getScriptPathAndName()
{
	return this->scriptFullAddress;
}

std::string Script::getScriptName()
{
	return this->scriptName;
}

bool Script::considerCurrentLocation(MasterWindow* Master)
{
	if (this->scriptFullAddress.size() > 0)
	{
		// Check location of vertical script.
		int position = this->scriptFullAddress.find_last_of('\\');
		std::string scriptLocation = this->scriptFullAddress.substr(0, position);
		if (scriptLocation + "\\" != Master->profile.getCurrentPathIncludingCategory())
		{
			int answer = MessageBox(0, "The requested vertical script is not currently located in the current configuration folder. This is recommended so that scripts related to a"
				" particular configuration are reserved to that configuration folder. Copy script to current configuration folder?", 0, MB_YESNO);
			if (answer == IDYES)
			{
				std::string scriptName = this->scriptFullAddress.substr(position, this->scriptFullAddress.size());
				this->scriptFullAddress = Master->profile.getCurrentPathIncludingCategory() + scriptName;
				this->scriptPath = Master->profile.getCurrentPathIncludingCategory();
				this->saveScriptAs(this->scriptFullAddress, Master);
			}
		}
		// else nothing
	}
	return false;
}

std::string Script::getExtension()
{
	return extension;
}

bool Script::updateScriptNameText(std::string path)
{
	//std::string categoryPath = eProfile.getCurrentPathIncludingCategory();
	int position = path.find_last_of('\\');
	if (position != -1)
	{
		std::string categoryPath = path.substr(0, position);
		std::string name = path.substr(position + 1, path.size());
		position = categoryPath.find_last_of('\\');
		std::string category = categoryPath.substr(position + 1, categoryPath.size());
		std::string text = category + "->" + name;
		this->fileNameText.SetWindowTextA(text.c_str());
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
		this->fileNameText.SetWindowTextA(text.c_str());
	}
	return false;
}

INT_PTR Script::handleColorMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, std::unordered_map<std::string, HBRUSH> brushes)
{
	DWORD controlID = GetDlgCtrlID((HWND)lParam);
	HDC hdcStatic = (HDC)wParam;
	if (controlID == this->childCombo.ID || controlID == this->edit.ID)
	{
		SetTextColor(hdcStatic, RGB(255, 255, 255));
		SetBkColor(hdcStatic, RGB(15, 15, 15));
		return (LRESULT)brushes["Dark Grey"];
	}
	else if (controlID == this->title.ID || controlID == this->savedIndicator.ID || controlID == this->fileNameText.ID || controlID == this->availableFunctionsCombo.ID)
	{
		SetTextColor(hdcStatic, RGB(218, 165, 32));
		SetBkColor(hdcStatic, RGB(25, 25, 25));
		return (LRESULT)brushes["Medium Grey"];
	}
	else
	{
		return NULL;
	}
}

bool Script::saveAsFunction()
{
	// check to make sure that the current script is defined like a function
	CString text;
	this->edit.GetWindowTextA(text);
	std::stringstream stream;
	stream << text.GetBuffer();
	ExperimentManager::eatComments(&stream);
	std::string word;
	stream >> word;
	if (word != "def")
	{
		errBox("ERROR: Function declarations must begin with \"def\".");
		return false;
	}
	std::string line;
	std::getline(stream, line, '\r');
	int pos = line.find_first_of("(");
	if (pos == std::string::npos)
	{
		errBox("No \"(\" found in function name. If there are no arguments, use empty parenthesis \"()\"");
	}
	int initNamePos = line.find_first_not_of(" \t");
	std::string functionName = line.substr(initNamePos, line.find_first_of("(") - 1);
	if (functionName.find_first_of(" ") != std::string::npos)
	{
		errBox("ERROR: Function name included a space! Name was" + functionName);
		return false;
	}
	std::string path = FUNCTIONS_FOLDER_LOCATION + functionName + FUNCTION_EXTENSION;
	struct stat buffer;
	if (stat(path.c_str(), &buffer) == 0)
	{
		int answer = MessageBox(0, ("The function \"" + functionName + "\" already exists! Overwrite it?").c_str(), 0, MB_YESNO);
		if (answer == IDNO)
		{
			return false;
		}
	}
	std::fstream functionFile(path, std::ios::out);
	if (!functionFile.is_open())
	{
		errBox("ERROR: the function file failed to open!");
		return false;
	}
	functionFile << text.GetBuffer();
	functionFile.close();
	// refresh this.
	this->loadFunctions();
	// test if script exists in nearby folder.
	return true;
}

bool Script::loadFunctions()
{
	// scan the function home for functions.
	// Re-add the entries back in and figure out which one is the current one.
	std::vector<std::string> names;
	std::string search_path = this->functionLocation + "\\*.func";
	WIN32_FIND_DATA fd;
	HANDLE hFind;
	hFind = FindFirstFile(search_path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				names.push_back(fd.cFileName);
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);
	}

	// open each file and get it's function argument.
	std::fstream functionFile;
	std::vector<std::string> finalNames;
	for (int functionInc = 0; functionInc < names.size(); functionInc++)
	{
		if (functionFile.is_open())
		{
			functionFile.close();
		}
		functionFile.open(this->functionLocation + "\\" + names[functionInc], std::ios::in);
		if (!functionFile.is_open())
		{
			MessageBox(0, ("ERROR: Failed to open function file: " + names[functionInc]).c_str(), 0, 0);
			continue;
		}
		std::stringstream functionStream;
		functionStream << functionFile.rdbuf();
		ExperimentManager::eatComments(&functionStream);
		std::string functionDeclaration;
		std::getline(functionStream, functionDeclaration, ':');
		std::string name; 
		std::vector<std::string> args;
		if (ExperimentManager::analyzeFunctionDefinition(functionDeclaration, name, args))
		{
			// make the name for the combo.
			name += "(";
			if (args.size() != 0)
			{
				name += args[0];
			}
			for (int argInc = 1; argInc < args.size(); argInc++)
			{
				name += ", " + args[argInc];
			}
			name += ")";
			finalNames.push_back(name);
		}
	}
	// clear the box.
	this->availableFunctionsCombo.ResetContent();
	// 
	this->availableFunctionsCombo.AddString("Parent Script");
	for (int nameInc = 0; nameInc < finalNames.size(); nameInc++)
	{
		this->availableFunctionsCombo.AddString(finalNames[nameInc].c_str());
	}
	return true;
}