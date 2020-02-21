#include "stdafx.h"
#include "ExperimentThread/ExperimentThreadInput.h"
#include "PrimaryWindows/AuxiliaryWindow.h"
#include "PrimaryWindows/MainWindow.h"
#include "PrimaryWindows/AndorWindow.h"
#include "PrimaryWindows/BaslerWindow.h"

ExperimentThreadInput::ExperimentThreadInput ( AuxiliaryWindow* auxWin, MainWindow* mainWin, AndorWindow* andorWin, 
	BaslerWindow* basWin) :
	ttls ( auxWin->getTtlCore ( ) ), aoSys ( auxWin->getAoSys ( ) ), aiSys ( auxWin->getAiSys ( ) ),
	python ( mainWin->getPython ( ) ), niawg ( mainWin->getNiawg ( ) ), comm ( mainWin->getCommRef ( ) ),
	rsg ( auxWin->getRsg ( ) ), eoAxialTek ( auxWin->getEoAxialTek ( ) ), topBottomTek ( auxWin->getTopBottomTek ( ) ),
	globalControl( auxWin->getGlobals() ), andorCamera( andorWin->getCamera() ), dds( auxWin->getDds() ), 
	logger(andorWin->getLogger() ), piezoCores( auxWin->getPiezoControllers() ), basCamera (basWin->getCore())
{ };

