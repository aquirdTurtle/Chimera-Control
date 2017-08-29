#include "stdafx.h"
#include "beginningSettingsDialogProc.h"
#include "fonts.h"
#include <sstream>


INT_PTR CALLBACK beginningSettingsDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
		{
			LoadLibrary(TEXT("Msftedit.dll"));
			eBeginDialogRichEdit = CreateWindowEx(0, _T("RICHEDIT50W"), "",
												  WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_AUTOHSCROLL | WS_HSCROLL | ES_READONLY,
												  0, 0, 572, 400, hDlg, (HMENU)IDC_BEGINNING_DIALOG_RICH_EDIT, GetModuleHandle(NULL), NULL);
			//SendMessage(eBeginDialogRichEdit, WM_SETFONT, WPARAM(sCodeFont), TRUE);
			std::string text((const char*)lParam);
			SetWindowText(eBeginDialogRichEdit, cstr(text));
			std::stringstream streamObj(text);
			std::string line;
			DWORD start = 0, end = 0;
			std::size_t prev, pos;
			std::string word, analysisWord;
			CHARFORMAT syntaxFormat;
			memset(&syntaxFormat, 0, sizeof(CHARFORMAT));
			syntaxFormat.cbSize = sizeof(CHARFORMAT);
			syntaxFormat.dwMask = CFM_COLOR;
			while (std::getline(streamObj, line))
			{
				DWORD lineStartCoordingate = start;

				prev = 0;
				
				while ((pos = line.find_first_of(" \t\r\n", prev)) != std::string::npos)
				{
					end = lineStartCoordingate + pos;
					word = line.substr(prev, pos - prev + 1);
					analysisWord = word.substr(0, word.length() - 1);
					if (analysisWord == "TRUE" || analysisWord == "SAVED")
					{
						syntaxFormat.crTextColor = RGB(0, 150, 0);
						SendMessage(eBeginDialogRichEdit, EM_SETSEL, start, end);
						SendMessage(eBeginDialogRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
					}
					else if (analysisWord == "FALSE" || analysisWord == "NOT")
					{
						syntaxFormat.crTextColor = RGB(255, 0, 0);
						SendMessage(eBeginDialogRichEdit, EM_SETSEL, start, end);
						SendMessage(eBeginDialogRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
					}
					end++;
					start = end;
					prev = pos + 1;
				}
				if (prev < line.length())
				{
					word = line.substr(prev, std::string::npos);
					if (analysisWord == "TRUE" || analysisWord == "SAVED")
					{
						syntaxFormat.crTextColor = RGB(0, 150, 0);
						SendMessage(eBeginDialogRichEdit, EM_SETSEL, start, end);
						SendMessage(eBeginDialogRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
					}
					else if (analysisWord == "FALSE" || analysisWord == "NOT")
					{
						syntaxFormat.crTextColor = RGB(255, 0, 0);
						SendMessage(eBeginDialogRichEdit, EM_SETSEL, start, end);
						SendMessage(eBeginDialogRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&syntaxFormat);
					}
					end++;
					start = end;
				}
			}

			return (INT_PTR)TRUE;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDC_BEGIN_SETTINGS_OK:
				{
					EndDialog(hDlg, LOWORD(false));
					return (INT_PTR)TRUE;
				}
				case IDC_BEGIN_SETTINGS_CANCEL:
				{
					EndDialog(hDlg, LOWORD(true));
					return (INT_PTR)TRUE;
				}
			}
			break;
		}
	}
	return FALSE;
}
