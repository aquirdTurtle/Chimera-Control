#pragma once
#include "stdafx.h"

struct Control
{
	HWND hwnd;
	RECT normalPos;
	int ID;
	int colorState = 0;
};