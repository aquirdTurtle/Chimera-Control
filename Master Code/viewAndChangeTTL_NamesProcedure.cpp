#pragma once

#include "stdafx.h"
#include "viewAndChangeTTL_NamesProcedure.h"
#include "Control.h"
#include <array>
#include "TTL_System.h"

INT_PTR CALLBACK viewAndChangeTTL_NamesProcedure(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static ttlInputStruct* input;
	static std::array<std::array<HWND, 16>, 4> edits;
	static int startx = 30, starty = 40;
	static int width = 63;
	static int height = 28;
	static int startID = 345;

	switch (message)
	{
		case WM_INITDIALOG:
		{
			input = (ttlInputStruct*)lParam;
			for (int rowInc = 0; rowInc < edits.size(); rowInc++)
			{
				for (int numberInc = 0; numberInc < edits[rowInc].size(); numberInc++)
				{
					edits[rowInc][numberInc] = CreateWindowEx(NULL, "EDIT", input->ttls->getName(rowInc, numberInc).c_str(), WS_CHILD | WS_VISIBLE | SS_SUNKEN | WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP,
						startx + numberInc * width, starty + rowInc * height, width, 25,
						hDlg, (HMENU)(startID + rowInc * edits[0].size() + numberInc), GetModuleHandle(NULL), NULL);
				}
			}

			break;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDCANCEL:
				{
					EndDialog(hDlg, (WPARAM)&input);
					return (INT_PTR)TRUE;
				}
				case IDOK:
				{
					for (int rowInc = 0; rowInc < edits.size(); rowInc++)
					{
						for (int numberInc = 0; numberInc < edits[rowInc].size(); numberInc++)
						{
							TCHAR name[256];
							SendMessage(edits[rowInc][numberInc], WM_GETTEXT, 256, (LPARAM)name);
							input->ttls->setName(rowInc, numberInc, name, input->toolTips, input->master);
						}
					}
					EndDialog(hDlg, (WPARAM)&input);
					return (INT_PTR)TRUE;
				}
			}
			break;
		}
	}
	return (INT_PTR)FALSE;
}