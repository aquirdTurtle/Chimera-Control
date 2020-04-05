#include "stdafx.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include "PrimaryWindows/AuxiliaryWindow.h"
#include "PrimaryWindows/MainWindow.h"
#include "PrimaryWindows/AndorWindow.h"
#include "PrimaryWindows/BaslerWindow.h"
#include "PrimaryWindows/ScriptingWindow.h"
#include "PrimaryWindows/IChimeraWindow.h"

ExperimentThreadInput::ExperimentThreadInput ( IChimeraWindow* win ) :
	ttls ( win->auxWin->getTtlCore ( ) ), aoSys ( win->auxWin->getAoSys ( ) ),
	python (win->mainWin->getPython ( ) ), comm (win->mainWin->getCommRef ( ) ), 
	globalControl(win->auxWin->getGlobals() ), logger(win->andorWin->getLogger() )
{
	devices = win->mainWin->getDevices ();
};

