#include "stdafx.h"
#include "MainWindow.h"
#include "initializeMainWindow.h"

IMPLEMENT_DYNAMIC(MainWindow, CDialog)

BEGIN_MESSAGE_MAP(MainWindow, CDialog)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

BOOL MainWindow::OnInitDialog()
{
	initializeMainWindow(*this);
	CMenu menu;
	menu.LoadMenu(IDR_MAIN_MENU);
	this->SetMenu(&menu);
	this->ShowWindow(SW_MAXIMIZE);
	return TRUE;
}


HBRUSH MainWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	switch (nCtlColor)
	{
		case CTLCOLOR_STATIC:
		{
			pDC->SetTextColor(mainRGBs["White"]);
			pDC->SetBkColor(mainRGBs["Medium Grey"]);
			return mainBrushes["Medium Grey"];
		}
		case CTLCOLOR_EDIT:
		{
			pDC->SetTextColor(mainRGBs["White"]);
			pDC->SetBkColor(mainRGBs["Dark Blue"]);
			return mainBrushes["Dark Blue"];
		}
		case CTLCOLOR_LISTBOX:
		{
			pDC->SetTextColor(mainRGBs["White"]);
			pDC->SetBkColor(mainRGBs["Dark Grey"]);
			return mainBrushes["Dark Grey"];
		}
		default:
		{
			return mainBrushes["Light Grey"];
		}
	}
}