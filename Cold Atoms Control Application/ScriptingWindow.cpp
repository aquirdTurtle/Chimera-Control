#include "stdafx.h"
#include "ScriptingWindow.h"
#include "initializeScriptingWindow.h"
#include "afxwin.h"

IMPLEMENT_DYNAMIC(ScriptingWindow, CDialog)

BEGIN_MESSAGE_MAP(ScriptingWindow, CDialog)

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