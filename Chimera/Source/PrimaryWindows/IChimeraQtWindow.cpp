#include "stdafx.h"
#include "IChimeraQtWindow.h"
#include "QtMainWindow.h"
#include "QtAuxiliaryWindow.h"
#include "QtAndorWindow.h"
#include "QtScriptWindow.h"
#include "ExperimentMonitoringAndStatus/colorbox.h"
#include "GeneralUtilityFunctions/CommonFunctions.h"
#include <qshortcut.h>
#include <qmenubar.h>

IChimeraQtWindow::IChimeraQtWindow (QWidget* parent) : QMainWindow(parent) {}

void IChimeraQtWindow::reportErr (QString errStr, unsigned errorLevel){
	mainWin->onErrorMessage (errStr, errorLevel);
}

void IChimeraQtWindow::reportStatus (QString statusStr, unsigned notificationLevel){
	mainWin->handleNotification (statusStr, notificationLevel);
}

void IChimeraQtWindow::loadFriends ( QtMainWindow* mainWin_, QtScriptWindow* scriptWin_, QtAuxiliaryWindow* auxWin_,
									 QtAndorWindow* andorWin_ ){
	mainWin = mainWin_;
	scriptWin = scriptWin_;
	auxWin = auxWin_;
	andorWin = andorWin_;
}


void IChimeraQtWindow::changeBoxColor (std::string sysDelim, std::string color){
	if (statBox->initialized){
		statBox->changeColor (sysDelim, color);
	}
}

std::vector<IChimeraQtWindow*> IChimeraQtWindow::winList (){
	std::vector<IChimeraQtWindow*> list = { (IChimeraQtWindow*)scriptWin, (IChimeraQtWindow*)andorWin, 
											(IChimeraQtWindow*)auxWin, (IChimeraQtWindow*)mainWin };
	return list;
}

void IChimeraQtWindow::configUpdated () {
	mainWin->notifyConfigUpdate ();
}

void IChimeraQtWindow::initializeShortcuts (){
	QShortcut* sc_F5= new QShortcut (QKeySequence (Qt::Key_F5), this);
	connect (sc_F5, &QShortcut::activated, [this]() {commonFunctions::handleCommonMessage(ID_ACCELERATOR_F5, this); });

	QShortcut* sc_esc2 = new QShortcut (QKeySequence (Qt::CTRL + Qt::Key_F5), this);
	connect (sc_esc2, &QShortcut::activated, [this]() {commonFunctions::handleCommonMessage (ID_ACCELERATOR_ESC, this); });
	QShortcut* sc_esc3 = new QShortcut (QKeySequence (Qt::SHIFT + Qt::Key_F5), this);
	connect (sc_esc3, &QShortcut::activated, [this]() {commonFunctions::handleCommonMessage (ID_ACCELERATOR_ESC, this); });

	QShortcut* sc_F1 = new QShortcut (QKeySequence (Qt::Key_F1), this);
	connect (sc_F1, &QShortcut::activated, [this]() {commonFunctions::handleCommonMessage (ID_ACCELERATOR_F1, this); });
	
	QShortcut* sc_F11 = new QShortcut (QKeySequence (Qt::Key_F11), this);
	connect (sc_F11, &QShortcut::activated, [this]() {commonFunctions::handleCommonMessage (ID_ACCELERATOR_F11, this); });

	QShortcut* sc_F2 = new QShortcut (QKeySequence (Qt::Key_F2), this);
	connect (sc_F2, &QShortcut::activated, [this]() {commonFunctions::handleCommonMessage (ID_ACCELERATOR_F2, this); });

	QShortcut* sc_ctrlS = new QShortcut (QKeySequence (Qt::CTRL + Qt::Key_S), this);
	connect (sc_ctrlS, &QShortcut::activated, [this]() {commonFunctions::handleCommonMessage (ID_FILE_SAVEALL, this); });
}

void IChimeraQtWindow::initializeMenu (){
	constexpr auto cmnMsg = commonFunctions::handleCommonMessage;

	auto menubar = menuBar ();
	/// FILE
	auto fileM = menubar->addMenu ("&File");

	auto* saveAll = new QAction ("Save All\tCtrl-S", this);
	connect (saveAll, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_FILE_SAVEALL, this); });
	fileM->addAction (saveAll);

	/// RUN
	auto runMenuM = menubar->addMenu ("&Run Menu");

	auto* runAll = new QAction ("Run Everything\tF5", this);
	connect (runAll, &QAction::triggered,[this, cmnMsg]() {cmnMsg (ID_ACCELERATOR_F5, this); });
	runMenuM->addAction (runAll);

	runMenuM->addAction ("Run Camera_X");
	runMenuM->addAction ("Run Master_X");
	runMenuM->addAction ("Run Basler_X");
	runMenuM->addAction ("Run Basler and Master_X");
	runMenuM->addAction ("Write Waveforms Only_X");

	auto* abortAll = new QAction ("Abort All\tCtrl+F5", this);
	connect (abortAll, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_ACCELERATOR_ESC, this); });
	runMenuM->addAction (abortAll);

	runMenuM->addAction ("Abort Camera_X");
	runMenuM->addAction ("Abort Master_X");
	runMenuM->addAction ("Abort Basler_X");
	auto* pause = new QAction ("Pause Experiment\tF2", this);
	connect (pause, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_RUNMENU_PAUSE, this); });
	runMenuM->addAction (pause);
	/// PROFILE
	auto profileM = menubar->addMenu ("Profil&e");
	auto mc_p_m = profileM->addMenu ("Master Configuration");
	auto* saveMasterConfig = new QAction ("Save Master Configuration", this);
	connect (saveMasterConfig, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_MASTERCONFIG_SAVEMASTERCONFIGURATION, this); });
	mc_p_m->addAction (saveMasterConfig);

	auto* reloadMasterConfig = new QAction ("Re-Load Master Configuration", this);
	connect (reloadMasterConfig, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_MASTERCONFIGURATION_RELOAD_MASTER_CONFIG, this); });
	mc_p_m->addAction (reloadMasterConfig);

	auto conf_p_m = profileM->addMenu ("Con&figuration");

	auto* saveConfig = new QAction ("&Save Configuration", this);
	connect (saveConfig, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_CONFIGURATION_SAVECONFIGURATIONSETTINGS, this); });
	conf_p_m->addAction (saveConfig);

	auto* saveConfigAs = new QAction ("Save Configuration &As", this);
	connect (saveConfigAs, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_CONFIGURATION_SAVE_CONFIGURATION_AS, this); });
	conf_p_m->addAction (saveConfigAs);

	profileM->addAction ("Save Entire Profile_X");
	/// SCRIPTS
	auto scriptsM = menubar->addMenu ("&Scripts");
	auto masterSc = scriptsM->addMenu ("M&aster Script");
		
	auto* newMaster = new QAction ("Ne&w Master Script", this);
	connect (newMaster, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_MASTERSCRIPT_NEW, this); });
	masterSc->addAction (newMaster);
	auto* openMaster = new QAction ("Op&en Master Script", this);
	connect (openMaster, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_MASTERSCRIPT_OPENSCRIPT, this); });
	masterSc->addAction (openMaster);
	auto* saveMaster = new QAction ("&Save Master Script", this);
	connect (saveMaster, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_MASTERSCRIPT_SAVE, this); });
	masterSc->addAction (saveMaster);
	auto* saveMasterAs = new QAction ("Save Master Script &As", this);
	connect (saveMasterAs, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_MASTERSCRIPT_SAVEAS, this); });
	masterSc->addAction (saveMasterAs);
	
	/// 
	auto* newInt = new QAction ("Ne&w Intensity Agilent Script", this);

	auto masterSystemsM = menubar->addMenu ("Master Systems");
	auto* changeIndvDo = new QAction ("View or Change Individual Digital Output Settings", this);
	connect (changeIndvDo, &QAction::triggered, [this, cmnMsg]() {auxWin->ViewOrChangeTTLNames(); });
	masterSystemsM->addAction (changeIndvDo);

	auto* changeIndvAo = new QAction ("View or Change Individual Analog Output Settings", this);
	connect (changeIndvAo, &QAction::triggered, [this, cmnMsg]() {auxWin->ViewOrChangeDACNames (); });
	masterSystemsM->addAction (changeIndvAo);

	auto helpM = menubar->addMenu ("Help");
	helpM->addAction ("General Information_X");
	helpM->addAction ("About_X");
	helpM->addAction ("Hardware Status_X");
	auto prefM = menubar->addMenu ("Preferences");
	auto* reloadStylesheets = new QAction ("Reload Stylesheet", this);
	connect (reloadStylesheets, &QAction::triggered, [this, cmnMsg]() {this->mainWin->setStyleSheets ();});
	prefM->addAction (reloadStylesheets);
}