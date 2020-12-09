#include "stdafx.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include "PrimaryWindows/QtAuxiliaryWindow.h"
#include "PrimaryWindows/QtMainWindow.h"
#include "PrimaryWindows/QtAndorWindow.h"
#include "PrimaryWindows/QtBaslerWindow.h"
#include "PrimaryWindows/QtScriptWindow.h"
#include "PrimaryWindows/IChimeraQtWindow.h"

ExperimentThreadInput::ExperimentThreadInput ( IChimeraQtWindow* win ) :
	ttls ( win->auxWin->getTtlCore ( ) ), aoSys ( win->auxWin->getAoSys ( ) ),
	logger(win->andorWin->getLogger() ){
	devices = win->mainWin->getDevices ();
};

