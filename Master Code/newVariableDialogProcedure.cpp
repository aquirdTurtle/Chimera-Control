#include "stdafx.h"
#include "newVariableDialogProcedure.h"
#include "Windows.h"

INT_PTR CALLBACK newVariableDialogProcedure(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
				case IDCANCEL:
				{
					EndDialog(hDlg, LOWORD(NULL));
					return (INT_PTR)TRUE;
				}
				case IDOK:
				{
					EndDialog(hDlg, WPARAM(""));
					return (INT_PTR)TRUE;
				}
			}
			break;
		}
	}
	return (INT_PTR)FALSE;
}
