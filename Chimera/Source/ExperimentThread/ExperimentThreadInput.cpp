#include "stdafx.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include "PrimaryWindows/AuxiliaryWindow.h"
#include "PrimaryWindows/MainWindow.h"
#include "PrimaryWindows/AndorWindow.h"
#include "PrimaryWindows/BaslerWindow.h"
#include "PrimaryWindows/ScriptingWindow.h"
#include "PrimaryWindows/IChimeraWindow.h"

ExperimentThreadInput::ExperimentThreadInput ( IChimeraWindow* win ) :
	ttls ( win->auxWin->getTtlCore ( ) ), aoSys ( win->auxWin->getAoSys ( ) ), aiSys (win->auxWin->getAiSys ( ) ),
	python (win->mainWin->getPython ( ) ), niawg (win->scriptWin->getNiawg ( ) ), comm (win->mainWin->getCommRef ( ) ),
	rsg (win->auxWin->getRsg ( ) ), eoAxialTek (win->auxWin->getEoAxialTek ( ) ), topBottomTek (win->auxWin->getTopBottomTek ( ) ),
	globalControl(win->auxWin->getGlobals() ), andorCamera(win->andorWin->getCamera() ), dds(win->auxWin->getDds() ),
	logger(win->andorWin->getLogger() ), piezoCores(win->auxWin->getPiezoControllers() ), basCamera (win->basWin->getCore())
{ };

