#include "stdafx.h"
#include "errBox.h"
#include "Windows.h"

void errBox(std::string msg)
{
	MessageBox(0, msg.c_str(), "ERROR!", MB_ICONERROR);
}
