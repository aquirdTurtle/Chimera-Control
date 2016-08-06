// win32ToMFC.cpp : Defines the entry point for the application.
#include "stdafx.h"
#include "resource.h"
#include <afxwin.h>
struct CMyWinApp : public CWinApp
{
	BOOL InitInstance()
	{
		CDialog window(IDD_DIALOG1);
		window.DoModal();
		return TRUE;
	}
};
CMyWinApp theApp;
