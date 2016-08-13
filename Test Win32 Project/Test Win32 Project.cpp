// Test Win32 Project.cpp : Defines the entry point for the application.
//

//#pragma comment(lib, "comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "stdafx.h"
#include "resource.h"
#include "Test Win32 Project.h"
//#include "Windows.h"
#include "Commctrl.h"


#define MAX_LOADSTRING 100
#define ID_TOOLTIP 101

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
HWND				CreateToolTip(int toolID, HWND hDlg, HINSTANCE hInst, PTSTR pszText);

class MyDialog : public CDialog  
{
	using CDialog::CDialog;
	public:
		afx_msg void MyDialog::handleOK();
};

MyDialog mainWindow(IDD_DIALOG1);

#include <iostream>
//#include <afx.h>
#include <string>
CButton mfcButton;
CToolTipCtrl mfcToolTip;

HWND Button, tooltip_mess;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{

	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		MessageBox(0, "Fatal Error: MFC initialization failed", 0, 0);
		return 1;
	}
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
	return mainWindow.DoModal();
	//int test = mainWindow.Create(IDD_DIALOG1);
	//int test2 = mainWindow.ShowWindow(SW_SHOW);
	/*
	INITCOMMONCONTROLSEX icc;

	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC = ICC_BAR_CLASSES | ICC_TAB_CLASSES | ICC_WIN95_CLASSES;

	//InitCommonControlsEx(&icc);
	InitCommonControls();
	*/
    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TESTWIN32PROJECT, szWindowClass, MAX_LOADSTRING);
    //MyRegisterClass(hInstance);

    // Perform application initialization:
    /*
	if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }
	*/

    //HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TESTWIN32PROJECT));

    //MSG msg;

    // Main message loop:
	/*
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
	*/
}

afx_msg void MyDialog::handleOK()
{

	return;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TESTWIN32PROJECT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TESTWIN32PROJECT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
		case WM_CREATE:
		{
			Button = CreateWindowEx(0, "BUTTON", "My Button", WS_VISIBLE | WS_CHILD, 10, 10, 100, 24, hWnd, (HMENU)ID_TOOLTIP, hInst, NULL);
			tooltip_mess = CreateToolTip(ID_TOOLTIP, hWnd, hInst, "Tooltip message");
			mfcButton.Create("MFC", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, { 10,34,110,58 }, CWnd::FromHandle(hWnd), 10);
			if (mfcToolTip.Create(CWnd::FromHandle(hWnd)) == 0)
			{
				MessageBox(0, "ERROR", 0, 0);
			}
			if (mfcToolTip.AddTool(CWnd::FromHandle(hWnd), "About Box") == 0)
			{
				MessageBox(0, "ERROR", 0, 0);
			}
			//CWnd* pWnd = GetDlgItem(10);

			if (mfcToolTip.AddTool(&mfcButton, "About Box") == 0)
			{
				MessageBox(0, "ERROR", 0, 0);
			}
			mfcToolTip.Activate(TRUE);

			if (tooltip_mess)
			{
				SendMessage(tooltip_mess, TTM_ACTIVATE, TRUE, 0);
			}
			else
			{
				MessageBox(0, "FAILED", 0, 0);
			}
		}
		case WM_COMMAND:
			{
				int wmId = LOWORD(wParam);
				// Parse the menu selections:
				switch (wmId)
				{
				case IDM_ABOUT:
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
					break;
				case IDM_EXIT:
					DestroyWindow(hWnd);
					break;
				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
				}
			}
			break;
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				// TODO: Add any drawing code that uses hdc here...
				EndPaint(hWnd, &ps);
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

HWND CreateToolTip(int toolID, HWND hDlg, HINSTANCE hInst, PTSTR pszText)
{
	if (!toolID || !hDlg || !pszText)
	{
		return NULL;
	}

	// Get the window of the tool.
	HWND hwndTool = GetDlgItem(hDlg, toolID);
	if (!hwndTool)
	{
		return NULL;
	}

	// Create the tooltip. g_hInst is the global instance handle.
	HWND hwndTip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
		WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		hDlg, NULL,
		hInst, NULL);

	if (!hwndTip)
	{
		return NULL;
	}

	// Associate the tooltip with the tool.
	TOOLINFO toolInfo = { 0 };
	toolInfo.cbSize = sizeof(toolInfo) - sizeof(void*);;
	toolInfo.hwnd = hDlg;
	toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	toolInfo.uId = (UINT_PTR)hwndTool;
	toolInfo.lpszText = pszText;
	if (!SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo))
	{
		int a = GetLastError();
		MessageBox(0, (std::to_string(a).c_str()), 0, 0);
		DestroyWindow(hwndTip);
		return NULL;
	}
	TOOLINFO toolTest = { 0 };
	toolTest.cbSize = sizeof(toolInfo) - sizeof(void*);;
	toolTest.hwnd = hDlg;
	toolTest.uId = (UINT_PTR)hwndTool;
	SendMessage(hwndTip, TTM_GETTOOLINFO, 0, (LPARAM)&toolTest);


	return hwndTip;
}
