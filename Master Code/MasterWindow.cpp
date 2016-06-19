#include "stdafx.h"
#include "MasterWindow.h"
#include "Control.h"
//#include "newVariableDialogProcedure.h"
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
	MasterWindow * NewMasterWindow = new MasterWindow(9000);

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
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLORBTN:
		{
			INT_PTR result = this->ttlBoard.colorTTLs(hwnd, message, wParam, lParam, this);
			if (result != NULL)
			{
				return result;
			}
			break;
		}
		case WM_NOTIFY:
		{
			int notifyMessage = ((LPNMHDR)lParam)->code;
			switch (notifyMessage)
			{
				case NM_DBLCLK:
				{
					std::vector<Script*> scriptList;
					scriptList.push_back(&masterScript);
					this->variables.updateVariableInfo(lParam, scriptList, this);
					break;
				}
				case NM_RCLICK:
				{
					this->variables.deleteVariable(lParam);
					break;
				}
			}
			break;
		}
		case WM_DESTROY:
		{
			MasterWindow * pMasterWindow = new MasterWindow(9000);
			pMasterWindow = (MasterWindow *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if (pMasterWindow)
			{
				pMasterWindow->masterWindowHandle = 0;
			}
			break;
		}
		case WM_TIMER:
		{
			if (this->masterScript.coloringIsNeeded())
			{
				this->masterScript.handleTimerCall(*this);
			}
			break;
		}
		/// Handle Everything Else...///////////////////////
		case WM_COMMAND:
		{
			if (!this->masterScript.handleEditChange(wParam, lParam, hwnd))
			{
				break;
			}
			else if (!this->masterScript.childComboChangeHandler(wParam, lParam, *this))
			{
				break;
			}
			if (!ttlBoard.handleTTLPress(wParam, lParam))
			{
				break;
			}
			if (!ttlBoard.handleHoldPress(wParam, lParam))
			{
				break;
			}
			switch (LOWORD(wParam))
			{
				
			}
		}
		default:
		{
			return DefWindowProc(hwnd, message, wParam, lParam);
		}
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

int MasterWindow::InitializeWindowControls()
{
	POINT controlLocation{ 0, 0 };
	profile.initialize(controlLocation, *this);
	ttlBoard.initialize(controlLocation, masterWindowHandle);
	dacBoards.initialize(controlLocation, masterWindowHandle);
	variables.initialize(controlLocation, masterWindowHandle);
	controlLocation = POINT{ 480, 90 };
	notes.initializeControls(controlLocation, masterWindowHandle);
	controlLocation = POINT{ 1320, 0 };
	masterScript.initialize(600, 1080, controlLocation, *this);
	return 0;
}