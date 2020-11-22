#include "stdafx.h"
#include <GeneralObjects/IChimeraSystem.h>
#include <PrimaryWindows/IChimeraQtWindow.h>
#include <PrimaryWindows/QtMainWindow.h>

IChimeraSystem::IChimeraSystem (IChimeraQtWindow* parent_in) {
	parent = parent_in;
	connect (this, &IChimeraSystem::error, parent, &IChimeraQtWindow::reportErr);
	connect (this, &IChimeraSystem::notification, parent, &IChimeraQtWindow::reportStatus);
}

