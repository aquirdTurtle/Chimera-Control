#include "stdafx.h"
#include "viewAndChangeDAC_NamesProcedure.h"
#include "DAC_System.h"


INT_PTR CALLBACK viewAndChangeDAC_NamesProcedure(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static dacInputStruct input;
	static std::array<HWND, 24> edits;
	static int startx = 30, starty = 40;
	static int width = 63;
	static int height = 28;
	static int startID = 445;

	switch (message)
	{
		case WM_INITDIALOG:
		{
			input = *(dacInputStruct*)lParam;
			RECT location = { 0, 0, 20, 20 };
			for (int dacInc = 0; dacInc < edits.size(); dacInc++)
			{
				if (dacInc == edits.size() / 3 || dacInc == 2 * edits.size() / 3)
				{
					// go to second or third collumn
					location.left += 160;
					location.right += 160;
					location.top -= 25 * edits.size() / 3;
					location.bottom -= 25 * edits.size() / 3;
				}
				// create label
				CreateWindowEx(0, "STATIC", std::to_string(dacInc + 1).c_str(), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
					location.left, location.top, location.right - location.left, location.bottom - location.top,
					hDlg, (HMENU)-1, GetModuleHandle(NULL), NULL);
				location.left += 20;
				location.right += 140;
				edits[dacInc] = CreateWindowEx(0, "EDIT", input.dacs->getName(dacInc).c_str(), WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					location.left, location.top, location.right - location.left, location.bottom - location.top,
					hDlg, (HMENU)startID + dacInc, GetModuleHandle(NULL), NULL);

				location.left -= 20;
				location.right -= 140;
				location.top += 25;
				location.bottom += 25;
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
					for (int dacInc = 0; dacInc < edits.size(); dacInc++)
					{
						TCHAR name[256];
						/// MAKE SURE that there are 16 numbers per row.
						SendMessage(edits[dacInc], WM_GETTEXT, 256, (LPARAM)name);
						input.dacs->setName(dacInc, name, input.toolTips, input.master);
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