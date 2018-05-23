#pragma once

#include "stdafx.h"
#include "resource.h"
#include "MainWindow.h"
#include "splashDialog.h"
#include <chrono>
#include "commonTypes.h"

chronoTime initTime;

// making it derived from CWinAppEx gives extra functionality over CWinApp, like extra tooltip functionality :D
class ChimeraApp : CWinAppEx
	{
	public:
		ChimeraApp() : CWinAppEx(), splash(new splashDialog), theMainApplicationWindow(IDD_LARGE_TEMPLATE, splash, &initTime){	}
		BOOL InitInstance( );
		BOOL ExitInstance( );
		BOOL PreTranslateMessage(MSG* pMsg);
		virtual BOOL ProcessMessageFilter(int code, LPMSG lpMsg);
	private:
		ULONG_PTR gdip_token;
		HACCEL m_haccel;
		splashDialog* splash;
		MainWindow theMainApplicationWindow;
};