
#include "stdafx.h"
#include "Communicator.h"
#include "CameraWindow.h"
#include "AuxiliaryWindow.h"


// pass all the windows so that the object can (in principle) send messages to any window.
void Communicator::initialize(MainWindow* mainWinParent, ScriptingWindow* scriptingWin, CameraWindow* cameraWin, 
							  AuxiliaryWindow* auxWindow )
{
	mainWin = mainWinParent;
	scriptWin = scriptingWin;
	camWin = cameraWin;
	auxWin = auxWindow;
}


void Communicator::sendNoAtomsAlert( )
{
	mainWin->PostMessageA( eNoAtomsAlertMessageID, 0, 0 );
}


// the two camera messages go straight to the camera window.
void Communicator::sendCameraFin()
{
	camWin->PostMessage( eCameraFinishMessageID, 0, 0 );
}

void Communicator::sendNormalFinish( )
{
	mainWin->PostMessageA( eNormalFinishMessageID );
}

void Communicator::sendCameraProgress(long progress)
{
	camWin->PostMessageA( eCameraProgressMessageID, 0, (LPARAM)progress );
}

void Communicator::sendRepProgress(ULONG rep)
{
	mainWin->PostMessageA(eRepProgressMessageID, 0, LPARAM(rep));
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
	postMyString( mainWin, eErrorTextMessageID, statusMsg );
}

/*
* Don't use this function directly, it was designed to be used with the macro in the header file.
*/
void Communicator::sendFatalErrorEx( std::string statusMsg, const char *file, int line)
{
	postMyString( mainWin, eFatalErrorMessageID, statusMsg );
}


void Communicator::sendColorBox( System sys, char code )
{
	systemInfo<char> colors;
	switch (sys)
	{
		case Niawg:
			colors = { code, '-', '-','-' };
			break;
		case Camera:
			colors = { '-', code, '-', '-' };
			break;
		case Intensity:
			colors = { '-', '-', code, '-' };
			break;
		case Master:
			colors = { '-', '-', '-', code };
			break;
	}
	sendColorBox( colors );
}


void Communicator::sendColorBox( systemInfo<char> colors )
{
	mainWin->changeBoxColor( colors );
	scriptWin->changeBoxColor( colors );
	camWin->changeBoxColor( colors );
	auxWin->changeBoxColor( colors );
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


void Communicator::postMyString( CWnd* window, UINT messageTypeID, std::string message )
{
	// The window recieving this message is responsible for deleting this pointer.
	char* messageChars = new char[message.size() + 1];
	sprintf_s( messageChars, message.size() + 1, "%s", cstr(message));
	window->PostMessageA( messageTypeID, 0, (LPARAM)messageChars );
}
