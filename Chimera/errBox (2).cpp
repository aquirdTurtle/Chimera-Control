#include "stdafx.h"
#include "errBox.h"

void errBox(std::string errMsg)
{
	MessageBox(0, errMsg.c_str(), "ERROR!", MB_ICONERROR);
}
