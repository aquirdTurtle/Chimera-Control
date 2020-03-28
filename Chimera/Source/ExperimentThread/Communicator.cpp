// created by Mark O. Brown

#include "stdafx.h"
#include "ExperimentThread/Communicator.h"
#include "PrimaryWindows/AndorWindow.h"
#include "PrimaryWindows/AuxiliaryWindow.h"
#include "PrimaryWindows/MainWindow.h"
#include "PrimaryWindows/ScriptingWindow.h"
#include "PrimaryWindows/BaslerWindow.h"
#include "PrimaryWindows/DeformableMirrorWindow.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include "LowLevel/externals.h"

// pass all the windows so that the object can (in principle) send messages to any window.
void Communicator::initialize( IChimeraWindow* win )
{
	mainWin = win->mainWin;
	scriptWin = win->scriptWin;
	andorWin = win->andorWin;
	auxWin = win->auxWin;
	basWin = win->basWin;
	dmWin = win->dmWin;
}

void Communicator::sendBaslerFin ( )
{
	andorWin->PostMessage ( CustomMessages::BaslerFinMessageID, 0, 0 );
}


void Communicator::sendNoMotAlert ( )
{
	mainWin->PostMessage (CustomMessages::NoMotAlertMessageID, 0, 0 );
}



void Communicator::sendNoAtomsAlert ( )
{
	mainWin->PostMessage (CustomMessages::NoAtomsAlertMessageID, 0, 0 );
}


void Communicator::sendFinish ( ExperimentType type )
{
	mainWin->PostMessage (CustomMessages::GeneralFinMsgID, int ( type ), 0 );
}


// the two camera messages go straight to the camera window.
void Communicator::sendCameraFin()
{
	andorWin->PostMessage(CustomMessages::AndorFinishMessageID, 0, 0 );
}

void Communicator::sendExperimentProcedureFinish ( )
{
	mainWin->PostMessageA (CustomMessages::experimentProcedureFinish, 0, 0 );
}

void Communicator::sendCameraCalFin( )
{
	andorWin->PostMessage(CustomMessages::AndorCalFinMessageID, 0, 0 );
}


void Communicator::sendAutoServo( )
{
	auxWin->PostMessage(CustomMessages::AutoServoMessage, 0, 0 );
}


void Communicator::sendCameraProgress(long progress)
{
	andorWin->PostMessage(CustomMessages::AndorProgressMessageID, 0, (LPARAM)progress );
}

void Communicator::sendCameraCalProgress( long progress )
{
	andorWin->PostMessage(CustomMessages::AndorCalProgMessageID, 0, (LPARAM)progress );
}

void Communicator::sendLogVoltsMessage ( UINT variation )
{
	auxWin->PostMessage (CustomMessages::LogVoltsMessageID, variation );
}


void Communicator::sendRepProgress(ULONG rep)
{
	mainWin->PostMessage(CustomMessages::RepProgressMessageID, 0, LPARAM(rep));
}

void Communicator::sendTimer( std::string timerMsg )
{
	if ( timerMsg != "" )
	{
		andorWin->setTimerText( timerMsg );
	}
}

/*
 * Don't use this function directly, it was designed to be used with the macro in the header file.
 */
void Communicator::sendErrorEx( std::string statusMsg, const char *file, int line)
{
	postMyString( mainWin, CustomMessages::ErrorUpdateMessageID, statusMsg );
}

/*
* Don't use this function directly, it was designed to be used with the macro in the header file.
*/
void Communicator::sendFatalErrorEx( std::string statusMsg, const char *file, int line)
{
	postMyString( mainWin, CustomMessages::FatalErrorMessageID, statusMsg );
}


void Communicator::sendColorBox( System sys, char code )
{
	systemInfo<char> colors;
	switch ( sys )
	{
		case System::Niawg:
			colors = { code, '-', '-', '-' };
			break;
		case System::Andor:
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
	andorWin->changeBoxColor( colors );
	auxWin->changeBoxColor( colors );
	basWin->changeBoxColor (colors);
	dmWin->changeBoxColor (colors);
}

void Communicator::sendStatus(std::string statusMsg)
{
	postMyString(mainWin, CustomMessages::StatusUpdateMessageID, statusMsg);
	mainWin->setShortStatus( statusMsg );
}

void Communicator::sendDebug(std::string statusMsg)
{
	if (statusMsg != "")
	{
		postMyString(mainWin, CustomMessages::DebugUpdateMessageID, statusMsg);
	}
}

void Communicator::sendPrepareBasler (baslerSettings& settingsToPrepare)
{
	basWin->SendMessage(CustomMessages::prepareBaslerWinAcq, 0, LPARAM (&settingsToPrepare));
}

void Communicator::sendPrepareAndor (AndorRunSettings& settingsToPrepare)
{
	andorWin->SendMessage (CustomMessages::prepareAndorWinAcq, 0, LPARAM (&settingsToPrepare));
}

void Communicator::postMyString( CWnd* window, UINT messageTypeID, std::string message )
{
	// The window recieving this message is responsible for deleting this pointer.
	char* messageChars = new char[message.size() + 1];
	sprintf_s( messageChars, message.size() + 1, "%s", cstr(message));
	window->PostMessageA( messageTypeID, 0, (LPARAM)messageChars );
}
