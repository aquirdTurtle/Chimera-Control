// AUTHOR: MARK OVERTON BROWN
// Created: 4/21/2016

#include "stdafx.h"
#include "Master Code.h"
#include "MasterWindow.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// structure that contains all the fundamental information for the main window.	
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MASTERCODE));

    MSG msg;
	MasterWindow * pMasterWindow = MasterWindow::InitializeWindowInfo(hInstance);
	if (pMasterWindow == NULL)
	{
		return 0;
	}
    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


