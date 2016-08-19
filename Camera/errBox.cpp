#include "stdafx.h"
#include "errBox.h"

// automatically give the error icon and window and accepts a string. Just a bit nicer, esp. when wanting to add strings for the argument. 
void errBox(std::string msg)
{
	MessageBox(0, msg.c_str(), "ERROR!", MB_ICONERROR);
	return;
}
