#include "stdafx.h"
#include "namePromptDialogProc.h"
#include "Windows.h"
#include "resource.h"
#include "externals.h"

INT_PTR CALLBACK namePromptDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
		{
			CreateWindowEx(0, "STATIC", "EM Gain Setting:",
				WS_CHILD | WS_VISIBLE | BS_RIGHT,
				initPos.left, initPos.top, initPos.right - initPos.left, initPos.bottom - initPos.top, e,
				(HMENU)-1, eHInst, NULL);
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
