// Win32Project2.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
// required for all Windows applications
#include <windows.h>
// Andor functions
#include "atmcd32d.h"
#include <stdio.h>
#include "cameraWndProc.h"
#include "constants.h"
#include "externals.h"
#include <string>
#include <vector>
#include "Resource.h"
#include "dialogProcedures.h"
#include "myAndor.h"
#include "postMyString.h"
#include "Richedit.h"
#include "initializationUpdate.h"
#include "appendColoredText.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	char aBuffer[256];
	// Look in current working directory for driver files
	GetCurrentDirectory(256, aBuffer);

	eHInst = hInstance;

	float speed = 0, STemp = 0;
	int iSpeed = 0, iAD = 0, nAD = 0, index = 0;
	MSG msg;

	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = cameraWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = (HICON)LoadIconA(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = CreateSolidBrush(RGB(54, 70, 54));
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wcex.lpszClassName = (LPCSTR)"Camera Class";
	wcex.hIconSm = (HICON)LoadIconA(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	if (!RegisterClassEx(&wcex))
	{
		int a = GetLastError();
		MessageBox(0, ("Register Class Failed: " + std::to_string(a)).c_str(), 0, 0);
		return -1;
	}
	
	eInitializeDialogBoxHandle = CreateDialog(eHInst, MAKEINTRESOURCE(IDD_INITIALIZE_DIALOG_BOX), eCameraWindowHandle, 
											 (DLGPROC)dialogProcedures::richEditMessageBoxProc);

	CHARFORMAT2 defaultCharFormat, redCharFormat, greenCharFormat;
	memset(&redCharFormat, 0, sizeof(CHARFORMAT));
	redCharFormat.cbSize = sizeof(CHARFORMAT);
	redCharFormat.dwMask = CFM_COLOR;
	redCharFormat.crTextColor = RGB(255, 0, 0);

	memset(&greenCharFormat, 0, sizeof(CHARFORMAT));
	greenCharFormat.cbSize = sizeof(CHARFORMAT);
	greenCharFormat.dwMask = CFM_COLOR;
	greenCharFormat.crTextColor = RGB(0, 150, 0);

	memset(&defaultCharFormat, 0, sizeof(CHARFORMAT));
	defaultCharFormat.cbSize = sizeof(CHARFORMAT);
	defaultCharFormat.dwMask = CFM_COLOR;
	defaultCharFormat.crTextColor = RGB(0, 0, 0);

	appendColoredText("INITIALIZATION MESSAGES:\r\n", eRichEditMessageBoxRichEditHandle, IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID, defaultCharFormat,
					  eInitializeDialogBoxHandle);

	std::string errorMessage;
	if (ANDOR_SAFEMODE)
	{
		errorMessage = "SAFEMODE: Not attempted.";
	}
	if (!ANDOR_SAFEMODE)
	{
		// Initialize driver in current directory
		errorMessage = myAndor::andorErrorChecker(Initialize(aBuffer));
	}
	appendColoredText("Initializing......................... ", eRichEditMessageBoxRichEditHandle, IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID, defaultCharFormat, 
					  eInitializeDialogBoxHandle);
	initializationUpdate(errorMessage, defaultCharFormat, redCharFormat, greenCharFormat);

	if (!ANDOR_SAFEMODE)
	{
		// Get camera eCameraCapabilities
		errorMessage = myAndor::andorErrorChecker(GetCapabilities(&eCameraCapabilities));
	}
	//eCameraCapabilities.
	appendColoredText("Get Andor Capabilities information... ", eRichEditMessageBoxRichEditHandle, IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID, defaultCharFormat, 
					  eInitializeDialogBoxHandle);
	initializationUpdate(errorMessage, defaultCharFormat, redCharFormat, greenCharFormat);

	if (!ANDOR_SAFEMODE)
	{
		// Get Head Model
		errorMessage = myAndor::andorErrorChecker(GetHeadModel(eModel));
	}
	appendColoredText("Get Head Model information........... ", eRichEditMessageBoxRichEditHandle, IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID, defaultCharFormat,
					  eInitializeDialogBoxHandle);
	initializationUpdate(errorMessage, defaultCharFormat, redCharFormat, greenCharFormat);

	if (!ANDOR_SAFEMODE)
	{
		// Get detector information
		errorMessage = myAndor::andorErrorChecker(GetDetector(&eXPixels, &eYPixels));
	}
	else 
	{
		eXPixels = 512;
		eYPixels = 512;
	}
	appendColoredText("Get Detector information............. ", eRichEditMessageBoxRichEditHandle, IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID, defaultCharFormat,
					  eInitializeDialogBoxHandle);
	initializationUpdate(errorMessage, defaultCharFormat, redCharFormat, greenCharFormat);

	if (!ANDOR_SAFEMODE)
	{
		// Set Vertical speed to recommended
		GetFastestRecommendedVSSpeed(&eVerticalSpeedNumber, &speed);
		errorMessage = myAndor::andorErrorChecker(SetVSSpeed(eVerticalSpeedNumber));
	}
	appendColoredText("Set Vertical Speed................... ", eRichEditMessageBoxRichEditHandle, IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID, defaultCharFormat,
					  eInitializeDialogBoxHandle);
	initializationUpdate(errorMessage, defaultCharFormat, redCharFormat, greenCharFormat);

	// Set Horizontal Speed to max
	STemp = 0;
	eHorizontalSpeedNumber = 0;
	eADNumber = 0;
	if (!ANDOR_SAFEMODE)
	{
		errorMessage = myAndor::andorErrorChecker(GetNumberADChannels(&nAD));
	}
	appendColoredText("Get number AD Channel................ ", eRichEditMessageBoxRichEditHandle, IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID, defaultCharFormat,
					  eInitializeDialogBoxHandle);
	initializationUpdate(errorMessage, defaultCharFormat, redCharFormat, greenCharFormat);
	if (!ANDOR_SAFEMODE)
	{
		errorMessage = myAndor::andorErrorChecker(SetBaselineClamp(1));
	}
	appendColoredText("Set Baseline Clamp................... ", eRichEditMessageBoxRichEditHandle, IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID, defaultCharFormat,
		eInitializeDialogBoxHandle);
	initializationUpdate(errorMessage, defaultCharFormat, redCharFormat, greenCharFormat);
	if (!ANDOR_SAFEMODE)
	{
		errorMessage = myAndor::andorErrorChecker(SetBaselineOffset(0));
	}
	appendColoredText("Set Baseline Offset Value............ ", eRichEditMessageBoxRichEditHandle, IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID, defaultCharFormat,
		eInitializeDialogBoxHandle);
	initializationUpdate(errorMessage, defaultCharFormat, redCharFormat, greenCharFormat);

	if (!ANDOR_SAFEMODE)
	{
		// Setting this makes the camera ALWAYS send an image as soon as it receives it instead of waiting a couple images before notifying the computer.
		// It does this wait if you don't set this and the images are taken very fast, e.g. < 15 ms.
		errorMessage = myAndor::andorErrorChecker(SetDMAParameters(1, 0.0001));
	}
	appendColoredText("Set DMA Parameters................... ", eRichEditMessageBoxRichEditHandle, IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID, defaultCharFormat,
					   eInitializeDialogBoxHandle);
	initializationUpdate(errorMessage, defaultCharFormat, redCharFormat, greenCharFormat);

	if (!ePlotter.is_open())
	{
		errorMessage = "GNUPLOT didn't open correctly.";
	}
	else
	{
		errorMessage = "GOOD";
	}
	appendColoredText("Opening GNUPLOT...................... ", eRichEditMessageBoxRichEditHandle, IDC_RICH_EDIT_MESSAGE_BOX_RICH_EDIT_ID, defaultCharFormat,
		eInitializeDialogBoxHandle);
	initializationUpdate(errorMessage, defaultCharFormat, redCharFormat, greenCharFormat);
	//Create the event to be used by the Andor SDK library
	//HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	//Pass the event to the SDK
	//unsigned int errorvalue = SetDriverEvent(hEvent);

	/*
	if (errorvalue != DRV_SUCCESS) 
	{
	appendText("ERROR: Set Driver Event Error: " + std::to_string(errorvalue) + "\r\n", IDC_ERROR_EDIT);
	UpdateWindow(eStatusEditHandle);
	ExitThread(1);
	}
	*/

	// Wait for 2 seconds to allow MCD to calibrate fully before allowing an
	// acquisition to begin
	Sleep(2000);

	eCameraWindowHandle = CreateWindowEx(0, (LPCSTR)"Camera Class", (LPCSTR)"Camera Control",
		WS_MAXIMIZE | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
		0, 0, 1680, 1010, NULL, NULL, hInstance, NULL);
	// Show the window
	ShowWindow(eCameraWindowHandle, SW_MAXIMIZE);
	ShowWindow(eInitializeDialogBoxHandle, SW_SHOW);
	UpdateWindow(eCameraWindowHandle);						// Sends WM_PAINT message
	/*	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAcceleratorA(eScriptingWindowHandle, hAccel, &msg) && !TranslateAccelerator(eCameraWindowHandle, hAccel, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	*/
	HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));


	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(eCameraWindowHandle, hAccel, &msg) && !IsDialogMessage(eInitializeDialogBoxHandle, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return 0;
}