#include "stdafx.h"
#include "colorScript.h"
#include <string>
#include <sstream>
#include <stdio.h>
#include "getSyntaxColor.h"
#include "externals.h"
#include "appendText.h"

int colorScript(HWND scriptEdit, std::string editType, DWORD beginingOfChange, DWORD endOfChange, HWND scriptPredefinitionsCombo, bool isParent)
{
	long long beginingSigned = beginingOfChange;
	long long endSigned = endOfChange;
	LRESULT scriptLength = SendMessage(scriptEdit, WM_GETTEXTLENGTH, 0, 0);
	char* buffer = new char[scriptLength + 1];
	SendMessage(scriptEdit, WM_GETTEXT, scriptLength + 1, (LPARAM)buffer);
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
	int relevantID = GetDlgCtrlID(scriptEdit);
	DWORD start = 0, end = 0;
	std::size_t prev, pos;
	
	// get the entries in the combo
	// now check to see if I need to add this to the combo
	int count = SendMessage(scriptPredefinitionsCombo, CB_GETCOUNT, 0, 0);
	for (int predefinedInc = 0; predefinedInc < count; predefinedInc++)
	{
		TCHAR itemText[256];
		SendMessage(scriptPredefinitionsCombo, CB_GETLBTEXT, predefinedInc, (LPARAM)itemText);
		predefinedScripts.push_back(itemText);
	}
	
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
			analysisWord = word.substr(0, word.length() - 1);
			// if comment is found, the rest of the line is green.
			if (coloring != "comment1" && coloring != "comment2")
			{
				tempColor = getSyntaxColor(analysisWord, editType);
				if (tempColor != coloring)
				{
					coloring = tempColor;
					if (coloring == "comment1")
					{
						syntaxFormat.crTextColor = RGB(34, 139, 34);
						SendMessage(scriptEdit, EM_SETSEL, start, end);
						SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "comment2")
					{
						syntaxFormat.crTextColor = RGB(107, 35, 35);
						SendMessage(scriptEdit, EM_SETSEL, start, end);
						SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "command")
					{
						syntaxFormat.crTextColor = RGB(100, 100, 205);
						SendMessage(scriptEdit, EM_SETSEL, start, end);
						SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "number")
					{
						syntaxFormat.crTextColor = RGB(255, 255, 255);
						SendMessage(scriptEdit, EM_SETSEL, start, end);
						SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "logic")
					{
						syntaxFormat.crTextColor = RGB(0, 255, 255);
						SendMessage(scriptEdit, EM_SETSEL, start, end);
						SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "delimiter")
					{
						syntaxFormat.crTextColor = RGB(100, 100, 100);
						SendMessage(scriptEdit, EM_SETSEL, start, end);
						SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "variable")
					{
						syntaxFormat.crTextColor = RGB(255, 215, 0);
						SendMessage(scriptEdit, EM_SETSEL, start, end);
						SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "option")
					{
						syntaxFormat.crTextColor = RGB(210, 180, 140);
						SendMessage(scriptEdit, EM_SETSEL, start, end);
						SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "script")
					{
						syntaxFormat.crTextColor = RGB(147, 112, 219);
						SendMessage(scriptEdit, EM_SETSEL, lineStartCoordingate, end);
						SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "script file")
					{
						syntaxFormat.crTextColor = RGB(147, 112, 219);
						SendMessage(scriptEdit, EM_SETSEL, lineStartCoordingate, end);
						SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
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
								SendMessage(scriptPredefinitionsCombo, CB_ADDSTRING, 0, (LPARAM)line.substr(0, line.size() - 8).c_str());
							}
						}

					}
					else if (coloring == "unrecognized")
					{
						syntaxFormat.crTextColor = RGB(255, 0, 0);
						SendMessage(scriptEdit, EM_SETSEL, start, end);
						SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
				}
			}

			SendMessage(scriptEdit, EM_SETSEL, start, end);
			SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
			end++;
			start = end;
			//SendMessage(scriptEdit, EM_REPLACESEL, 0, (LPARAM)word.c_str());
			prev = pos + 1;
		}
		if (prev < line.length())
		{
			word = line.substr(prev, std::string::npos);
			tempColor = getSyntaxColor(word, editType);
			if (coloring != "comment1" && coloring != "comment2")
			{
				if (tempColor != coloring)
				{
					if (coloring == "comment1")
					{
						syntaxFormat.crTextColor = RGB(34, 139, 34);
						SendMessage(scriptEdit, EM_SETSEL, start, end);
						SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "comment2")
					{
						syntaxFormat.crTextColor = RGB(107, 35, 35);
						SendMessage(scriptEdit, EM_SETSEL, start, end);
						SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "command")
					{
						syntaxFormat.crTextColor = RGB(100, 100, 205);
						SendMessage(scriptEdit, EM_SETSEL, start, end);
						SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "number")
					{
						syntaxFormat.crTextColor = RGB(255, 255, 255);
						SendMessage(scriptEdit, EM_SETSEL, start, end);
						SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "logic")
					{
						syntaxFormat.crTextColor = RGB(0, 255, 255);
						SendMessage(scriptEdit, EM_SETSEL, start, end);
						SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "unrecognized")
					{
						syntaxFormat.crTextColor = RGB(255, 0, 0);
						SendMessage(scriptEdit, EM_SETSEL, start, end);
						SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "delimiter")
					{
						syntaxFormat.crTextColor = RGB(100, 100, 100);
						SendMessage(scriptEdit, EM_SETSEL, start, end);
						SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "variable")
					{
						syntaxFormat.crTextColor = RGB(255, 215, 0);
						SendMessage(scriptEdit, EM_SETSEL, start, end);
						SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "option")
					{
						syntaxFormat.crTextColor = RGB(210, 180, 140);
						SendMessage(scriptEdit, EM_SETSEL, start, end);
						SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "script")
					{
						syntaxFormat.crTextColor = RGB(147, 112, 219);
						SendMessage(scriptEdit, EM_SETSEL, lineStartCoordingate, end);
						SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
					else if (coloring == "script file")
					{
						syntaxFormat.crTextColor = RGB(147, 112, 219);
						SendMessage(scriptEdit, EM_SETSEL, lineStartCoordingate, end);
						SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
						if (isParent)
						{
							bool isNew = true;
							for (int predefinedInc = 0; predefinedInc < predefinedScripts.size(); predefinedInc++)
							{
								if (line == predefinedScripts[predefinedInc])
								{
									isNew = false;
									break;
								}
							}
							if (isNew)
							{
								predefinedScripts.push_back(line.substr(0, line.size() - 8));
								SendMessage(scriptPredefinitionsCombo, CB_ADDSTRING, 0, (LPARAM)line.substr(0, line.size() - 8).c_str());
							}
						}
					}
					else if (coloring == "unrecognized")
					{
						syntaxFormat.crTextColor = RGB(255, 0, 0);
						SendMessage(scriptEdit, EM_SETSEL, start, end);
						SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
						start = end;
					}
				}
			}
			SendMessage(scriptEdit, EM_SETSEL, start, end);
			SendMessage(scriptEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
			end++;
			start = end;
		}
	}


	return 0;
}
