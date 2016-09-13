// AUTHOR: MARK OVERTON BROWN
// Created: 4/21/2016

#include "stdafx.h"
#include "Master Code.h"
#include "MasterWindow.h"
#include "afxsock.h"

class CMyWinApp : CWinApp
{
	private:
		BOOL InitInstance() override
		{
			MessageBox(0, "RUNNING!", 0, 0);
			return TRUE;
			// initialize sockets.
			AfxSocketInit();
			// create the main window.
			MasterWindow master(IDD_DIALOG1);
			master.DoModal();
			return TRUE;
		}
};

CMyWinApp theApp;
