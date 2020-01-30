#include "stdafx.h"
#include "MasterThreadInput.h"
#include "AuxiliaryWindow.h"
#include "AuxiliaryWindow2.h"
#include "MainWindow.h"
#include "AndorWindow.h"

ExperimentThreadInput::ExperimentThreadInput (AuxiliaryWindow* auxWin, MainWindow* mainWin, AndorWindow* andorWin,
											  AuxiliaryWindow2* auxWin2) :
	ttls ( auxWin->getTtlBoard ( ) ), aoSys ( auxWin->getAoSys ( ) ), aiSys ( auxWin->getAiSys ( ) ),
	python ( mainWin->getPython ( ) ), niawg ( mainWin->getNiawg ( ) ), comm ( mainWin->getCommRef ( ) ),
	rsg ( auxWin->getRsg ( ) ), eoAxialTek ( auxWin->getEoAxialTek ( ) ), topBottomTek ( auxWin->getTopBottomTek ( ) ),
	globalControl( auxWin->getGlobals() ), andorCamera( andorWin->getCamera() ), dds( auxWin->getDds() ), 
	logger(andorWin->getLogger() ), piezoControllers( auxWin->getPiezoControllers() ), Dmir(auxWin2->GetCore() ) 
{ };

