#include "stdafx.h"
#include <GeneralObjects/IChimeraSystem.h>
#include <PrimaryWindows/IChimeraQtWindow.h>
#include <PrimaryWindows/QtMainWindow.h>

IChimeraSystem::IChimeraSystem (IChimeraQtWindow* parent_in) {
	parentWin = parent_in;
	connect (this, &IChimeraSystem::error, parentWin, &IChimeraQtWindow::reportErr);
	connect (this, &IChimeraSystem::warning, parentWin, &IChimeraQtWindow::reportWarning);
	connect (this, &IChimeraSystem::notification, parentWin, &IChimeraQtWindow::reportStatus);
}

