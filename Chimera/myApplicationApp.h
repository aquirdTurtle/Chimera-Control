#pragma once

#include "stdafx.h"

// making it derived from CWinAppEx gives extra functionality over CWinApp, like extra tooltip functionality :D
class myApplicationApp : CWinAppEx
	{
	public:
		myApplicationApp() : CWinAppEx(), theMainApplicationWindow(IDD_LARGE_TEMPLATE){	}
		BOOL InitInstance();
		BOOL PreTranslateMessage(MSG* pMsg);
		virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);
	private:
		HACCEL m_haccel;
		MainWindow theMainApplicationWindow;
};
