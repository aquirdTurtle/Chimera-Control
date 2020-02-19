// created by Mark O. Brown
#include "stdafx.h"
#include "TextPromptDialog.h"

IMPLEMENT_DYNAMIC(TextPromptDialog, CDialog)

BEGIN_MESSAGE_MAP(TextPromptDialog, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_COMMAND(IDOK, &TextPromptDialog::catchOk)
	ON_COMMAND(IDCANCEL, &TextPromptDialog::catchCancel)
	ON_COMMAND(ID_ACCELERATOR_F5 , &TextPromptDialog::catchf5)
END_MESSAGE_MAP()

void TextPromptDialog::catchf5( ) { }

HBRUSH TextPromptDialog::OnCtlColor (CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	switch (nCtlColor)
	{
		case CTLCOLOR_EDIT:
		{
			pDC->SetTextColor (_myRGBs["MainWin-Text"]);
			pDC->SetBkColor (_myRGBs["Interactable-Bkgd"]);
			return *_myBrushes["Interactable-Bkgd"];
		}
		default:
		{
			pDC->SetTextColor (_myRGBs["Text"]);
			pDC->SetBkColor (_myRGBs["Main-Bkgd"]);
			return *_myBrushes["Main-Bkgd"];
		}
	}
}


BOOL TextPromptDialog::OnInitDialog()
{
	DWORD options = WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER;
	if (passwordOption)
	{
		options |= ES_PASSWORD;
	}
	RECT rect;
	GetClientRect (&rect);
	description.Create(cstr(descriptionText), options | ES_READONLY, { 0,0,rect.right,75 }, this, 0);
	prompt.Create( options , { 0,75,rect.right,rect.bottom - 50 }, this, 0 );
	prompt.SetWindowText (initStringValue.c_str());
	okBtn.sPos = { 0, rect.bottom - 50, rect.right/2, rect.bottom };
	okBtn.Create ("OK", NORM_PUSH_OPTIONS, okBtn.sPos, this, IDOK);
	cancelBtn.sPos = { rect.right/2, rect.bottom - 50, rect.right, rect.bottom };
	cancelBtn.Create ("CANCEL", NORM_PUSH_OPTIONS, cancelBtn.sPos, this, IDCANCEL);
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
	*result = cstr(textStr);
	EndDialog(0);
}
