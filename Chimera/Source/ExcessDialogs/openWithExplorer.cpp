// created by Mark O. Brown
#include "stdafx.h"

#include "openWithExplorer.h"
#include <qdesktopservices.h>
#include <qfiledialog.h>
/*
 * This function gets the name of a file to open, using the fancy windows dialog box.
 * It returns the chosen filename if successful, it returns "" if the user canceled.
 */
std::string openWithExplorer(IChimeraWindowWidget* parent, std::string extension)
{
	auto res = QFileDialog::getOpenFileName (parent, "", PROFILES_PATH.c_str(), ("*."+extension).c_str());
	return str(res);
}
