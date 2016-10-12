#include "stdafx.h"
#include "ScriptingWindow.h"
#include "initializeScriptingWindow.h"
#include "afxwin.h"

IMPLEMENT_DYNAMIC(ScriptingWindow, CDialog)

BEGIN_MESSAGE_MAP(ScriptingWindow, CDialog)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

BOOL ScriptingWindow::OnInitDialog()
{
	initializeScriptingWindow(*this);

	CMenu menu;
	menu.LoadMenu(IDR_MAIN_MENU);
	this->SetMenu(&menu);
	
	this->ShowWindow(SW_MAXIMIZE);
	return TRUE;
}

HBRUSH ScriptingWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	switch (nCtlColor)
	{
		case CTLCOLOR_STATIC:
		{
			pDC->SetTextColor(scriptRGBs["White"]);
			pDC->SetBkColor(scriptRGBs["Dark Grey Red"]);
			return scriptBrushes["Dark Grey Red"];
		}
		case CTLCOLOR_EDIT:
		{
			pDC->SetTextColor(scriptRGBs["White"]);
			pDC->SetBkColor(scriptRGBs["Dark Red"]);
			return scriptBrushes["Dark Red"];
		}
		case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor(scriptRGBs["White"]);
			pDC->SetBkColor(scriptRGBs["Dark Grey"]);
			return scriptBrushes["Dark Grey"];
		}
		default:
		{
			return scriptBrushes["Light Grey"];
		}
	}
}