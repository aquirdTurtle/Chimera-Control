// created by Mark O. Brown
#include "stdafx.h"
#include "saveWithExplorer.h"
#include <PrimaryWindows/IChimeraQtWindow.h>
#include <qfiledialog.h>

/*
 * This function opens up the dialog for selecting an existing or new file name for saving scripts and configurations. It uses GetSaveFileName.
 * hwndOwner is the handle for the owner window of the new dialog
 * extension is the text of the default extension for saving this time.
 * returns the name that was selected. If no name is selected, this returns "".
 */
std::string saveWithExplorer(IChimeraQtWindow* owner, std::string extension, profileSettings location)
{
	return str(QFileDialog::getSaveFileName ( owner, "Save File", location.configLocation.c_str(), 
													  ("*."+extension).c_str()));
}

