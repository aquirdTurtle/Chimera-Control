#pragma once

#include "stdafx.h"
class myApplicationApp : CWinApp
{
public:
	myApplicationApp() : CWinApp(), theMainApplicationWindow(IDD_LARGE_TEMPLATE){	}
	BOOL InitInstance();
	BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);
private:
	HACCEL m_haccel;
	MainWindow theMainApplicationWindow;
	//ScriptingWindow ScriptWin;
};
