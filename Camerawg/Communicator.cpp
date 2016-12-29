
#include "stdafx.h"
#include "Communicator.h"
#include "postMyString.h"
#include "CameraWindow.h"

void Communicator::initialize(MainWindow* comm, ScriptingWindow* scriptingWin, CameraWindow* cameraWin)
{
	this->mainWin = comm;
	this->scriptWin = scriptingWin;
	this->camWin = cameraWin;
	return;
}

/*
Note that in all of the following, using "" as the input means that the communicaotr does not send any message to the
control of interest.
*/

void Communicator::sendCameraFin()
{
	PostMessage( mainWin->GetSafeHwnd(), eCameraFinishMessageID, 0, 0 );
}

void Communicator::sendCameraProgress(long progress)
{
	PostMessage( mainWin->GetSafeHwnd(), eCameraProgressMessageID, 0, (LPARAM)progress );
}

void Communicator::sendTimer( std::string timerMsg )
{
	if ( timerMsg != "" )
	{
		this->camWin->setTimerText( timerMsg );
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
