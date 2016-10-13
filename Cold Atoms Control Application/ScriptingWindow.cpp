#include "stdafx.h"
#include "ScriptingWindow.h"
#include "initializeScriptingWindow.h"
#include "afxwin.h"

IMPLEMENT_DYNAMIC(ScriptingWindow, CDialog)

BEGIN_MESSAGE_MAP(ScriptingWindow, CDialog)
	ON_WM_CTLCOLOR()
	ON_EN_CHANGE(0, &ScriptingWindow::horizontalEditChange)
	ON_EN_CHANGE(0, &ScriptingWindow::agilentEditChange)
	ON_EN_CHANGE(0, &ScriptingWindow::agilentEditChange)
END_MESSAGE_MAP()

BOOL ScriptingWindow::OnInitDialog()
{
	// ADD MORE INITIALIZATIONS HERE
	int id = 101234;
	POINT startLocaiton = { 0, 28 };
	verticalNIAWGScript.initializeControls(640, 1000, startLocaiton, eScriptingWindowHandle, "Vertical NIAWG", id);
	startLocaiton = { 640, 28 };
	horizontalNIAWGScript.initializeControls(640, 1000, startLocaiton, eScriptingWindowHandle, "Horizontal NIAWG", id);
	startLocaiton = { 1280, 28 };
	intensityAgilentScript.initializeControls(640, 1000, startLocaiton, eScriptingWindowHandle, "Agilent", id);

	// legacy... to be removed soon.
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

void ScriptingWindow::horizontalEditChange()
{
	
	return;
}

void ScriptingWindow::agilentEditChange()
{
	return;
}

void ScriptingWindow::verticalEditChange()
{
	return;
}
