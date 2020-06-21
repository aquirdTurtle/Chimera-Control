#include "stdafx.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include "PrimaryWindows/QtAuxiliaryWindow.h"
#include "PrimaryWindows/QtMainWindow.h"
#include "PrimaryWindows/QtAndorWindow.h"
#include "PrimaryWindows/QtBaslerWindow.h"
#include "PrimaryWindows/QtScriptWindow.h"
#include "PrimaryWindows/IChimeraWindowWidget.h"

ExperimentThreadInput::ExperimentThreadInput ( IChimeraWindowWidget* win ) :
	ttls ( win->auxWin->getTtlCore ( ) ), aoSys ( win->auxWin->getAoSys ( ) ),
	python (win->mainWin->getPython ( ) ), comm (win->mainWin->getCommRef ( ) ), 
	globalControl(win->auxWin->getGlobals() ), logger(win->andorWin->getLogger() )
{
	devices = win->mainWin->getDevices ();
};

