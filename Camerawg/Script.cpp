#include "stdafx.h"
#include "Script.h"
#include "constants.h"
#include "fonts.h"
#include <sstream>
#include <algorithm>
#include "boost/lexical_cast.hpp"
#include "cleanString.h"
#include "textPromptDialogProcedure.h"

Script::Script(){}
 
Script::~Script(){}

std::string Script::getScriptText()
{
	return false;
}

std::string Script::getSyntaxColor(std::string word, std::string editType, std::vector<variable> vars)
{
	// check special cases
	if (word.size() == 0)
	{
		return "-1";
	}
	else if (word[0] == '%')
	{
		if (word.size() > 1)
		{
			if (word[1] == '%')
			{
				return "comment2";
			}
		}
		return "comment1";
	}

	// Check NIAWG-specific commands
	if (editType == "Horizontal NIAWG" || editType == "Vertical NIAWG")
	{
		if (word == "gen" || word == "1," || word == "2," || word == "3," || word == "4," || word == "5," || word == "freq" || word == "amp" || word == "const"
			|| word == "&" || word == "ramp")
		{
			return "command";
		}
		// check logic
		if (word == "repeat" || word == "until" || word == "trigger" || word == "end" || word == "forever")
		{
			return "logic";
		}
		// check options
		if (word == "lin" || word == "nr" || word == "tanh")
		{
			return "option";
		}
		// check variable

		if (word == "predefined" || word == "script")
		{
			return "script";
		}
		if (word.size() > 8)
		{
			if (word.substr(word.size() - 8, 8) == ".nScript")
			{
				return "script file";
			}
		}
	}
	// check Agilent-specific commands
	else if (editType == "Agilent")
	{
		std::transform(word.begin(), word.end(), word.begin(), ::tolower);
		if (word == "agilent" || word == "ramp" || word == "hold" || word == "predefined" || word == "script")
		{
			return "command";
		}
		if (word == "once" || word == "oncewaittrig" || word == "lin" || word == "tanh" || word == "repeatuntiltrig")
		{
			return "option";
		}
		if (word.size() > 8)
		{
			if (word.substr(word.size() - 8, 8) == ".aScript")
			{
				return "script file";
			}
		}
	}
	for (int varInc = 0; varInc < vars.size(); varInc++)
	{
		if (word == vars[varInc].name)
		{
			return "variable";
		}
	}
	// check delimiter
	if (word == "#")
	{
		return "delimiter";
	}
	// see if it's a double.
	try
	{
		boost::lexical_cast<double>(word);
		return "number";
	}
	catch (boost::bad_lexical_cast &)
	{
		return "unrecognized";
	}
}

bool Script::updateSavedStatus(bool scriptIsSaved)
{
	this->isSaved = scriptIsSaved;
	if (scriptIsSaved)
	{
		savedIndicator.SetCheck(BST_CHECKED);
	}
	else
	{
		savedIndicator.SetCheck(BST_UNCHECKED);
	}
	return false;
}

bool Script::coloringIsNeeded()
{
	return !syntaxColoringIsCurrent;
}

bool Script::handleTimerCall(profileSettings profile, std::vector<variable> vars)
{
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
		CHARRANGE range;
		edit.GetSel(range);
		initScrollPos = edit.GetScrollPos(SB_VERT);
		// color syntax
		this->colorScriptSection(editChangeBegin, editChangeEnd, profile, vars);
		editChangeEnd = 0;
		editChangeBegin = ULONG_MAX;
		syntaxColoringIsCurrent = true;
		edit.SetSel(range);
		finScrollPos = edit.GetScrollPos(SB_VERT);
		edit.LineScroll(-(finScrollPos - initScrollPos));
		this->updateSavedStatus(tempSaved);
	}
	return false;
}

bool Script::handleEditChange()
{
	CHARRANGE range;
	edit.GetSel(range);
	if (range.cpMin < editChangeBegin)
	{
		editChangeBegin = range.cpMin;
	}
	if (range.cpMax > editChangeEnd)
	{
		editChangeEnd = range.cpMax;
	}
	syntaxColoringIsCurrent = false;
	updateSavedStatus(false);
	return TRUE;
}

bool Script::colorEntireScript(profileSettings profile, std::vector<variable> vars)
{
	return this->colorScriptSection(0, ULONG_MAX, profile, vars);
}

bool Script::colorScriptSection(DWORD beginingOfChange, DWORD endOfChange, profileSettings profile, std::vector<variable> vars)
{
	long long beginingSigned = beginingOfChange;
	long long endSigned = endOfChange;
	
	int scriptLength = edit.GetTextLength(); 
	CString buffer;
	edit.GetWindowText(buffer);
	std::string script(buffer);
	std::vector<std::string> predefinedScripts;
	std::string coloring;
	std::string word;
	std::stringstream fileTextStream(script);
	std::string line;
	std::string currentTextToAdd;
	std::string analysisWord;
	std::string tempColor;
	CHARFORMAT syntaxFormat;
	memset(&syntaxFormat, 0, sizeof(CHARFORMAT));
	syntaxFormat.cbSize = sizeof(CHARFORMAT);
	syntaxFormat.dwMask = CFM_COLOR;
	int relevantID = edit.GetDlgCtrlID();
	std::size_t prev, pos;
	CHARRANGE range = {0, 0};
	while (std::getline(fileTextStream, line))
	{
		DWORD lineStartCoordingate = range.cpMin;
		int endTest = range.cpMax + line.size();
		if (endTest < beginingSigned - 5 || range.cpMin > endSigned)
		{
			// then skip to next line.
			range.cpMax = endTest;
			range.cpMin = range.cpMax;
			continue;
		}
		prev = 0;
		coloring = "";
		while ((pos = line.find_first_of(" \t\r\n", prev)) != std::string::npos)
		{
			range.cpMax = lineStartCoordingate + pos;
			word = line.substr(prev, pos - prev + 1);
			// kill whatever is on the end of it.
			analysisWord = word.substr(0, word.length() - 1);
			// if comment is found, the rest of the line is green.
			if (coloring != "comment1" && coloring != "comment2")
			{
				tempColor = this->getSyntaxColor(analysisWord, deviceType, vars);
				if (tempColor != coloring)
				{
					coloring = tempColor;
					if (coloring == "comment1")
					{
						syntaxFormat.crTextColor = RGB(34, 139, 34);
					}
					else if (coloring == "comment2")
					{
						syntaxFormat.crTextColor = RGB(107, 35, 35);
					}
					else if (coloring == "command")
					{
						syntaxFormat.crTextColor = RGB(100, 100, 205);
					}
					else if (coloring == "number")
					{
						syntaxFormat.crTextColor = RGB(255, 255, 255);
					}
					else if (coloring == "logic")
					{
						syntaxFormat.crTextColor = RGB(0, 255, 255);
					}
					else if (coloring == "delimiter")
					{
						syntaxFormat.crTextColor = RGB(100, 100, 100);
					}
					else if (coloring == "variable")
					{
						syntaxFormat.crTextColor = RGB(255, 215, 0);
					}
					else if (coloring == "option")
					{
						syntaxFormat.crTextColor = RGB(210, 180, 140);
					}
					else if (coloring == "script")
					{
						syntaxFormat.crTextColor = RGB(147, 112, 219);
					}
					else if (coloring == "script file")
					{
						syntaxFormat.crTextColor = RGB(147, 112, 219);
					}
					else if (coloring == "unrecognized")
					{
						syntaxFormat.crTextColor = RGB(255, 0, 0);
					}
					edit.SetSel(range);
					edit.SetSelectionCharFormat(syntaxFormat);
					range.cpMin = range.cpMax;
				}
			}
			edit.SetSel(range);
			edit.SetSelectionCharFormat(syntaxFormat);
			range.cpMax++;
			range.cpMin = range.cpMax;
			prev = pos + 1;
		}
		if (prev < std::string::npos)
		{
			word = line.substr(prev, std::string::npos);
			range.cpMax = lineStartCoordingate + line.length();
			tempColor = getSyntaxColor(word, deviceType, vars);
			if (coloring != "comment1" && coloring != "comment2")
			{
				coloring = tempColor;
				if (coloring == "comment1")
				{
					syntaxFormat.crTextColor = RGB(34, 139, 34);
				}
				else if (coloring == "comment2")
				{
					syntaxFormat.crTextColor = RGB(107, 35, 35);
				}
				else if (coloring == "command")
				{
					syntaxFormat.crTextColor = RGB(100, 100, 205);
				}
				else if (coloring == "number")
				{
					syntaxFormat.crTextColor = RGB(255, 255, 255);
				}
				else if (coloring == "logic")
				{
					syntaxFormat.crTextColor = RGB(0, 255, 255);
				}
				else if (coloring == "unrecognized")
				{
					syntaxFormat.crTextColor = RGB(255, 0, 0);
				}
				else if (coloring == "delimiter")
				{
					syntaxFormat.crTextColor = RGB(100, 100, 100);
				}
				else if (coloring == "variable")
				{
					syntaxFormat.crTextColor = RGB(255, 215, 0);
				}
				else if (coloring == "option")
				{
					syntaxFormat.crTextColor = RGB(210, 180, 140);
				}
				else if (coloring == "script")
				{
					syntaxFormat.crTextColor = RGB(147, 112, 219);
				}
				else if (coloring == "script file")
				{
					syntaxFormat.crTextColor = RGB(147, 112, 219);
				}
				edit.SetSel(range);
				edit.SetSelectionCharFormat(syntaxFormat);
				range.cpMin = range.cpMax;
			}
			edit.SetSel(range);
			edit.SetSelectionCharFormat(syntaxFormat);
			range.cpMin = range.cpMax;
		}
	}
	this->updateChildCombo(profile);
	return false;
}

bool Script::updateChildCombo(profileSettings profile)
{
	// check the current setting.
	int selection = childCombo.GetCurSel();
	TCHAR text[256];
	childCombo.GetLBText(selection, text);
	std::string textStr(text);
	if (textStr != "Parent Script")
	{
		// if not parent, don't update the child combo.
		return false;
	}

	// get script
	int scriptLength = edit.GetTextLength();
	CString buffer;
	edit.GetWindowText(buffer);
	// get the name that's currently set.

	selection = childCombo.GetCurSel();
	TCHAR name[256];
	childCombo.GetLBText(selection, name);
	std::string currentName(name);
	// I'll use this later to reset the combo.
	// reset the combo and name vector.
	childCombo.ResetContent();
	childrenNames.clear();
	std::string script(buffer);
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
			std::string predefinedScriptName;
			std::getline(scriptStream, predefinedScriptName);
			// remove trailing '\r'
			predefinedScriptName = predefinedScriptName.substr(0, predefinedScriptName.size() - 1);
			// test if script exists in nearby folder.
			struct stat buffer;
			std::string path = profile.categoryPath + predefinedScriptName;
			if (stat(path.c_str(), &buffer) == 0)
			{
				// add to combo normally.
				childCombo.AddString(predefinedScriptName.c_str());
				childrenNames.push_back(predefinedScriptName);
			}
			else
			{
				childCombo.AddString((predefinedScriptName + " (File Not Found!)").c_str());
			}
		}
	}
	// add the parent string message
	childCombo.AddString("Parent Script");
	// reset the child window.
	childCombo.SelectString(0, currentName.c_str());
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
		return (INT_PTR)eGreyRedBrush;
	}
	else if (controlID == title.ID)
	{
		SetTextColor(hdcStatic, RGB(255, 255, 255));
		SetBkMode(hdcStatic, TRANSPARENT);
		SetBkColor(hdcStatic, RGB(75, 0, 0));
		return (INT_PTR)eDarkRedBrush;
	}
	else if (controlID == savedIndicator.ID)
	{
		SetTextColor(hdcStatic, RGB(255, 255, 255));
		SetBkMode(hdcStatic, TRANSPARENT);
		SetBkColor(hdcStatic, RGB(50, 45, 45));
		return (INT_PTR)eGreyRedBrush;
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
		return (INT_PTR)eDullRedBrush;
	}
	else
	{
		return false;
	}
}

bool Script::initializeControls(int width, int height, POINT& startingLocation, CWnd* parent, std::string deviceTypeInput, 
	int& idStart, std::unordered_map<std::string, CFont*> fonts, std::vector<CToolTipCtrl*>& tooltips)
{
	LoadLibrary(TEXT("Msftedit.dll"));
	deviceType = deviceTypeInput;
	if (deviceTypeInput == "Horizontal NIAWG" || deviceTypeInput == "Vertical NIAWG")
	{
		extension = NIAWG_SCRIPT_EXTENSION;
	}
	else if (deviceTypeInput == "Agilent")
	{
		extension = AGILENT_SCRIPT_EXTENSION;
	}
	else
	{
		throw std::invalid_argument("FATAL: Device input type not recognized during construction of script control.");
	}
	isSaved = true;
	editChangeEnd = 0;
	editChangeBegin = ULONG_MAX;

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
	// 
	title.sPos = { startingLocation.x, startingLocation.y, startingLocation.x + width, startingLocation.y + 20 };
	title.ID = idStart++;
	title.Create(titleText.c_str(), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, title.sPos, parent, title.ID);
	title.SetFont(fonts["Heading Font"]);
	startingLocation.y += 20;
	//
	fileNameText.sPos = { startingLocation.x, startingLocation.y, startingLocation.x + width, startingLocation.y + 20 };
	fileNameText.ID = idStart++;
	fileNameText.Create(WS_CHILD | WS_VISIBLE | SS_ENDELLIPSIS | ES_READONLY, fileNameText.sPos, parent, fileNameText.ID);
	fileNameText.SetFont(fonts["Normal Font"]);
	startingLocation.y += 20;
	//
	savedIndicator.sPos = { startingLocation.x, startingLocation.y, startingLocation.x + 80, startingLocation.y + 20 };
	savedIndicator.ID = idStart++;
	savedIndicator.Create("Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT, savedIndicator.sPos, parent, savedIndicator.ID);
	savedIndicator.SetFont(fonts["Normal Font"]);
	savedIndicator.SetCheck(BST_CHECKED);
	isSaved = true;
	startingLocation.y += 20;
	//
	
	childCombo.sPos = { startingLocation.x, startingLocation.y, startingLocation.x + width, startingLocation.y + 800 };
	childCombo.ID = idStart++;
	if (childCombo.ID != IDC_HORIZONTAL_SCRIPT_COMBO && childCombo.ID != IDC_VERTICAL_SCRIPT_COMBO && childCombo.ID != IDC_AGILENT_SCRIPT_COMBO)
	{
		throw;
	}
	childCombo.Create(CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE, childCombo.sPos, parent, childCombo.ID);
	childCombo.SetFont(fonts["Normal Font"]);
	childCombo.AddString("Parent Script");
	childCombo.SetCurSel(0);
	startingLocation.y += 25;
	// Edit
	edit.sPos = { startingLocation.x, startingLocation.y, startingLocation.x + width, height};
	edit.ID = idStart++;
	if (edit.ID != IDC_HORIZONTAL_SCRIPT_EDIT && edit.ID != IDC_VERTICAL_SCRIPT_EDIT && edit.ID != IDC_AGILENT_SCRIPT_EDIT)
	{
		throw std::invalid_argument("FATAL: script edit id does not match known script edit ID's.");
	}
	edit.Create(WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | WS_HSCROLL 
				| WS_BORDER | ES_WANTRETURN, edit.sPos, parent, edit.ID);
	edit.SetFont(fonts["Code Font"]);
	edit.SetBackgroundColor(0, RGB(30, 25, 25));
	edit.SetEventMask(ENM_CHANGE);
	edit.SetDefaultCharFormat(myCharFormat);
	return false;
}

bool Script::rearrange()
{
	return false;
}

bool Script::childComboChangeHandler( MainWindow* mainWin)
{
	int selection = this->childCombo.GetCurSel();
	// prompt for save
	this->checkSave( mainWin->getCurentProfileSettings(), mainWin->niawgIsRunning() );
	TCHAR selectedText[256];
	childCombo.GetLBText(selection, selectedText);
	std::string viewName(selectedText);
	this->changeView(viewName, mainWin->getCurentProfileSettings(), mainWin->getAllVariables());
	this->updateSavedStatus(true);
	return false;
}

bool Script::changeView(std::string viewName, profileSettings profile, std::vector<variable> vars)
{
	if (viewName == "Parent Script")
	{
		// load parent
		this->loadFile(profile.categoryPath + scriptName + extension, profile, vars);
		this->currentViewIsParent = true;
		this->currentViewName = scriptName;
	}
	else
	{
		// load child
		this->loadFile(profile.categoryPath + viewName, profile, vars);
		this->currentViewIsParent = false;
		this->currentViewName = viewName;
	}
	return false;
}

// can save either parent or child depending on input.
bool Script::saveScript(profileSettings profile, bool saveParent, bool niawgIsRunning)
{
	std::string relevantName;
	if (saveParent)
	{
		relevantName = scriptName;
	}
	else
	{
		relevantName = currentViewName;
	}
	if (relevantName == "")
	{
		// must give new name. This should only work if the experiment and category have been set. 
		if (profile.experiment == "")
		{
			errBox("The script is unnamed, and no experiment has been set. Please select an experiment and category or select \"Save As\" to save in an arbitrary location.");
			return true;
		}
		if (profile.category == "")
		{
			errBox("The script is unnamed, and no Category has been set. Please select a category or select \"Save As\" to save in an arbitrary location.");
			return true;
		}
		std::string newName = (const char*)DialogBoxParam(eGlobalInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure,
			(LPARAM)("The " + deviceType + " script is unnamed! Please enter new name for the script, and the script will be saved in the current location: " + profile.experiment + " -> " + profile.category).c_str());
		if (newName == "")
		{
			// canceled
			return true;
		}
		std::string path = profile.categoryPath + newName + extension;
		this->saveScriptAs(path, saveParent);
		// continue and resave anyways.
	}

	if (niawgIsRunning)
	{
		if (scriptName == eMostRecentHorizontalScriptNames || scriptName == eMostRecentIntensityScriptNames || scriptName == eMostRecentVerticalScriptNames)
		{
			MessageBox(0, "ERROR: System is currently running. You can't save over any files in use by the system while it runs, which includes the "
				"horizontal and vertical AOM scripts and the intensity script.", 0, 0);
			return true;
		}
	}


	int textLength = edit.GetWindowTextLength();
	CString editText;
	edit.GetWindowText(editText);
	std::fstream saveFile;
	int extPos = relevantName.find_last_of(".");
	if (extPos != -1)
	{
		// the scriptname already has an extension...
		std::string existingExtension = relevantName.substr(extPos);
		std::string nameNoExtension = relevantName.substr(0, extPos);
		if (existingExtension != this->extension)
		{
			errBox("ERROR: The " + this->deviceType + " scriptName (as understood by the code) already has an "
				"extension, read by the code as " + existingExtension + ", and that extension doesn't match the extension for this device! Script name is: " +
				relevantName + " While the proper extension is " + this->extension + ". The file will be saved as " +
				nameNoExtension + extension);
			std::string path = profile.categoryPath + nameNoExtension + extension;
			this->saveScriptAs(path, saveParent);
			return false;
		}
		else
		{
			// take the extension off of the script name. That's no good.
			if (saveParent)
			{
				this->scriptName = nameNoExtension;
			}
			else
			{
				this->currentViewName = nameNoExtension;
			}
			saveFile.open(profile.categoryPath + nameNoExtension + extension, std::fstream::out);
		}
	}
	else
	{
		// In theory the code should always do this line, not the above check.
		saveFile.open(profile.categoryPath + relevantName + extension, std::fstream::out);
	}
	if (!saveFile.is_open())
	{
		MessageBox(0, ("ERROR: Failed to open script file: " + profile.categoryPath + scriptName + extension).c_str(), 0, 0);
		return true;
	}
	saveFile << editText;
	saveFile.close();
	// for good measure.
	this->scriptAddress = profile.categoryPath + scriptName + extension;
	this->scriptCategory = profile.category;
	this->scriptExperiment = profile.experiment;
	this->updateSavedStatus(true);
	return false;
}
// can save either parent or child depending on input.
bool Script::saveScript(profileSettings profile, bool niawgIsRunning)
{
	return this->saveScript(profile, this->currentViewIsParent, niawgIsRunning);
}

//
bool Script::saveScriptAs(std::string location, bool saveParent, bool niawgIsRunning)
{
	if (location == "")
	{
		return true;
	}
	if (niawgIsRunning)
	{
		if (location == eMostRecentHorizontalScriptNames || location == eMostRecentIntensityScriptNames || location == eMostRecentVerticalScriptNames)
		{
			MessageBox(0, "ERROR: System is currently running. You can't save over any files in use by the system while it runs, which includes the "
				"horizontal and vertical AOM scripts and the intensity script.", 0, 0);
			return true;
		}
	}
	
	int textLength = edit.GetWindowTextLength();
	CString editText;
	edit.GetWindowText(editText);
	std::fstream saveFile(location, std::fstream::out);
	if (!saveFile.is_open())
	{
		MessageBox(0, ("ERROR: Failed to open script file: " + location).c_str(), 0, 0);
		return true;
	}
	saveFile << editText;
	// this location should have the script name, the script category, and the script experiment location in it.
	this->scriptAddress = location;
	int index = location.find_last_of("\\");
	if (saveParent)
	{
		scriptName = location.substr(index + 1, location.size());
	}
	else
	{
		currentViewName = location.substr(index + 1, location.size());
	}
	location = location.substr(0, index);
	index = location.find_last_of("\\");
	scriptCategory = location.substr(index + 1, location.size());
	location = location.substr(0, index);
	index = location.find_last_of("\\");
	scriptExperiment = location.substr(index + 1, location.size());
	saveFile.close();
	this->updateSavedStatus(true);
	return false;
}

// 
bool Script::saveScriptAs(std::string location, bool niawgIsRunning )
{
	return this->saveScriptAs(location, this->currentViewIsParent, niawgIsRunning);
}


bool Script::checkChildSave(profileSettings profile)
{
	// get the 
	if (isSaved)
	{
		// don't need to do anything
		return false;
	}
	int selection = childCombo.GetCurSel();
	TCHAR name[256];
	childCombo.GetLBText(selection, name);
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
			std::string newName = (const char*)DialogBoxParam(eGlobalInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure,
				(LPARAM)("Please enter new name for the script " + scriptName + ".").c_str());
			std::string path = profile.categoryPath + newName + this->extension;
			this->saveScriptAs(path, false);
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
			this->saveScriptAs(nameStr, false);
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
bool Script::checkSave(profileSettings profile, bool niawgIsRunning )
{
	if (isSaved)
	{
		// don't need to do anything
		return false;
	}
	if (currentViewName == "")
	{
		int answer = MessageBox(0, ("Current " + deviceType + " script file is unsaved and unnamed. Save it with a with new name?").c_str(), 0, MB_YESNOCANCEL);
		if (answer == IDCANCEL)
		{
			// function calling might want to watch for true.
			return true;
		}
		else if (answer == IDNO)
		{
			return false;
		}
		else if (answer == IDYES)
		{
			std::string newName = (const char*)DialogBoxParam(eGlobalInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure,
				(LPARAM)("Please enter new name for the script " + scriptName + ".").c_str());
			std::string path = profile.categoryPath + newName + this->extension;
			this->saveScriptAs(path, niawgIsRunning );
			return false;
		}
		else
		{
			MessageBox(0, "WTF ERROR", 0, 0);
			return true;
		}
	}
	else
	{
		int answer = MessageBox(0, ("Save " + deviceType + " script file as " + currentViewName + "?").c_str(), 0, MB_YESNOCANCEL);
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
			this->saveScript(profile, niawgIsRunning );
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
bool Script::renameScript(profileSettings profile)
{
	if (this->scriptName == "")
	{
		return false;
	}
	std::string newName = (const char*)DialogBoxParam(eGlobalInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure, 
		(LPARAM)("Please enter new name for the script " + scriptName + ".").c_str());
	if (newName == "")
	{
		// canceled
		return false;
	}
	int result = MoveFile((profile.categoryPath + scriptName + extension).c_str(),
		(profile.categoryPath + newName + extension).c_str());
	if (result == 0)
	{
		MessageBox(0, "ERROR: Failed to move file.", 0, 0);
		return true;
	}
	this->scriptAddress = profile.categoryPath + scriptName + extension;
	this->scriptCategory = profile.category;
	this->scriptExperiment = profile.experiment;
	return false;
}
//
bool Script::deleteScript(profileSettings profile)
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

	int result = DeleteFile((profile.categoryPath + scriptName + extension).c_str());
	if (result == 0)
	{
		MessageBox(0, "ERROR: Deleting script file failed!", 0, 0);
		return true;
	}
	else
	{
		this->scriptName = "";
		this->currentViewName = scriptName;
		this->scriptAddress = "";
		this->scriptCategory = "";
		this->scriptExperiment = "";
	}
	return false;
}
//
bool Script::newScript(profileSettings profile, std::vector<variable> vars)
{
	std::string tempName;
	tempName = DEFAULT_SCRIPT_FOLDER_PATH;
	if (deviceType == "Horizontal NIAWG")
	{
		if (profile.orientation == HORIZONTAL_ORIENTATION)
		{
			tempName += "DEFAULT_HCONFIG_HORIZONTAL_SCRIPT.nScript";
			//"DEFAULT_HORIZONTAL_SCRIPT.script"
		}
		else if (profile.orientation == VERTICAL_ORIENTATION)
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
		if (profile.orientation == HORIZONTAL_ORIENTATION)
		{
			tempName += "DEFAULT_HCONFIG_VERTICAL_SCRIPT.nScript";
			//"DEFAULT_HORIZONTAL_SCRIPT.script"
		}
		else if (profile.orientation == VERTICAL_ORIENTATION)
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
	this->reset();
	this->loadFile(tempName, profile, vars);
	// add the current category to the address. 
	this->scriptAddress = profile.categoryPath;
	this->scriptCategory = profile.category;
	this->scriptExperiment = profile.experiment;
	return false;
}
//
bool Script::openParentScript(std::string parentScriptFileAndPath, profileSettings profile, std::vector<variable> vars)
{
	if (parentScriptFileAndPath == "")
	{
		return false;
	}
	std::string location = parentScriptFileAndPath;
	this->scriptAddress = location;
	int index;
	index = location.find_last_of(".");
	// includes the .
	std::string extStr = location.substr(index, location.size());
	location = location.substr(0, index);
	index = location.find_last_of("\\");
	scriptName = location.substr(index + 1, location.size());
	this->currentViewName = scriptName;
	location = location.substr(0, index);
	index = location.find_last_of("\\");
	scriptCategory = location.substr(index + 1, location.size());
	location = location.substr(0, index);
	index = location.find_last_of("\\");
	scriptExperiment = location.substr(index + 1, location.size());
	if (deviceType == "Horizontal NIAWG" || deviceType == "Vertical NIAWG")
	{
		if (extStr != NIAWG_SCRIPT_EXTENSION)
		{
			MessageBox(0, "ERROR: Attempted to open non-NIAWG script inside NIAWG script control.", 0, 0);
			return true;
		}
	}
	else if (deviceType == "Agilent")
	{
		if (extStr != AGILENT_SCRIPT_EXTENSION)
		{
			MessageBox(0, "ERROR: Attempted to open non-agilent script from agilent script control.", 0, 0);
			return true;
		}
	}
	else
	{
		MessageBox(0, "ERROR: Unrecognized device type inside script control! Ask Mark about Bugs.", 0, 0);
		return true;
	}
	if (!this->loadFile(parentScriptFileAndPath, profile, vars))
	{
		return true;
	}
	// Check the location of the script.
	index = parentScriptFileAndPath.find_last_of("\\");
	std::string scriptLocation = parentScriptFileAndPath.substr(0, index);
	if (scriptLocation + "\\" != profile.categoryPath && profile.categoryPath != "")
	{
		int answer = MessageBox( 0, ("The requested script " + scriptName + " at " + scriptLocation + " is not currently located in the "
									  "current configuration folder (" + profile.categoryPath + ". This is recommended so that scripts "
									  "related to a particular configuration are reserved to that category folder. Copy script to current "
									  "category folder?").c_str(), 0, MB_YESNO );
		if (answer == IDYES)
		{
			std::string location = profile.categoryPath + scriptName;
			std::string path = location;
			scriptAddress = location;
			int index = location.find_last_of("\\");
			scriptName = location.substr(index + 1, location.size());
			currentViewName = scriptName;
			location = location.substr(0, index);
			index = location.find_last_of("\\");
			scriptCategory = location.substr(index + 1, location.size());
			location = location.substr(0, index);
			index = location.find_last_of("\\");
			scriptExperiment = location.substr(index + 1, location.size());
			scriptName = parentScriptFileAndPath.substr(index + 1, parentScriptFileAndPath.size());
			currentViewName = scriptName;
			scriptAddress = profile.categoryPath + scriptName;
			scriptCategory = profile.category;
			scriptExperiment = profile.experiment;
			path = profile.categoryPath + scriptName;
			saveScriptAs(path, true);
		}
	}
	updateScriptNameText();
	colorEntireScript(profile, vars);
	updateSavedStatus(true);
	return false;
}

/*
]---	This function only puts the given file on the edit for this class, it doesn't change current settings parameters. It's used bare when just changing the
]-		view of the edit, while it's used with some surrounding changes for loading a new parent.
 */
bool Script::loadFile(std::string pathToFile, profileSettings profile, std::vector<variable> vars)
{
	std::ifstream openFile(pathToFile.c_str());
	if (!openFile.is_open())
	{
		MessageBox(0, ("ERROR: Failed to open script file: " + pathToFile + ".").c_str(), 0, 0);
		reset();
		return false;
	}
	std::string tempLine;
	std::string fileText;
	while (std::getline(openFile, tempLine))
	{
		cleanString(tempLine);
		fileText += tempLine;
	}
	// put the default into the new control.
	edit.SetWindowText(fileText.c_str());
	this->colorEntireScript(profile, vars);
	this->updateScriptNameText();
	openFile.close();
	return true;
}

bool Script::reset()
{
	this->scriptName = "";
	this->currentViewName = scriptName;
	this->currentViewIsParent = true;
	this->scriptAddress = "";
	this->scriptCategory = "";
	this->scriptExperiment = "";
	this->updateSavedStatus(false);
	fileNameText.SetWindowText("");
	edit.SetWindowText("");
	return false;
}

bool Script::savedStatus()
{
	return isSaved;
}

std::string Script::getScriptAddress()
{
	return this->scriptAddress;
}

std::string Script::getScriptName()
{
	return this->scriptName;
}

bool Script::considerCurrentLocation(profileSettings profile)
{
	if (this->scriptAddress.size() > 0)
	{
		// Check location of vertical script.
		int index = this->scriptAddress.find_last_of('\\');
		std::string scriptLocation = this->scriptAddress.substr(0, index);
		if (scriptLocation + "\\" != profile.categoryPath)
		{
			int answer = MessageBox(0, ("The requested script " + scriptName + " at " + scriptLocation + " is not currently located in the current configuration "
				"folder (" + profile.categoryPath + ". This is recommended so that scripts related to a particular configuration are "
				"reserved to that category folder. Copy script to current category folder?").c_str(), 0, MB_YESNO);
			if (answer == IDYES)
			{
				// grab the correct file name at the end.
				this->scriptName = this->scriptAddress.substr(index, this->scriptAddress.size());
				this->currentViewName = scriptName;
				// this name includes the extension already.
				this->scriptAddress = profile.categoryPath + scriptName;
				this->scriptCategory = profile.category;
				this->scriptExperiment = profile.experiment;
				this->saveScriptAs(this->scriptAddress, true);
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

// updates based on the address in the script object. Relies on the script object correctly knowing it's own address and info.
bool Script::updateScriptNameText()
{
	std::string text;
	// add experiment to text.
	if (this->scriptExperiment == "")
	{
		text = "???";
	}
	else
	{
		text = this->scriptExperiment;
	}
	text += " -> ";
	// add category to text.
	if (this->scriptCategory == "")
	{
		text += "???";
	}
	else
	{
		text += this->scriptCategory;
	}
	text += " -> ";
	// add name to text.
	if (this->scriptName == "")
	{
		text += "???";
	}
	else
	{
		text += this->scriptName;
	}
	// set text.
	fileNameText.SetWindowText(text.c_str());
	return false;
}

void Script::checkExtension(profileSettings profile)
{
	// check the view name
	int extPos = this->currentViewName.find_last_of(".");
	if (extPos != -1)
	{
		// the scriptname already has an extension...
		std::string existingExtension = this->currentViewName.substr(extPos);
		std::string nameNoExtension = this->currentViewName.substr(0, extPos);
		if (existingExtension != this->extension)
		{
			errBox("ERROR: The " + this->deviceType + " scriptName (as understood by the code) already has an "
				"extension, read by the code as " + existingExtension + ", and that extension doesn't match the extension for this device! Script name is: " +
				currentViewName + " While the proper extension is " + this->extension + ". The file will be saved as " +
				nameNoExtension + extension);
			std::string path = profile.categoryPath + nameNoExtension + extension;
			this->saveScriptAs(path, false);
			return;
		}
		else
		{
			// take the extension off of the script name. That's no good. 
			this->currentViewName = nameNoExtension;
		}
	}

	// check the view name
	extPos = this->scriptName.find_last_of(".");
	if (extPos != -1)
	{
		// the scriptname already has an extension...
		std::string existingExtension = this->scriptName.substr(extPos);
		std::string nameNoExtension = this->scriptName.substr(0, extPos);
		if (existingExtension != this->extension)
		{
			errBox("ERROR: The " + this->deviceType + " scriptName (as understood by the code) already has an "
				"extension, read by the code as " + existingExtension + ", and that extension doesn't match the extension for this device! Script name is: " +
				currentViewName + " While the proper extension is " + this->extension + ". The file will be saved as " +
				nameNoExtension + extension);
			std::string path = profile.categoryPath + nameNoExtension + extension;
			this->saveScriptAs(path, true);
			return;
		}
		else
		{
			// take the extension off of the script name. That's no good. 
			this->scriptName = nameNoExtension;
		}
	}

	return;
}
