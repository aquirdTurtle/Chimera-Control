#include "stdafx.h"
#include "IChimeraQtWindow.h"
#include "QtMainWindow.h"
#include "QtAuxiliaryWindow.h"
#include "QtAndorWindow.h"
#include "QtBaslerWindow.h"
#include "QtDeformableMirrorWindow.h"
#include "QtScriptWindow.h"
#include "ExperimentMonitoringAndStatus/colorbox.h"
#include "GeneralUtilityFunctions/CommonFunctions.h"
#include <qshortcut.h>
#include <qmenubar.h>

IChimeraQtWindow::IChimeraQtWindow (QWidget* parent) : QMainWindow(parent) {}

void IChimeraQtWindow::reportErr (statusMsg msg){
	mainWin->onErrorMessage (msg);
}

void IChimeraQtWindow::reportWarning(statusMsg msg) {
	msg.baseLevel += 1;
	mainWin->onErrorMessage(msg);
}

void IChimeraQtWindow::reportStatus (statusMsg msg){
	mainWin->handleNotification (msg);
}

void IChimeraQtWindow::loadFriends ( QtMainWindow* mainWin_, QtScriptWindow* scriptWin_, QtAuxiliaryWindow* auxWin_,
										 QtBaslerWindow* basWin_, QtDeformableMirrorWindow* dmWindow_, QtAndorWindow* andorWin_ ){
	mainWin = mainWin_;
	scriptWin = scriptWin_;
	auxWin = auxWin_;
	basWin = basWin_;
	dmWin = dmWindow_;
	andorWin = andorWin_;
}


void IChimeraQtWindow::changeBoxColor (std::string sysDelim, std::string color){
	if (statBox->initialized){
		statBox->changeColor (sysDelim, color);
	}
}

std::vector<IChimeraQtWindow*> IChimeraQtWindow::winList (){
	std::vector<IChimeraQtWindow*> list = { (IChimeraQtWindow*)scriptWin, (IChimeraQtWindow*)andorWin, 
		(IChimeraQtWindow*)auxWin, (IChimeraQtWindow*)basWin, (IChimeraQtWindow*)dmWin, 
		(IChimeraQtWindow*)mainWin };
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

	//auto* exit = new QAction ("Exit Program\tAlt-F4", this);
	//connect (exit, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_FILE_CLOSE, this); });
	//fileM->addAction (exit);

	/// RUN
	auto runMenuM = menubar->addMenu ("&Run Menu");

	auto* runAll = new QAction ("Run Everything\tF5", this);
	connect (runAll, &QAction::triggered,[this, cmnMsg]() {cmnMsg (ID_ACCELERATOR_F5, this); });
	runMenuM->addAction (runAll);

	runMenuM->addAction ("Run Camera_X");
	runMenuM->addAction ("Run NIAWG_X");
	runMenuM->addAction ("Run Master_X");
	runMenuM->addAction ("Run Basler_X");
	runMenuM->addAction ("Run Basler and Master_X");
	runMenuM->addAction ("Write Waveforms Only_X");

	auto* abortAll = new QAction ("Abort All\tCtrl+F5", this);
	connect (abortAll, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_ACCELERATOR_ESC, this); });
	runMenuM->addAction (abortAll);

	runMenuM->addAction ("Abort Camera_X");
	runMenuM->addAction ("Abort NIAWG_X");
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
	auto niawgScM = scriptsM->addMenu ("NIA&WG Script");
	auto intM = scriptsM->addMenu ("Int&ensity Script");
	auto masterSc = scriptsM->addMenu ("M&aster Script");
	auto otherAg = scriptsM->addMenu ("Othe&r Agilents");
	auto tbM = otherAg->addMenu ("Top / Bottom");
	auto axM = otherAg->addMenu ("Axial");
	auto flashingM = otherAg->addMenu ("Flashing");
	auto uwM = otherAg->addMenu ("Microwave");
	std::vector<QMenu*> scriptMenus = { flashingM, uwM };
	std::vector<QString> scriptNames = { "Flashing", "Microwave" };
	for (auto num : range (scriptMenus.size())) {
		auto menu = scriptMenus[num];
		auto name = scriptNames[num];
		menu->addAction ("Ne&w " + name + " Script_X");
		menu->addAction ("Op&en " + name + " Script_X");
		menu->addAction ("&Save " + name + " Script_X");
		menu->addAction ("Save " + name + " Script &As_X");
	}

	auto* newAxScript = new QAction ("Ne&w Axial Script", this);
	connect (newAxScript, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_AXIAL_NEW_SCRIPT, this); });
	axM->addAction (newAxScript);
	auto* openAxScript = new QAction ("Op&en Axial Script", this);
	connect (openAxScript, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_AXIAL_OPEN_SCRIPT, this); });
	axM->addAction (openAxScript);
	auto* saveAxScript = new QAction ("&Save Axial Script", this);
	connect (saveAxScript, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_AXIAL_SAVE_SCRIPT, this); });
	axM->addAction (saveAxScript);
	auto* saveAxScriptAs = new QAction ("Save Axial Script &As", this);
	connect (saveAxScriptAs, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_AXIAL_SAVE_SCRIPT_AS, this); });
	axM->addAction (saveAxScriptAs);

	auto* newTBScript = new QAction ("Ne&w Top/Bottom Script", this);
	connect (newTBScript, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_TOP_BOTTOM_NEW_SCRIPT, this); });
	tbM->addAction (newTBScript);
	auto* openTBScript = new QAction ("Op&en Top/Bottom Script", this);
	connect (openTBScript, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_TOP_BOTTOM_OPEN_SCRIPT, this); });
	tbM->addAction (openTBScript);
	auto* saveTBScript = new QAction ("&Save Top/Bottom Script", this);
	connect (saveTBScript, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_TOP_BOTTOM_SAVE_SCRIPT, this); });
	tbM->addAction (saveTBScript);
	auto* saveTBScriptAs = new QAction ("Save Top/Bottom Script &As", this);
	connect (saveTBScriptAs, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_TOP_BOTTOM_SAVE_SCRIPT_AS, this); });
	tbM->addAction (saveTBScriptAs);


	auto* newNiawg = new QAction ("Ne&w NIAWG Script", this);
	connect (newNiawg, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_FILE_MY_NIAWG_NEW, this); });
	niawgScM->addAction (newNiawg);
	auto* openNiawg = new QAction ("Op&en NIAWG Script", this);
	connect (openNiawg, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_FILE_MY_NIAWG_OPEN, this); });
	niawgScM->addAction (openNiawg);
	auto* saveNiawg = new QAction ("&Save NIAWG Script", this);
	connect (saveNiawg, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_FILE_MY_NIAWG_SAVE, this); });
	niawgScM->addAction (saveNiawg);
	auto* saveNiawgAs = new QAction ("Save NIAWG Script &As", this);
	connect (saveNiawgAs, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_FILE_MY_NIAWG_SAVEAS, this); });
	niawgScM->addAction (saveNiawgAs);
	
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
	connect (newInt, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_FILE_MY_INTENSITY_NEW, this); });
	intM->addAction (newInt);
	auto* openInt = new QAction ("Op&en Intensity Agilent Script", this);
	connect (openInt, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_FILE_MY_INTENSITY_OPEN, this); });
	intM->addAction (openInt);
	auto* saveInt = new QAction ("&Save Intensity Agilent Script", this);
	connect (saveInt, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_FILE_MY_INTENSITY_SAVE, this); });
	intM->addAction (saveInt);
	auto* saveasInt = new QAction ("Save Intensity Agilent Script &As", this);
	connect (saveasInt, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_FILE_MY_INTENSITY_SAVEAS, this); });
	intM->addAction (saveasInt);

	auto niawgM = menubar->addMenu ("NIAWG");
	auto* reloadDefaultNiawg = new QAction ("Reload Default Waveforms", this);
	connect (reloadDefaultNiawg, &QAction::triggered, [this]() {this->scriptWin->restartNiawgDefaults (); });
	niawgM->addAction (reloadDefaultNiawg);
	   
	niawgM->addAction ("Send Software Trigger_X");
	niawgM->addAction ("Stream Waveform_X");
	niawgM->addAction ("Get NIWAG Error_X");
	niawgM->addAction ("NIAWG is On_X");
	auto andorM = menubar->addMenu ("Andor");
	andorM->addAction ("Show Pictures in Real Time_X");
	andorM->addAction ("Autoscale Pictures_X");
	andorM->addAction ("Always Show Grid_X");
	andorM->addAction ("Color > Max Special_X");
	andorM->addAction ("Color < Min Special_X");
	auto andorsubm = andorM->addMenu ("Data Type");
	andorsubm->addAction ("Raw Counts_X");
	andorsubm->addAction ("Photons Collected_X");
	andorsubm->addAction ("Photons Scattered_X");
	auto plottingM = menubar->addMenu ("Plotting");
	plottingM->addAction ("Stop Plotter_X");
	auto masterSystemsM = menubar->addMenu ("Master Systems");
	masterSystemsM->addAction ("Round DAC Values to DAC's Precision_X");

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
	auto baslerM = menubar->addMenu ("Basler");
	baslerM->addAction ("Autoscale Pictures_X");
}