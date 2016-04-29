#include "stdafx.h"
#include "MasterWindow.h"
#include "Control.h"
#include "newVariableDialogProcedure.h"
#include "constants.h"
MasterWindow* MasterWindow::InitializeWindowInfo(HINSTANCE hInstance)
{
	//cHInstance = hInstance;
	WNDCLASSEX masterWindowInfo;
	ZeroMemory(&masterWindowInfo, sizeof(WNDCLASSEX));
	if (!GetClassInfoEx(hInstance, "Master Window Class", &masterWindowInfo))
	{
		masterWindowInfo.cbSize = sizeof(WNDCLASSEX);
		masterWindowInfo.style = CS_HREDRAW | CS_VREDRAW;
		masterWindowInfo.hInstance = hInstance;
		masterWindowInfo.lpszClassName = "Master Window Class";
		masterWindowInfo.lpfnWndProc = MasterWindowProcedure;
		masterWindowInfo.cbClsExtra = 0;
		masterWindowInfo.cbWndExtra = 0;

		masterWindowInfo.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ATOM_ICON));
		masterWindowInfo.hIconSm = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ATOM_ICON), IMAGE_ICON, 32, 32, 0);
		masterWindowInfo.hCursor = LoadCursor(NULL, IDC_ARROW);;
		masterWindowInfo.hbrBackground = CreateSolidBrush(RGB(10, 10, 10));
		masterWindowInfo.lpszMenuName = MAKEINTRESOURCE(IDC_MASTERCODE);
		if (RegisterClassEx(&masterWindowInfo) == 0)
		{
			int errVal = GetLastError();
			MessageBox(0, ("The master window class failed to register. Error: " + std::to_string(errVal)).c_str(), "Error", 0);
			return 0;
		}
	}
	// create a pointer to a new object
	MasterWindow * NewMasterWindow = new MasterWindow;

	HWND newMasterWindowHandle = CreateWindowEx(0, "Master Window Class", "Cold Atoms Control: Master Window",
												WS_MAXIMIZE | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
												0, 0, 1920, 1000, NULL, NULL, hInstance, NewMasterWindow);
	if (newMasterWindowHandle == NULL)
	{
		delete NewMasterWindow;
		MessageBox(NULL, "Problem creating the window at CreateWindowEx.", "Error", 0);
		return 0;
	}
	ShowWindow(newMasterWindowHandle, SW_MAXIMIZE);
	return NewMasterWindow;
}

MasterWindow::~MasterWindow()
{
	if (masterWindowHandle)
	{
		DestroyWindow(masterWindowHandle);
	}
}

LRESULT MasterWindow::MasterWindowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) 
	{
		MasterWindow * pMasterWindow;
		case WM_NCCREATE: 
		{
			CREATESTRUCT * pcs = (CREATESTRUCT*)lParam;
			pMasterWindow = (MasterWindow*)pcs->lpCreateParams;
			pMasterWindow->masterWindowHandle = hwnd;
			SetLastError(0);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pcs->lpCreateParams);
			return TRUE;
		}
		default:
		{
			pMasterWindow = (MasterWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (pMasterWindow)
			{
				return pMasterWindow->MasterWindowMessageHandler(hwnd, uMsg, wParam, lParam);
			}
			else
			{
				return DefWindowProc(hwnd, uMsg, wParam, lParam);
			}
		}
	}
}

LRESULT MasterWindow::MasterWindowMessageHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
		{
			InitializeWindowControls();
			break;
		}
		case WM_DESTROY:
		{
			MasterWindow * pMasterWindow = new MasterWindow;
			pMasterWindow = (MasterWindow *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (pMasterWindow)
			{
				pMasterWindow->masterWindowHandle = 0;
			}
			break;
		}
		case ID_VARIABLE_ADD:
		{
			DialogBox(0, MAKEINTRESOURCE(IDD_NEW_VARIABLE), hwnd, newVariableDialogProcedure);
			//result = (TCHAR*)DialogBox(eGlobalInstance, MAKEINTRESOURCE(IDD_FOLDER_NAME_PROMPT), parentWindow, namePromptDialogProc);
			break;
		}
		default:
		{
			return DefWindowProc(hwnd, message, wParam, lParam);
		}
	}
}


int MasterWindow::InitializeWindowControls()
{
	POINT currentLocation{ 0, 0 };
	currentLocation.y = ttlBoard.CreateTTLControls(currentLocation, masterWindowHandle);
	currentLocation.y = dacBoards.InitializeDACControls(currentLocation, masterWindowHandle);
	currentLocation.y = variables.initializeVariableControls(currentLocation, masterWindowHandle);
	return 0;
}