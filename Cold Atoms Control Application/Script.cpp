#include "stdafx.h"
#include "Script.h"
#include "constants.h"
#include "fonts.h"
#include <sstream>
#include <algorithm>
#include "boost/lexical_cast.hpp"
#include "cleanString.h"
#include "textPromptDialogProcedure.h"

Script::Script(std::string deviceTypeInput, int& idStart) : deviceType{ deviceTypeInput }, editID{ idStart }, savedIndicatorID{ idStart + 1 },
															childComboID{ idStart + 2 }, fileNameTextID{ idStart + 3 }, titleID{ idStart + 4 }, 
															idStart{ idStart }, idEnd{ idStart + 4 }
{
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
		extension = "Sadness";
	}
	isSaved = true;
	editChangeEnd = 0;
	editChangeBegin = ULONG_MAX;
	idStart += 5;
}

Script::~Script(){}

std::string Script::getScriptText()
{
	return false;
}

std::string Script::getSyntaxColor(std::string word, std::string editType)
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
	for (int varInc = 0; varInc < eVariables.getCurrentNumberOfVariables(); varInc++)
	{
		if (word == eVariables.getVariableInfo(varInc).name)
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
		SendMessage(savedIndicator.hwnd, BM_SETCHECK, BST_CHECKED, NULL);
	}
	else
	{
		SendMessage(savedIndicator.hwnd, BM_SETCHECK, BST_UNCHECKED, NULL);
	}
	return false;
}

bool Script::coloringIsNeeded()
{
	return !syntaxColoringIsCurrent;
}

bool Script::handleTimerCall()
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
		SendMessage(edit.hwnd, EM_GETSEL, (WPARAM)&x1, (LPARAM)&x2);
		initScrollPos = GetScrollPos(edit.hwnd, SB_VERT);
		// color syntax
		this->colorScriptSection(editChangeBegin, editChangeEnd);
		editChangeEnd = 0;
		editChangeBegin = ULONG_MAX;
		syntaxColoringIsCurrent = true;
		SendMessage(edit.hwnd, EM_SETSEL, (WPARAM)x1, (LPARAM)x2);
		finScrollPos = GetScrollPos(edit.hwnd, SB_VERT);
		SendMessage(edit.hwnd, EM_LINESCROLL, 0, -(finScrollPos - initScrollPos));
		this->updateSavedStatus(tempSaved);
	}
	return false;
}

bool Script::handleEditChange(WPARAM wParam, LPARAM lParam)
{
	int controlID = GetDlgCtrlID((HWND)lParam);
	if (controlID == editID)
	{
		if (HIWORD(wParam) == EN_CHANGE)
		{
			//		int editChangeBegin;
			// int editChangeEnd;
			DWORD begin, end;
			SendMessage(edit.hwnd, EM_GETSEL, (WPARAM)&begin, (LPARAM)&end);
			if (begin < editChangeBegin)
			{
				editChangeBegin = begin;
			}
			if (end > editChangeEnd)
			{
				editChangeEnd = end;
			}
			syntaxColoringIsCurrent = false;
			this->updateSavedStatus(false);
			SetTimer(eScriptingWindowHandle, SYNTAX_TIMER_ID, SYNTAX_TIMER_LENGTH, (TIMERPROC)NULL);
		}
		return false;
	}
	else
	{
		return true;
	}
}

bool Script::colorEntireScript()
{
	return (this->colorScriptSection(0, ULONG_MAX));
}

bool Script::colorScriptSection(DWORD beginingOfChange, DWORD endOfChange)
{
	long long beginingSigned = beginingOfChange;
	long long endSigned = endOfChange;
	int scriptLength = SendMessage(this->edit.hwnd, WM_GETTEXTLENGTH, 0, 0);
	char* buffer = new char[scriptLength + 1];
	SendMessage(this->edit.hwnd, WM_GETTEXT, scriptLength + 1, (LPARAM)buffer);
	std::string script(buffer);
	delete buffer;
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
	int relevantID = GetDlgCtrlID(edit.hwnd);
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
		coloring = "";
		while ((pos = line.find_first_of(" \t\r\n", prev)) != std::string::npos)
		{
			end = lineStartCoordingate + pos;
			word = line.substr(prev, pos - prev + 1);
			// kill whatever is on the end of it.
			analysisWord = word.substr(0, word.length() - 1);
			// if comment is found, the rest of the line is green.
			if (coloring != "comment1" && coloring != "comment2")
			{
				tempColor = this->getSyntaxColor(analysisWord, deviceType);
				if (tempColor != coloring)
				{
					coloring = tempColor;
					if (coloring == "comment1")
					{
						syntaxFormat.crTextColor = RGB(34, 139, 34);
						SendMessage(edit.hwnd, EM_SETSEL, start, end);
						SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "comment2")
					{
						syntaxFormat.crTextColor = RGB(107, 35, 35);
						SendMessage(edit.hwnd, EM_SETSEL, start, end);
						SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "command")
					{
						syntaxFormat.crTextColor = RGB(100, 100, 205);
						SendMessage(edit.hwnd, EM_SETSEL, start, end);
						SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "number")
					{
						syntaxFormat.crTextColor = RGB(255, 255, 255);
						SendMessage(edit.hwnd, EM_SETSEL, start, end);
						SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "logic")
					{
						syntaxFormat.crTextColor = RGB(0, 255, 255);
						SendMessage(edit.hwnd, EM_SETSEL, start, end);
						SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "delimiter")
					{
						syntaxFormat.crTextColor = RGB(100, 100, 100);
						SendMessage(edit.hwnd, EM_SETSEL, start, end);
						SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "variable")
					{
						syntaxFormat.crTextColor = RGB(255, 215, 0);
						SendMessage(edit.hwnd, EM_SETSEL, start, end);
						SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "option")
					{
						syntaxFormat.crTextColor = RGB(210, 180, 140);
						SendMessage(edit.hwnd, EM_SETSEL, start, end);
						SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "script")
					{
						syntaxFormat.crTextColor = RGB(147, 112, 219);
						SendMessage(edit.hwnd, EM_SETSEL, start, end);
						SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "script file")
					{
						syntaxFormat.crTextColor = RGB(147, 112, 219);
						SendMessage(edit.hwnd, EM_SETSEL, lineStartCoordingate, end);
						SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
						/*
						if (isParent)
						{
							bool isNew = true;
							for (int predefinedInc = 0; predefinedInc < predefinedScripts.size(); predefinedInc++)
							{
								if (line.substr(0, line.size() - 8) == predefinedScripts[predefinedInc])
								{
									isNew = false;
									break;
								}
							}
							if (isNew)
							{
								predefinedScripts.push_back(line.substr(0, line.size() - 8));
								SendMessage(childCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)line.substr(0, line.size() - 8).c_str());
							}
						}
						*/
					}
					else if (coloring == "unrecognized")
					{
						syntaxFormat.crTextColor = RGB(255, 0, 0);
						SendMessage(edit.hwnd, EM_SETSEL, start, end);
						SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
				}
			}

			SendMessage(edit.hwnd, EM_SETSEL, start, end);
			SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
			end++;
			start = end;
			//SendMessage(edit.hwnd, EM_REPLACESEL, 0, (LPARAM)word.c_str());
			prev = pos + 1;
		}
		if (prev < std::string::npos)
		{
			word = line.substr(prev, std::string::npos);
			end = lineStartCoordingate + line.length();
			tempColor = getSyntaxColor(word, deviceType);
			if (coloring != "comment1" && coloring != "comment2")
			{
				coloring = tempColor;
				if (coloring == "comment1")
				{
					syntaxFormat.crTextColor = RGB(34, 139, 34);
					SendMessage(edit.hwnd, EM_SETSEL, start, end);
					SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
					start = end;
				}
				else if (coloring == "comment2")
				{
					syntaxFormat.crTextColor = RGB(107, 35, 35);
					SendMessage(edit.hwnd, EM_SETSEL, start, end);
					SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
					start = end;
				}
				else if (coloring == "command")
				{
					syntaxFormat.crTextColor = RGB(100, 100, 205);
					SendMessage(edit.hwnd, EM_SETSEL, start, end);
					SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
					start = end;
				}
				else if (coloring == "number")
				{
					syntaxFormat.crTextColor = RGB(255, 255, 255);
					SendMessage(edit.hwnd, EM_SETSEL, start, end);
					SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
					start = end;
				}
				else if (coloring == "logic")
				{
					syntaxFormat.crTextColor = RGB(0, 255, 255);
					SendMessage(edit.hwnd, EM_SETSEL, start, end);
					SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
					start = end;
				}
				else if (coloring == "unrecognized")
				{
					syntaxFormat.crTextColor = RGB(255, 0, 0);
					SendMessage(edit.hwnd, EM_SETSEL, start, end);
					SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
					start = end;
				}
				else if (coloring == "delimiter")
				{
					syntaxFormat.crTextColor = RGB(100, 100, 100);
					SendMessage(edit.hwnd, EM_SETSEL, start, end);
					SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
					start = end;
				}
				else if (coloring == "variable")
				{
					syntaxFormat.crTextColor = RGB(255, 215, 0);
					SendMessage(edit.hwnd, EM_SETSEL, start, end);
					SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
					start = end;
				}
				else if (coloring == "option")
				{
					syntaxFormat.crTextColor = RGB(210, 180, 140);
					SendMessage(edit.hwnd, EM_SETSEL, start, end);
					SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
					start = end;
				}
				else if (coloring == "script")
				{
					syntaxFormat.crTextColor = RGB(147, 112, 219);
					SendMessage(edit.hwnd, EM_SETSEL, lineStartCoordingate, end);
					SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
					start = end;
				}
				else if (coloring == "script file")
				{
					syntaxFormat.crTextColor = RGB(147, 112, 219);
					SendMessage(edit.hwnd, EM_SETSEL, lineStartCoordingate, end);
					SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
					start = end;
				}
			}
			SendMessage(edit.hwnd, EM_SETSEL, start, end);
			SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
			//end += 1;
			start = end;
		}
	}
	this->updateChildCombo();
	return false;
}

bool Script::updateChildCombo()
{
	// check the current setting.
	int selection = SendMessage(this->childCombo.hwnd, CB_GETCURSEL, 0, 0);
	TCHAR text[256];
	SendMessage(this->childCombo.hwnd, CB_GETLBTEXT, selection, (LPARAM)text);
	std::string textStr(text);
	if (textStr != "Parent Script")
	{
		// if not parent, don't update the child combo.
		return false;
	}

	// get script
	int scriptLength = SendMessage(this->edit.hwnd, WM_GETTEXTLENGTH, 0, 0);
	char* buffer = new char[scriptLength + 1];
	SendMessage(this->edit.hwnd, WM_GETTEXT, scriptLength + 1, (LPARAM)buffer);
	// get the name that's currently set.
	selection = SendMessage(this->childCombo.hwnd, CB_GETCURSEL, 0, 0);
	TCHAR name[256];
	SendMessage(this->childCombo.hwnd, CB_GETLBTEXT, selection, (LPARAM)name);
	std::string currentName(name);
	// I'll use this later to reset the combo.
	// reset the combo and name vector.
	SendMessage(this->childCombo.hwnd, CB_RESETCONTENT, 0, 0);

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
			std::string scriptName;
			std::getline(scriptStream, scriptName);
			// remove trailing '\r'
			scriptName = scriptName.substr(0, scriptName.size() - 1);
			// test if script exists in nearby folder.
			struct stat buffer;
			std::string path = eProfile.getCurrentPathIncludingCategory() + scriptName;
			if (stat(path.c_str(), &buffer) == 0)
			{
				// add to combo normally.
				SendMessage(this->childCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)scriptName.c_str());
				childrenNames.push_back(scriptName);
			}
			else
			{
				SendMessage(this->childCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)(scriptName + " (File Not Found!)").c_str());
			}
		}
	}
	// add the parent string message
	SendMessage(this->childCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)"Parent Script");
	// reset the child window.
	SendMessage(this->childCombo.hwnd, CB_SELECTSTRING, 0, (LPARAM)currentName.c_str());
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
	if (controlID == editID)
	{
		SetTextColor(hdcStatic, RGB(255, 255, 255));
		SetBkColor(hdcStatic, RGB(50, 45, 45));
		return (INT_PTR)eGreyRedBrush;
	}
	else if (controlID == titleID)
	{
		SetTextColor(hdcStatic, RGB(255, 255, 255));
		SetBkMode(hdcStatic, TRANSPARENT);
		SetBkColor(hdcStatic, RGB(75, 0, 0));
		return (INT_PTR)eDarkRedBrush;
	}
	else if (controlID == savedIndicatorID)
	{
		SetTextColor(hdcStatic, RGB(255, 255, 255));
		SetBkMode(hdcStatic, TRANSPARENT);
		SetBkColor(hdcStatic, RGB(50, 45, 45));
		return (INT_PTR)eGreyRedBrush;
	}
	else if (controlID == childComboID)
	{
		return false;
	}
	else if (controlID == fileNameTextID)
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

bool Script::initializeControls(int width, int height, POINT& startingLocation, HWND parent)
{
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
	RECT itemBox;
	//
	itemBox = title.position = { startingLocation.x, startingLocation.y, startingLocation.x + width, startingLocation.y + 20 };
	title.hwnd = CreateWindowEx(NULL, "STATIC", titleText.c_str(), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
		itemBox.left, itemBox.top, itemBox.right - itemBox.left, itemBox.bottom - itemBox.top,
		parent, (HMENU)titleID, GetModuleHandle(NULL), NULL);
	SendMessage(eStaticVerticalEditHandle, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	startingLocation.y += 20;
	//
	itemBox = fileNameText.position = { startingLocation.x, startingLocation.y, startingLocation.x + width, startingLocation.y + 20 };
	fileNameText.hwnd = CreateWindowEx(NULL, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_ENDELLIPSIS,
		itemBox.left, itemBox.top, itemBox.right - itemBox.left, itemBox.bottom - itemBox.top, 
		parent, (HMENU)fileNameTextID, GetModuleHandle(NULL), NULL);
	SendMessage(eStaticVerticalEditHandle, WM_SETFONT, WPARAM(sHeadingFont), TRUE);
	startingLocation.y += 20;
	itemBox = savedIndicator.position = { startingLocation.x, startingLocation.y, startingLocation.x + 80, startingLocation.y + 20 };
	savedIndicator.hwnd = CreateWindowEx(NULL, "BUTTON", "Saved?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_LEFTTEXT,
		itemBox.left, itemBox.top, itemBox.right - itemBox.left, itemBox.bottom - itemBox.top,
		parent, (HMENU)savedIndicatorID, GetModuleHandle(NULL), NULL);
	SendMessage(savedIndicator.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	SendMessage(savedIndicator.hwnd, BM_SETCHECK, BST_CHECKED, NULL);
	isSaved = true;
	startingLocation.y += 20;
	itemBox = childCombo.position = { startingLocation.x, startingLocation.y, startingLocation.x + width, startingLocation.y + 800 };
	childCombo.hwnd = CreateWindowEx(NULL, TEXT("ComboBox"), "", CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE,
		itemBox.left, itemBox.top, itemBox.right - itemBox.left, itemBox.bottom - itemBox.top, 
		parent,	(HMENU)childComboID, GetModuleHandle(NULL), NULL);
	SendMessage(childCombo.hwnd, WM_SETFONT, WPARAM(sNormalFont), TRUE);
	SendMessage(childCombo.hwnd, CB_ADDSTRING, 0, (LPARAM)("Parent Script"));
	SendMessage(childCombo.hwnd, CB_SETCURSEL, 0, 0);
	startingLocation.y += 25;
	// Edit
	itemBox = edit.position = { startingLocation.x, startingLocation.y, startingLocation.x + width, height};
	edit.hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, _T("RICHEDIT50W"), "",
		WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | WS_HSCROLL,
		itemBox.left, itemBox.top, itemBox.right - itemBox.left, itemBox.bottom - itemBox.top,
		parent, (HMENU)editID, GetModuleHandle(NULL), NULL);
	SendMessage(edit.hwnd, WM_SETFONT, WPARAM(sCodeFont), TRUE);
	SendMessage(edit.hwnd, EM_SETBKGNDCOLOR, 0, RGB(30, 25, 25));
	SendMessage(edit.hwnd, EM_SETEVENTMASK, 0, ENM_CHANGE);
	SendMessage(edit.hwnd, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&myCharFormat);
	return false;
}

bool Script::reorganizeControls()
{
	return false;
}

bool Script::childComboChangeHandler(WPARAM messageWParam, LPARAM messageLParam)
{
	int controlID = GetDlgCtrlID((HWND)messageLParam);
	if (controlID != this->childComboID)
	{
		return true;
	}
	if (HIWORD(messageWParam) != CBN_SELCHANGE)
	{
		return true;
	}
	// prompt for save
	this->checkSave();
	// get text from child

	// check what was selected.
	int selection = SendMessage(this->childCombo.hwnd, CB_GETCURSEL, 0, 0);
	TCHAR selectedText[256];
	SendMessage(this->childCombo.hwnd, CB_GETLBTEXT, selection, (LPARAM)selectedText);
	std::string viewName(selectedText);
	this->changeView(viewName);
	this->updateSavedStatus(true);
	return false;
}

bool Script::changeView(std::string viewName)
{
	if (viewName == "Parent Script")
	{
		// load parent
		this->loadFile(eProfile.getCurrentPathIncludingCategory() + scriptName + extension);
	}
	else
	{
		// load child
		this->loadFile(eProfile.getCurrentPathIncludingCategory() + viewName);
	}
	return false;
}
//
bool Script::saveScript()
{
	if (scriptName == "")
	{
		// must give new name. This should only work if the experiment and category have been set. 
		if (eProfile.getCurrentExperiment() == "")
		{
			errBox("The script is unnamed, and no experiment has been set. Please select an experiment and category or select \"Save As\" to save in an arbitrary location.");
			return true;
		}
		if (eProfile.getCurrentCategory() == "")
		{
			errBox("The script is unnamed, and no Category has been set. Please select a category or select \"Save As\" to save in an arbitrary location.");
			return true;
		}
		std::string newName = (const char*)DialogBoxParam(eGlobalInstance, MAKEINTRESOURCE(IDD_TEXT_PROMPT_DIALOG), 0, (DLGPROC)textPromptDialogProcedure,
			(LPARAM)("The " + deviceType + " script is unnamed! Please enter new name for the script, and the script will be saved in the current location: " + eProfile.getCurrentExperiment() + " -> " + eProfile.getCurrentCategory()).c_str());
		if (newName == "")
		{
			// canceled
			return true;
		}
		std::string path = eProfile.getCurrentPathIncludingCategory() + newName + extension;
		this->saveScriptAs(path);
	}
	if (eSystemIsRunning)
	{
		if (scriptName == eMostRecentHorizontalScriptNames || scriptName == eMostRecentIntensityScriptNames || scriptName == eMostRecentVerticalScriptNames)
		{
			MessageBox(0, "ERROR: System is currently running. You can't save over any files in use by the system while it runs, which includes the "
				"horizontal and vertical AOM scripts and the intensity script.", 0, 0);
			return true;
		}
	}
	int textLength = GetWindowTextLength(this->edit.hwnd);
	// + 1 for null at end
	char* editText = new char[textLength + 1];
	int myError = GetWindowText(this->edit.hwnd, editText, textLength + 1);
<<<<<<< HEAD
=======
	//std::fstream saveFile(eProfile.getCurrentPathIncludingCategory() + scriptName + extension, std::fstream::out);
>>>>>>> aa1508cc322859dd996de8ea3251432cbf6201b3
	std::fstream saveFile;
	int extPos = this->scriptName.find_last_of(".");
	if (extPos != -1)
	{
		// the scriptname already has an extension...
<<<<<<< HEAD
		std::string existingExtension = this->scriptName.substr(extPos + 1);
=======
		std::string existingExtension = this->scriptName.substr(extPos);
>>>>>>> aa1508cc322859dd996de8ea3251432cbf6201b3
		std::string nameNoExtension = this->scriptName.substr(0, extPos);
		if (existingExtension != this->extension)
		{
			errBox("ERROR: The " + this->deviceType + " scriptName (as understood by the code) already has an "
<<<<<<< HEAD
				"extension, and that extension doesn't match the extension for this device! Script name is: " +
				scriptName + " While the proper extension is " + extension + ". The file will be saved as " +
=======
				"extension, read by the code as " + existingExtension + ", and that extension doesn't match the extension for this device! Script name is: " +
				scriptName + " While the proper extension is " + this->extension + ". The file will be saved as " +
>>>>>>> aa1508cc322859dd996de8ea3251432cbf6201b3
				nameNoExtension + extension);
			std::string path = eProfile.getCurrentPathIncludingCategory() + nameNoExtension + extension;
			this->saveScriptAs(path);
			return false;
		}
		else
		{
			// take the extension off of the script name. That's no good. 
			this->scriptName = nameNoExtension;
<<<<<<< HEAD
			saveFile.open(eProfile.getCurrentPathIncludingCategory() + scriptName + extension , std::fstream::out);
=======
			saveFile.open(eProfile.getCurrentPathIncludingCategory() + scriptName + extension, std::fstream::out);
>>>>>>> aa1508cc322859dd996de8ea3251432cbf6201b3
		}
	}
	else
	{
		// In theory the code should always do this line, not the above check.
		saveFile.open(eProfile.getCurrentPathIncludingCategory() + scriptName + extension, std::fstream::out);
	}
	if (!saveFile.is_open())
	{
		MessageBox(0, ("ERROR: Failed to open script file: " + eProfile.getCurrentPathIncludingCategory() + scriptName + extension).c_str(), 0, 0);
		return true;
	}
	saveFile << editText;
	delete editText;
	saveFile.close();
	this->scriptAddress = eProfile.getCurrentPathIncludingCategory() + scriptName + extension;
	this->scriptCategory = eProfile.getCurrentCategory();
	this->scriptExperiment = eProfile.getCurrentExperiment();
	this->updateSavedStatus(true);
	return false;
}
//
bool Script::saveScriptAs(std::string location)
{
	if (location == "")
	{
		return true;
	}
	if (eSystemIsRunning)
	{
		if (location == eMostRecentHorizontalScriptNames || location == eMostRecentIntensityScriptNames || location == eMostRecentVerticalScriptNames)
		{
			MessageBox(0, "ERROR: System is currently running. You can't save over any files in use by the system while it runs, which includes the "
				"horizontal and vertical AOM scripts and the intensity script.", 0, 0);
			return true;
		}
	}
	int textLength = GetWindowTextLength(this->edit.hwnd);
	// + 1 for null at end
	char* editText = new char[textLength + 1];
	int myError = GetWindowText(this->edit.hwnd, editText, textLength + 1);
	std::fstream saveFile(location, std::fstream::out);
	if (!saveFile.is_open())
	{
		MessageBox(0, ("ERROR: Failed to open script file: " + location).c_str(), 0, 0);
		return true;
	}
	saveFile << editText;
	char fileChars[_MAX_FNAME];
	// this location should have the script name, the script category, and the script experiment location in it.
	this->scriptAddress = location;
	int position = location.find_last_of("\\");
	scriptName = location.substr(position + 1, location.size());
	location = location.substr(0, position);
	position = location.find_last_of("\\");
	scriptCategory = location.substr(position + 1, location.size());
	location = location.substr(0, position);
	position = location.find_last_of("\\");
	scriptExperiment = location.substr(position + 1, location.size());
	delete editText;
	saveFile.close();
	this->updateSavedStatus(true);
	return false;
}
bool Script::checkChildSave()
{
	// get the 
	if (isSaved)
	{
		// don't need to do anything
		return false;
	}
	int selection = SendMessage(this->childCombo.hwnd, CB_GETCURSEL, 0, 0);
	TCHAR name[256];
	SendMessage(this->childCombo.hwnd, CB_GETLBTEXT, selection, (LPARAM)name);
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
			std::string path = eProfile.getCurrentPathIncludingCategory() + newName + this->extension;
			this->saveScriptAs(path);
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
			this->saveScriptAs(nameStr);
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
bool Script::checkSave()
{
	if (isSaved)
	{
		// don't need to do anything
		return false;
	}
	if (scriptName == "")
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
			std::string path = eProfile.getCurrentPathIncludingCategory() + newName + this->extension;
			this->saveScriptAs(path);
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
		int answer = MessageBox(0, ("Save " + deviceType + " script file as " + scriptName + extension + "?").c_str(), 0, MB_YESNOCANCEL);
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
			this->saveScript();
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
bool Script::renameScript()
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
	int result = MoveFile((eProfile.getCurrentPathIncludingCategory() + scriptName + extension).c_str(),
		(eProfile.getCurrentPathIncludingCategory() + newName + extension).c_str());
	if (result == 0)
	{
		MessageBox(0, "ERROR: Failed to move file.", 0, 0);
		return true;
	}
	this->scriptAddress = eProfile.getCurrentPathIncludingCategory() + scriptName + extension;
	this->scriptCategory = eProfile.getCurrentCategory();
	this->scriptExperiment = eProfile.getCurrentExperiment();
	return false;
}
//
bool Script::deleteScript()
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

	int result = DeleteFile((eProfile.getCurrentPathIncludingCategory() + scriptName + extension).c_str());
	if (result == 0)
	{
		MessageBox(0, "ERROR: Deleting script file failed!", 0, 0);
		return true;
	}
	else
	{
		this->scriptName = "";
		this->scriptAddress = "";
		this->scriptCategory = "";
		this->scriptExperiment = "";
	}
	return false;
}
//
bool Script::newScript()
{
	std::string tempName;
	tempName = DEFAULT_SCRIPT_FOLDER_PATH;
	if (deviceType == "Horizontal NIAWG")
	{
		if (eProfile.getOrientation() == HORIZONTAL_ORIENTATION)
		{
			tempName += "DEFAULT_HCONFIG_HORIZONTAL_SCRIPT.nScript";
			//"DEFAULT_HORIZONTAL_SCRIPT.script"
		}
		else if (eProfile.getOrientation() == VERTICAL_ORIENTATION)
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
		if (eProfile.getOrientation() == HORIZONTAL_ORIENTATION)
		{
			tempName += "DEFAULT_HCONFIG_VERTICAL_SCRIPT.nScript";
			//"DEFAULT_HORIZONTAL_SCRIPT.script"
		}
		else if (eProfile.getOrientation() == VERTICAL_ORIENTATION)
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
	this->loadFile(tempName);
	// add the current category to the address. 
	this->scriptAddress = eProfile.getCurrentPathIncludingCategory();
	this->scriptCategory = eProfile.getCurrentCategory();
	this->scriptExperiment = eProfile.getCurrentExperiment();
	return false;
}
//
bool Script::openParentScript(std::string parentScriptFileAndPath)
{
	if (parentScriptFileAndPath == "")
	{
		return false;
	}
	char fileChars[_MAX_FNAME];
	char extChars[_MAX_EXT];
	int myError = _splitpath_s(parentScriptFileAndPath.c_str(), NULL, 0, NULL, 0, fileChars, _MAX_FNAME, extChars, _MAX_EXT);
	std::string extStr(extChars);
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
	this->scriptName = std::string(fileChars);
	this->scriptAddress = eProfile.getCurrentPathIncludingCategory() + scriptName + extension;
	this->scriptCategory = eProfile.getCurrentCategory();
	this->scriptExperiment = eProfile.getCurrentExperiment();
	if (!this->loadFile(parentScriptFileAndPath))
	{
		return true;
	}
<<<<<<< HEAD
=======
	std::string location = parentScriptFileAndPath;
	this->scriptAddress = location;
	int position = location.find_last_of("\\");
	scriptName = location.substr(position + 1, location.size());
	location = location.substr(0, position);
	position = location.find_last_of("\\");
	scriptCategory = location.substr(position + 1, location.size());
	location = location.substr(0, position);
	position = location.find_last_of("\\");
	scriptExperiment = location.substr(position + 1, location.size());

>>>>>>> aa1508cc322859dd996de8ea3251432cbf6201b3
	this->updateSavedStatus(true);
	// Check location of the script.
	position = parentScriptFileAndPath.find_last_of('\\');
	std::string scriptLocation = parentScriptFileAndPath.substr(0, position);
	if (scriptLocation + "\\" != (eProfile.getCurrentPathIncludingCategory()) && eProfile.getCurrentPathIncludingCategory() != "")
	{
		int answer = MessageBox(0, ("The requested script " + scriptName + " at " + scriptLocation + " is not currently located in the current configuration "
			"folder (" + eProfile.getCurrentPathIncludingCategory() + ". This is recommended so that scripts related to a particular configuration are "
			"reserved to that category folder. Copy script to current category folder?").c_str(), 0, MB_YESNO);
		if (answer == IDYES)
		{
<<<<<<< HEAD
			std::string location = (eProfile.getCurrentPathIncludingCategory()) + scriptName;
			std::string path = location;
			this->scriptAddress = location;
			int position = location.find_last_of("\\");
			this->scriptName = location.substr(position + 1, location.size());
			location = location.substr(0, position);
			position = location.find_last_of("\\");
			this->scriptCategory = location.substr(position + 1, location.size());
			location = location.substr(0, position);
			position = location.find_last_of("\\");
			this->scriptExperiment = location.substr(position + 1, location.size());
=======
			std::string scriptName = parentScriptFileAndPath.substr(position + 1, parentScriptFileAndPath.size());
			this->scriptAddress = eProfile.getCurrentPathIncludingCategory() + scriptName;
			this->scriptCategory = eProfile.getCurrentCategory();
			this->scriptExperiment = eProfile.getCurrentExperiment();
			std::string path = (eProfile.getCurrentPathIncludingCategory()) + scriptName;
>>>>>>> aa1508cc322859dd996de8ea3251432cbf6201b3
			this->saveScriptAs(path);
		}
	}
	this->updateScriptNameText();
	this->colorEntireScript();
	return false;
}

/*
]---	This function only puts the given file on the edit for this class, it doesn't change current settings parameters. It's used bare when just changing the
]-		view of the edit, while it's used with some surrounding changes for loading a new parent.
 */
bool Script::loadFile(std::string pathToFile)
{
	std::ifstream openFile(pathToFile.c_str());
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
	SendMessage(this->edit.hwnd, WM_SETTEXT, NULL, (LPARAM)fileText.c_str());
	this->colorEntireScript();
	this->updateScriptNameText();
	openFile.close();
	return true;
}

bool Script::reset()
{
	this->scriptName = "";
	this->scriptAddress = "";
	this->scriptCategory = "";
	this->scriptExperiment = "";
	this->updateSavedStatus(false);
	SetWindowText(this->fileNameText.hwnd, "");
	SendMessage(this->edit.hwnd, WM_SETTEXT, NULL, (LPARAM)"");
	return false;
}

bool Script::savedStatus()
{
	return isSaved;
}

std::string Script::getScriptPathAndName()
{
	return this->scriptAddress;
}

std::string Script::getScriptName()
{
	return this->scriptName;
}

bool Script::considerCurrentLocation()
{
	if (this->scriptAddress.size() > 0)
	{
		// Check location of vertical script.
		int position = this->scriptAddress.find_last_of('\\');
		std::string scriptLocation = this->scriptAddress.substr(0, position);
		if (scriptLocation + "\\" != eProfile.getCurrentPathIncludingCategory())
		{
			int answer = MessageBox(0, ("The requested script " + scriptName + " at " + scriptLocation + " is not currently located in the current configuration "
				"folder (" + eProfile.getCurrentPathIncludingCategory() + ". This is recommended so that scripts related to a particular configuration are "
				"reserved to that category folder. Copy script to current category folder?").c_str(), 0, MB_YESNO);
			if (answer == IDYES)
			{
				// grab the correct file name at the end.
				this->scriptName = this->scriptAddress.substr(position, this->scriptAddress.size());
				// this name includes the extension already.
				this->scriptAddress = eProfile.getCurrentPathIncludingCategory() + scriptName;
				this->scriptCategory = eProfile.getCurrentCategory();
				this->scriptExperiment = eProfile.getCurrentExperiment();
				this->saveScriptAs(this->scriptAddress);
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
	SendMessage(this->fileNameText.hwnd, WM_SETTEXT, 0, (LPARAM)text.c_str());
	return false;
}
