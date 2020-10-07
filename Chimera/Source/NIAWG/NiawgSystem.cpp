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
	controlNiawg->setGeometry (px, py += 30, 640, 25);
	parent->connect (controlNiawg, &QCheckBox::stateChanged, [this]() {updateWindowEnabled (); });
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
	saveFile << "END_NIAWG_INFORMATION\n";
	rearrangeCtrl.handleSaveConfig (saveFile);
}

void NiawgSystem::handleOpenConfig (ConfigStream& openfile){
	bool controlOpt;
	ConfigSystem::stdGetFromConfig (openfile, core, controlOpt, Version ("4.12"));
	controlNiawg->setChecked ( controlOpt );
	ConfigSystem::standardOpenConfig (openfile, "REARRANGEMENT_INFORMATION", &rearrangeCtrl);
}
