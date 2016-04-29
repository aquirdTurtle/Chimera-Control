#include "stdafx.h"
#include "appendText.h"
#include "externals.h"
#include <string>
void appendText(std::string newText, int textID, HWND windowHandle)
{
	HWND hWndOutput = GetDlgItem(windowHandle, textID);
	int index = GetWindowTextLength(hWndOutput);
	// set selection to end of text
	SendMessage(hWndOutput, EM_SETSEL, (WPARAM)index, (LPARAM)index); 
	TCHAR* TCHARnewText;
	TCHARnewText = (TCHAR*) newText.c_str();
	// append!
	SendMessage(hWndOutput, EM_REPLACESEL, 0, (LPARAM)TCHARnewText); 
	return;
}