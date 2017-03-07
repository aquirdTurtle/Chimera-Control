// AUTHOR: MARK OVERTON BROWN
// Created: 4/21/2016

#include "stdafx.h"
#include "Master Code.h"
#include "MasterWindow.h"
#include "afxsock.h"

// the application class definition.
class CMyWinApp : CWinApp
{
	public:
		CMyWinApp::CMyWinApp() : CWinApp(), TheMasterWindow( IDD_DIALOG1 ) {}

	private:
		MasterWindow TheMasterWindow;

		BOOL InitInstance() override
		{
			// initialize sockets
			AfxSocketInit();
			// create the main window.
			TheMasterWindow.DoModal();
			return TRUE;
		}

		BOOL PreTranslateMessage( MSG* pMsg )
		{
			if (pMsg->message == WM_KEYDOWN)
			{
				if (pMsg->wParam == VK_ESCAPE)
				{
					this->TheMasterWindow.HandleAbort();
					// Do not process further
					return TRUE;
				}
				if ( pMsg->wParam == VK_F1 )
				{
					this->TheMasterWindow.loadMotSettings();
					// Do not process further
					return TRUE;
				}
				if (pMsg->wParam == VK_F5)
				{
					this->TheMasterWindow.StartExperiment();
					// do not process further.
					return TRUE;
				} 
			}
			return CWinApp::PreTranslateMessage( pMsg );
		}
};

// the entire program happens in the constructor for this object.
CMyWinApp theApp;

