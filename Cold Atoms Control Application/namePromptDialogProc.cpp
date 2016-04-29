#include "stdafx.h"
#include "namePromptDialogProc.h"
#include "Windows.h"
#include "resource.h"

INT_PTR CALLBACK namePromptDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
		{
			return (INT_PTR)TRUE;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam)) 
			{
				case IDC_FOLDER_PROMPT_CANCEL:
				{
					EndDialog(hDlg, LOWORD(NULL));
					return (INT_PTR)TRUE;
				}
				case IDOK:
				{
					HWND inputEdit = GetDlgItem(hDlg, IDC_FOLDER_NAME_INPUT);
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
