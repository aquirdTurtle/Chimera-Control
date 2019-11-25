// created by Mark O. Brown

#include "stdafx.h"
#include "Communicator.h"
#include "AndorWindow.h"
#include "AuxiliaryWindow.h"
#include "MainWindow.h"
#include "ScriptingWindow.h"
#include "MasterThreadInput.h"
#include "externals.h"

// pass all the windows so that the object can (in principle) send messages to any window.
void Communicator::initialize(MainWindow* mainWinParent, ScriptingWindow* scriptingWin, AndorWindow* cameraWin, 
							  AuxiliaryWindow* auxWindow )
{
	mainWin = mainWinParent;
	scriptWin = scriptingWin;
	camWin = cameraWin;
	auxWin = auxWindow;
}


void Communicator::sendBaslerFin ( )
{
	camWin->PostMessageA ( MainWindow::BaslerFinMessageID, 0, 0 );
}


void Communicator::sendNoMotAlert ( )
{
	mainWin->PostMessageA ( MainWindow::NoMotAlertMessageID, 0, 0 );
}



void Communicator::sendNoAtomsAlert ( )
{
	mainWin->PostMessageA ( MainWindow::NoAtomsAlertMessageID, 0, 0 );
}


void Communicator::sendFinish ( ExperimentType type )
{
	mainWin->PostMessage ( MainWindow::GeneralFinMsgID, int ( type ), 0 );
}


// the two camera messages go straight to the camera window.
void Communicator::sendCameraFin()
{

	OutputDebugString ("ctFin!\n");
	camWin->PostMessage( MainWindow::AndorFinishMessageID, 0, 0 );
}

void Communicator::sendExperimentProcedureFinish ( )
{
	mainWin->PostMessageA ( MainWindow::experimentProcedureFinish, 0, 0 );
}

void Communicator::sendCameraCalFin( )
{
	camWin->PostMessage( MainWindow::AndorCalFinMessageID, 0, 0 );
}


void Communicator::sendAutoServo( )
{
	auxWin->PostMessageA( MainWindow::AutoServoMessage, 0, 0 );
}


void Communicator::sendCameraProgress(long progress)
{
	OutputDebugString (cstr("ctProg: " + str(progress) + ", " ));
	camWin->PostMessageA( MainWindow::AndorProgressMessageID, 0, (LPARAM)progress );
}

void Communicator::sendCameraCalProgress( long progress )
{
	camWin->PostMessageA( MainWindow::AndorCalProgMessageID, 0, (LPARAM)progress );
}

void Communicator::sendLogVoltsMessage ( UINT variation )
{
	auxWin->PostMessage ( MainWindow::LogVoltsMessageID, variation );
}


void Communicator::sendRepProgress(ULONG rep)
{
	mainWin->PostMessageA(MainWindow::RepProgressMessageID, 0, LPARAM(rep));
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
	postMyString( mainWin, MainWindow::ErrorUpdateMessageID, statusMsg );
}

/*
* Don't use this function directly, it was designed to be used with the macro in the header file.
*/
void Communicator::sendFatalErrorEx( std::string statusMsg, const char *file, int line)
{
	postMyString( mainWin, MainWindow::FatalErrorMessageID, statusMsg );
}


void Communicator::sendColorBox( System sys, char code )
{
	systemInfo<char> colors;
	switch ( sys )
	{
		case System::Niawg:
			colors = { code, '-', '-', '-' };
			break;
		case System::Camera:
			colors = { '-', code, '-', '-' };
			break;
		case System::Master:
			colors = { '-', '-', code, '-' };
			break;
		case System::Basler:
			colors = { '-','-','-',code };
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
	postMyString(mainWin, MainWindow::StatusUpdateMessageID, statusMsg);
	mainWin->setShortStatus( statusMsg );
}

void Communicator::sendDebug(std::string statusMsg)
{
	if (statusMsg != "")
	{
		postMyString(mainWin, MainWindow::DebugUpdateMessageID, statusMsg);
	}
}


void Communicator::postMyString( CWnd* window, UINT messageTypeID, std::string message )
{
	// The window recieving this message is responsible for deleting this pointer.
	char* messageChars = new char[message.size() + 1];
	sprintf_s( messageChars, message.size() + 1, "%s", cstr(message));
	window->PostMessageA( messageTypeID, 0, (LPARAM)messageChars );
}
