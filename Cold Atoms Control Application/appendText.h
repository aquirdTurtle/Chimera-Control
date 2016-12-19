#pragma once
#include "stdafx.h"
#include <Windows.h>
/*
* This functions appends the text "newText" to the edit control corresponding to textIDC.
*/
//void appendText(std::string newText, int textIDC, HWND parentWindow);
void appendText(std::string newText, CEdit& edit);
void appendText(std::string newText, Control<CRichEditCtrl>& edit);
