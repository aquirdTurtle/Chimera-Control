
#include "stdafx.h"
#include "Communicator.h"
#include "CameraWindow.h"

void Communicator::initialize(MainWindow* mainWinParent, ScriptingWindow* scriptingWin, CameraWindow* cameraWin)
{
	mainWin = mainWinParent;
	scriptWin = scriptingWin;
	camWin = cameraWin;
}

/*
	Note that in all of the following, using "" as the input means that the communicaotr does not send any message to the
	control of interest.
*/

// the two camera messages go straight to the camera window.
void Communicator::sendCameraFin()
{
	PostMessage( camWin->GetSafeHwnd(), eCameraFinishMessageID, 0, 0 );
}

void Communicator::sendCameraProgress(long progress)
{
	PostMessage( camWin->GetSafeHwnd(), eCameraProgressMessageID, 0, (LPARAM)progress );
}

void Communicator::sendTimer( std::string timerMsg )
{
	if ( timerMsg != "" )
	{
		camWin->setTimerText( timerMsg );
	}
}

/*
 * Don't use this function directly, it was designed to be used with the macro in the header file.
 */
void Communicator::sendErrorEx( std::string statusMsg, const char *file, int line)
{	
	statusMsg = statusMsg + "\r\n(Sent by file: " + std::string( file ) + " on line: "
		+ std::to_string( line ) + ")\r\n*********************************\r\n";
	postMyString( mainWin, eErrorTextMessageID, statusMsg );
}

/*
* Don't use this function directly, it was designed to be used with the macro in the header file.
*/
void Communicator::sendFatalErrorEx( std::string statusMsg, const char *file, int line)
{
	statusMsg = statusMsg + "\r\n(Sent by file: " + std::string( file ) + " on line: "
		+ std::to_string( line ) + ")\r\n*********************************\r\n";
	postMyString( mainWin, eFatalErrorMessageID, statusMsg );
}

void Communicator::sendColorBox( colorBoxes<char> colors )
{
	mainWin->changeBoxColor( colors );
	scriptWin->changeBoxColor( colors );
	camWin->changeBoxColor( colors );
}

void Communicator::sendStatus(std::string statusMsg)
{
	postMyString(mainWin, eStatusTextMessageID, statusMsg);
	mainWin->setShortStatus( statusMsg );
}

void Communicator::sendDebug(std::string statusMsg)
{
	if (statusMsg != "")
	{
		postMyString(mainWin, eDebugMessageID, statusMsg);
	}
}


void Communicator::postMyString( CWnd* window, unsigned int messageTypeID, std::string message )
{
	// The window recieving this message is responsible for deleting this pointer.
	char* messageChars = new char[message.size() + 1];
	sprintf_s( messageChars, message.size() + 1, "%s", message.c_str() );
	window->PostMessageA( messageTypeID, 0, (LPARAM)messageChars );
}
