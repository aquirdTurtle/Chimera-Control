#include "stdafx.h"
#include "postMyString.h"
#include <string>

int postMyString(unsigned int messageTypeID, std::string message, HWND windowHandle)
{
	// The window recieving this message is responsible for deleting this pointer.
	char* messageChars = new char[message.size() + 1];
	sprintf_s(messageChars, message.size() + 1, "%s", message.c_str());
	PostMessage(windowHandle, messageTypeID, 0, (LPARAM)messageChars);
	return 0;
}
