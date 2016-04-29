#include "stdafx.h"
#include "postMyString.h"

int postMyString(unsigned int messageTypeID, std::string message)
{
	// The window recieving this message is responsible for deleting this pointer.
	char* messageChars = new char[message.size() + 1];
	sprintf_s(messageChars, message.size() + 1, "%s", message.c_str());
	PostMessage(eMainWindowHandle, messageTypeID, 0, (LPARAM)messageChars);
	return 0;
}
