#include "stdafx.h"
#include "textPromptDialogProcedure.h"
#include "Windows.h"
#include "resource.h"
INT_PTR CALLBACK textPromptDialogProcedure(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
		{
			// lParam contains the text to be displayed on this guy. 
			// CHANGED recently: no more eGlobalInstance here.
			CreateWindowEx(0, "STATIC", (const char*)lParam, WS_CHILD | WS_VISIBLE | SS_LEFT, 0, 0, 460, 60, hDlg, (HMENU)-1, 0, NULL);
			return (INT_PTR)TRUE;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
					case IDCANCEL:
					{
						EndDialog(hDlg, WPARAM(""));
						return (INT_PTR)TRUE;
					}
					case IDOK:
					{
						HWND inputEdit = GetDlgItem(hDlg, IDD_TEXT_PROMPT_DIALOG);
						int editLength = GetWindowTextLength(inputEdit);
						TCHAR* buffer;
						buffer = new TCHAR[1024];
						GetWindowText(inputEdit, buffer, 1024);
						EndDialog(hDlg, WPARAM(buffer));
						return (INT_PTR)TRUE;
					}
			}
			break;
		}
	}
	return (INT_PTR)FALSE;
}