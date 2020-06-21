#include "stdafx.h"
#include "IChimeraWindowWidget.h"
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

IChimeraWindowWidget::IChimeraWindowWidget (QWidget* parent) : QMainWindow(parent) {}

void IChimeraWindowWidget::reportErr (std::string errStr){
	mainWin->getComm ()->sendError (errStr);
}

void IChimeraWindowWidget::reportStatus (std::string statusStr){
	mainWin->getComm ()->sendStatus (statusStr);
}

void IChimeraWindowWidget::loadFriends ( QtMainWindow* mainWin_, QtScriptWindow* scriptWin_, QtAuxiliaryWindow* auxWin_,
										 QtBaslerWindow* basWin_, QtDeformableMirrorWindow* dmWindow_, QtAndorWindow* andorWin_ )
{
	mainWin = mainWin_;
	scriptWin = scriptWin_;
	auxWin = auxWin_;
	basWin = basWin_;
	dmWin = dmWindow_;
	andorWin = andorWin_;
}


void IChimeraWindowWidget::changeBoxColor (std::string sysDelim, std::string color)
{
	if (statBox->initialized)
	{
		statBox->changeColor (sysDelim, color);
	}
}

std::vector<IChimeraWindowWidget*> IChimeraWindowWidget::winList ()
{
	std::vector<IChimeraWindowWidget*> list = { (IChimeraWindowWidget*)scriptWin, (IChimeraWindowWidget*)andorWin, 
		(IChimeraWindowWidget*)auxWin, (IChimeraWindowWidget*)basWin, (IChimeraWindowWidget*)dmWin, 
		(IChimeraWindowWidget*)mainWin };
	return list;
}

void IChimeraWindowWidget::configUpdated () {
	mainWin->notifyConfigUpdate ();
}

void IChimeraWindowWidget::initializeShortcuts ()
{
	QShortcut* sc_F5= new QShortcut (QKeySequence (Qt::Key_F5), this);
	connect (sc_F5, &QShortcut::activated, [this]() {commonFunctions::handleCommonMessage(ID_ACCELERATOR_F5, this); });

	QShortcut* sc_esc2 = new QShortcut (QKeySequence (Qt::CTRL + Qt::Key_F5), this);
	connect (sc_esc2, &QShortcut::activated, [this]() {commonFunctions::handleCommonMessage (ID_ACCELERATOR_ESC, this); });

	QShortcut* sc_F1 = new QShortcut (QKeySequence (Qt::Key_F1), this);
	connect (sc_F1, &QShortcut::activated, [this]() {commonFunctions::handleCommonMessage (ID_ACCELERATOR_F1, this); });
	
	QShortcut* sc_F11 = new QShortcut (QKeySequence (Qt::Key_F11), this);
	connect (sc_F11, &QShortcut::activated, [this]() {commonFunctions::handleCommonMessage (ID_ACCELERATOR_F11, this); });

	QShortcut* sc_F2 = new QShortcut (QKeySequence (Qt::Key_F2), this);
	connect (sc_F2, &QShortcut::activated, [this]() {commonFunctions::handleCommonMessage (ID_ACCELERATOR_F2, this); });

	QShortcut* sc_ctrlS = new QShortcut (QKeySequence (Qt::CTRL + Qt::Key_S), this);
	connect (sc_ctrlS, &QShortcut::activated, [this]() {commonFunctions::handleCommonMessage (ID_FILE_SAVEALL, this); });
}

void IChimeraWindowWidget::initializeMenu ()
{
	constexpr auto cmnMsg = commonFunctions::handleCommonMessage;

	auto menubar = menuBar ();
	/// FILE
	auto fileM = menubar->addMenu ("&File");

	auto* saveAll = new QAction ("Save All\tCtrl-S", this);
	connect (saveAll, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_FILE_SAVEALL, this); });
	fileM->addAction (saveAll);

	auto* exit = new QAction ("Exit Program\tAlt-F4", this);
	connect (exit, &QAction::triggered, [this, cmnMsg]() {cmnMsg (ID_FILE_CLOSE, this); });
	fileM->addAction (exit);

	/// RUN
	auto runMenuM = menubar->addMenu ("Run Menu");

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
	runMenuM->addAction ("Pause_X");
	auto profileM = menubar->addMenu ("Profile");
	auto mc_p_m = profileM->addMenu ("Master Configuration");
	mc_p_m->addAction ("Save Master Configuration_X");
	mc_p_m->addAction ("Re-Load Master Configuration_X");
	auto conf_p_m = profileM->addMenu ("Configuration");
	conf_p_m->addAction ("New Configuration_X");
	conf_p_m->addAction ("Save Current Configuration As_X");
	conf_p_m->addAction ("Rename Current Configuration_X");
	conf_p_m->addAction ("Delete Current Configuration_X");
	conf_p_m->addAction ("Save Configuration Settings_X");
	profileM->addAction ("Save Entire Profile_X");
	auto scriptsM = menubar->addMenu ("Scripts");
	auto niawgScM = scriptsM->addMenu ("NIAWG Script");
	auto intM = scriptsM->addMenu ("Intensity Script");
	auto masterSc = scriptsM->addMenu ("Master Script");
	auto otherAg = scriptsM->addMenu ("Other Agilents");
	auto tbM = otherAg->addMenu ("Top / Bottom");
	auto axM = otherAg->addMenu ("Axial");
	auto flashingM = otherAg->addMenu ("Flashing");
	auto uwM = otherAg->addMenu ("Microwave");
	std::vector<QMenu*> scriptMenus = { niawgScM, intM, masterSc, tbM, axM, flashingM, uwM };
	std::vector<QString> scriptNames = { "NIAWG", "Intensity", "Master", "Top / Bottom", "Axial", "Flashing", "Microwave" };
	for (auto num : range (scriptMenus.size())) {
		auto menu = scriptMenus[num];
		auto name = scriptNames[num];
		masterSc->addAction ("New " + name + " Script_X");
		masterSc->addAction ("Open " + name + " Script_X");
		masterSc->addAction ("Save " + name + " Script_X");
		masterSc->addAction ("Save " + name + "Script As_X");
	}
	auto niawgM = menubar->addMenu ("NIAWG");
	niawgM->addAction ("Reload Default Waveforms_X");
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
	masterSystemsM->addAction ("View or Change Individaul TTL Settings_X");
	masterSystemsM->addAction ("View or Change Individual DAC Settings_X");
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