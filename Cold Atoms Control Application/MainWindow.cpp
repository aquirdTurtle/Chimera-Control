#include "stdafx.h"
#include "MainWindow.h"
#include "initializeMainWindow.h"

IMPLEMENT_DYNAMIC(MainWindow, CDialog)

BEGIN_MESSAGE_MAP(MainWindow, CDialog)

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

