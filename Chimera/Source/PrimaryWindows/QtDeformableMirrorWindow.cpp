#include "stdafx.h"
#include "QtDeformableMirrorWindow.h"
#include "Agilent/AgilentSettings.h"
#include <qdesktopwidget.h>
#include "QtMainWindow.h"
#include <ExperimentMonitoringAndStatus/ColorBox.h>

QtDeformableMirrorWindow::QtDeformableMirrorWindow (QWidget* parent) : IChimeraQtWindow (parent), 
dm (this, DM_SERIAL, DM_SAFEMODE){
	statBox = new ColorBox ();
	setWindowTitle ("Deformable Mirror Window");
}

QtDeformableMirrorWindow::~QtDeformableMirrorWindow (){
}

void QtDeformableMirrorWindow::initializeWidgets (){
	QPoint pos = { 0,0 };
	statBox->initialize (pos, this, 480, mainWin->getDevices ());
	dm.initialize (pos, this, dm.getActNum (), DM_SERIAL, 65);
}

void QtDeformableMirrorWindow::handleAddAbberations () {
	try	{
		dm.add_Changes ();
	}
	catch (ChimeraError& err) {
		reportErr (err.qtrace ());
	}
}

void QtDeformableMirrorWindow::windowOpenConfig (ConfigStream& configFile){
	try	{
		if (configFile.ver >= Version ("4.7")) {
			DMOutputForm form;
			ConfigSystem::stdGetFromConfig (configFile, dm.getCore (), form);
			dm.setCoreInfo (form);
			dm.openConfig ();
		}
	}
	catch (ChimeraError&){
		throwNested ("Auxiliary Window failed to read parameters from the configuration file.");
	}
}

void QtDeformableMirrorWindow::windowSaveConfig (ConfigStream& newFile){
	dm.handleSaveConfig (newFile);
}

void QtDeformableMirrorWindow::fillExpDeviceList (DeviceList& list){
	list.list.push_back (dm.getCore ());
}