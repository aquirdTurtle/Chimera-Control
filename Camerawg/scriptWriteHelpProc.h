#pragma once

#include "stdafx.h"
#include "resource.h"
#include "windows.h"

/*
 * The simple procedure for the help window. Only handles cancels and closes.
 */
static BOOL CALLBACK scriptWriteHelpProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDCANCEL:
					SendMessage(hwnd, WM_CLOSE, 0, 0);
					return TRUE;
			}
			break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
			return TRUE;
	
		case WM_DESTROY:
			return TRUE;
	}
	return FALSE;
}
