#pragma once

#include "stdafx.h"

#include <string>
/*
* This function gets the name of a file to open, using the fancy windows dialog box.
* It returns the chosen filename if successful, it returns "" if the user canceled.
*/
std::string getFileNameDialog(HWND hwnd);

