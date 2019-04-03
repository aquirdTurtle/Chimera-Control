#include "stdafx.h"
#include "MasterThreadInput.h"
#include "AuxiliaryWindow.h"
#include "MainWindow.h"

MasterThreadInput::MasterThreadInput ( AuxiliaryWindow* auxWin, MainWindow* mainWin ) :
	ttls ( auxWin->getTtlBoard ( ) ), aoSys ( auxWin->getAoSys ( ) ), aiSys ( auxWin->getAiSys ( ) ),
	python ( mainWin->getPython ( ) ), niawg ( mainWin->getNiawg ( ) ), comm ( mainWin->getCommRef ( ) ),
	rsg ( auxWin->getRsg ( ) ), eoAxialTek ( auxWin->getEoAxialTek ( ) ), topBottomTek ( auxWin->getTopBottomTek ( ) ),
	globalControl( auxWin->getGlobals() )
{ };
