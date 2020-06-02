#include "stdafx.h"
#include "QtDeformableMirrorWindow.h"
#include "Agilent/AgilentSettings.h"
#include <qdesktopwidget.h>
#include "QtMainWindow.h"

QtDeformableMirrorWindow::QtDeformableMirrorWindow (QWidget* parent) : IChimeraWindowWidget (parent), dm (DM_SERIAL, DM_SAFEMODE)
{
	statBox = new ColorBox ();
	setWindowTitle ("Deformable Mirror Window");
}

QtDeformableMirrorWindow::~QtDeformableMirrorWindow ()
{

}

void QtDeformableMirrorWindow::initializeWidgets ()
{
	POINT pos = { 0,0 };
	int id = 1000;
	UINT ID = IDC_DM_PROGRAMNOW;
	statBox->initialize (pos, this, 480, mainWin->getDevices ());
	dm.initialize (pos, this, dm.getActNum (), DM_SERIAL, 65);
}

void QtDeformableMirrorWindow::handleProgramDmNow () {
	dm.ProgramNow ();
}

void QtDeformableMirrorWindow::handleNewDmProfile () {
	try {
		dm.loadProfile ();
	}
	catch (Error& err) {
		reportErr (err.trace ());
	}
}

void QtDeformableMirrorWindow::handlePistonChange (UINT id) {
}

void QtDeformableMirrorWindow::handleAddAbberations () {
	try
	{
		dm.add_Changes ();
	}
	catch (Error& err)
	{
		reportErr (err.trace ());
	}
}

void QtDeformableMirrorWindow::windowOpenConfig (ConfigStream& configFile)
{
	try
	{
		if (configFile.ver >= Version ("4.7"))
		{
			DMOutputForm form;
			ProfileSystem::stdGetFromConfig (configFile, dm.getCore (), form);
			dm.setCoreInfo (form);
			dm.openConfig ();
		}
	}
	catch (Error&)
	{
		throwNested ("Auxiliary Window failed to read parameters from the configuration file.");
	}
}

void QtDeformableMirrorWindow::windowSaveConfig (ConfigStream& newFile)
{
	dm.handleSaveConfig (newFile);
}

void QtDeformableMirrorWindow::fillExpDeviceList (DeviceList& list)
{
	list.list.push_back (dm.getCore ());
}