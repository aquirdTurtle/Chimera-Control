#pragma once

#include "stdafx.h"
#include "resource.h"
#include "MainWindow.h"
#include "splashDialog.h"

// making it derived from CWinAppEx gives extra functionality over CWinApp, like extra tooltip functionality :D
class ChimeraApp : CWinAppEx
	{
	public:
		ChimeraApp() : CWinAppEx(), splash(new splashDialog), theMainApplicationWindow(IDD_LARGE_TEMPLATE, splash){	}
		BOOL InitInstance();
		BOOL PreTranslateMessage(MSG* pMsg);
		virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);
	private:
		HACCEL m_haccel;
		splashDialog* splash;
		MainWindow theMainApplicationWindow;
};