#include "stdafx.h"
#include "textPromptDialogProcedure.h"
#include "Windows.h"
#include "resource.h"
#include "externals.h"

IMPLEMENT_DYNAMIC(TextPromptDialog, CDialog)

BEGIN_MESSAGE_MAP(TextPromptDialog, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_COMMAND(IDOK, &TextPromptDialog::catchOk)
	ON_COMMAND(IDCANCEL, &TextPromptDialog::catchCancel)
	// 
END_MESSAGE_MAP()

BOOL TextPromptDialog::OnInitDialog()
{
	description.Create(descriptionText.c_str(), WS_CHILD | WS_VISIBLE | ES_READONLY | WS_BORDER, { 0,0,1000,75 }, this, 0);
	prompt.Create(WS_CHILD | WS_VISIBLE | WS_BORDER, { 0,75,1000,150 }, this, 0);
	return TRUE;
}


void TextPromptDialog::catchCancel()
{
	*result = "";
	EndDialog(1);
}

void TextPromptDialog::catchOk()
{
	CString text;
	prompt.GetWindowText(text);
	std::string textStr(text);
	*result = textStr.c_str();
	EndDialog(0);
}


/*
INT_PTR CALLBACK textPromptDialogProcedure(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
		{
			// lParam contains the text to be displayed on this guy. 
			CreateWindowEx(0, "STATIC", (const char*)lParam, WS_CHILD | WS_VISIBLE | SS_LEFT, 5, 5, 400, 60, hDlg, (HMENU)-1, NULL, NULL);
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
*/