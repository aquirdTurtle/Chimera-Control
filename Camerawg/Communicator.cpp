
#include "stdafx.h"
#include "Communicator.h"
#include "postMyString.h"
#include "CameraWindow.h"

void Communicator::initialize(MainWindow* comm, ScriptingWindow* scriptingWin, CameraWindow* cameraWin)
{
	this->parent = comm;
	this->scriptWin = scriptingWin;
	this->camWin = cameraWin;
	return;
}

/*
Note that in all of the following, using "" as the input means that the communicaotr does not send any message to the
control of interest.
*/

void Communicator::sendError(std::string statusMsg, std::string shortMsg, std::string color)
{
	if (statusMsg != "")
	{
		postMyString(parent, eErrorTextMessageID, statusMsg);
	}
	if (shortMsg != "")
	{
		parent->setShortStatus(shortMsg);
	}
	if (color != "")
	{
		parent->changeShortStatusColor(color);
		scriptWin->changeBoxColor(color);
		camWin->changeBoxColor(color);
	}
}

void Communicator::sendFatalError(std::string statusMsg, std::string shortMsg, std::string color)
{
	if (statusMsg != "")
	{
		postMyString(parent, eFatalErrorMessageID, statusMsg);
	}
	if (shortMsg != "")
	{
		parent->setShortStatus(shortMsg);
	}
	if (color != "")
	{
		parent->changeShortStatusColor(color);
		scriptWin->changeBoxColor(color);
		camWin->changeBoxColor(color);
	}
}

void Communicator::sendStatus(std::string statusMsg, std::string shortMsg, std::string color)
{
	if (statusMsg != "")
	{
		postMyString(parent, eStatusTextMessageID, statusMsg);
	}
	if (shortMsg != "")
	{
		parent->setShortStatus(shortMsg);
	}
	if (color != "")
	{
		parent->changeShortStatusColor(color);
		scriptWin->changeBoxColor(color);
		camWin->changeBoxColor(color);
	}
}

void Communicator::sendDebug(std::string statusMsg, std::string shortMsg, std::string color)
{
	if (statusMsg != "")
	{
		postMyString(parent, eDebugMessageID, statusMsg);
	}
	if (shortMsg != "")
	{
		parent->setShortStatus(shortMsg);
	}
	if (color != "")
	{
		parent->changeShortStatusColor(color);
		scriptWin->changeBoxColor(color);
		camWin->changeBoxColor(color);
	}
}