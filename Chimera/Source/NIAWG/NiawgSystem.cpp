#include "stdafx.h"
#include "NiawgSystem.h"
#include "ConfigurationSystems/ConfigSystem.h"

NiawgSystem::NiawgSystem (DoRows::which trigRow, unsigned trigNumber, bool safemode, IChimeraQtWindow* parent) : 
	IChimeraSystem(parent), niawgScript (parent), core (trigRow, trigNumber, safemode)
{};

void NiawgSystem::initialize (QPoint& loc, IChimeraQtWindow* parent){
	auto& px = loc.rx (), & py = loc.ry ();
	niawgHeader = new QLabel ("NIAWG SYSTEM", parent);
	niawgHeader->setGeometry (px, py, 640, 30);

	controlNiawg = new QCheckBox ("Control System", parent);
	controlNiawg->setGeometry (px, py += 30, 320, 25);
	parent->connect (controlNiawg, &QCheckBox::stateChanged, [this]() {updateWindowEnabled (); });

	debugLvlLabel = new QLabel("Debug Level:", parent);
	debugLvlLabel->setGeometry (px+320, py, 160, 25);
	
	debugLvlEdit = new QLineEdit("0",parent);
	debugLvlEdit->setGeometry (px+480, py, 160, 25);

	py += 25;
	rearrangeCtrl.initialize (loc, parent);
	niawgScript.initialize ( 640, 660, loc, parent, "NIAWG", "NIAWG Script" );
} 


void NiawgSystem::updateWindowEnabled (){
	auto enabled = controlNiawg->isChecked ();
	niawgScript.setEnabled (enabled, enabled);
	rearrangeCtrl.setEnabled (enabled);
}


void NiawgSystem::handleSaveConfig (ConfigStream& saveFile){
	saveFile << "NIAWG_INFORMATION\n";
	saveFile << "/*Control Niawg:*/ "<< controlNiawg->isChecked() << "\n";
	saveFile << "/*Niawg Debug Level:*/ " << str(debugLvlEdit->text ()) << "\n";
	saveFile << "END_NIAWG_INFORMATION\n";
	rearrangeCtrl.handleSaveConfig (saveFile);
}

void NiawgSystem::handleOpenConfig (ConfigStream& openfile){
	niawgConfigSettings settings;
	ConfigSystem::stdGetFromConfig (openfile, core, settings, Version ("4.12"));
	controlNiawg->setChecked (settings.control);
	debugLvlEdit->setText (qstr (settings.debugLvl));
	ConfigSystem::standardOpenConfig (openfile, "REARRANGEMENT_INFORMATION", &rearrangeCtrl);
}
