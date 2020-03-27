#include "stdafx.h"
#include "NiawgSystem.h"
#include "ConfigurationSystems/ProfileSystem.h"

NiawgSystem::NiawgSystem (DoRows::which trigRow, UINT trigNumber, bool safemode) : core (trigRow, trigNumber, safemode)
{};

void NiawgSystem::initialize (int& id, POINT& loc, CWnd* parent, cToolTips& tooltips)
{
	niawgHeader.sPos = { loc.x, loc.y, loc.x + 640, loc.y += 30 };
	niawgHeader.Create ("NIAWG SYSTEM", NORM_HEADER_OPTIONS, niawgHeader.sPos, parent, id++);
	niawgHeader.fontType = fontTypes::HeadingFont;

	controlNiawg.sPos = { loc.x, loc.y, loc.x + 640, loc.y += 25 };
	controlNiawg.Create ("Control System?", NORM_CHECK_OPTIONS, controlNiawg.sPos, parent, IDC_CONTROL_NIAWG_CHECK );
	controlNiawg.SetCheck (true);

	rearrangeCtrl.initialize (id, loc, parent, tooltips);
	niawgScript.initialize (640, 400, loc, tooltips, parent, id, "NIAWG", "NIAWG Script", { IDC_NIAWG_FUNCTION_COMBO,
							IDC_NIAWG_EDIT }, _myRGBs["Interactable-Bkgd"]);
}


void NiawgSystem::updateWindowEnabled ()
{
	auto enabled = controlNiawg.GetCheck ();
	niawgScript.setEnabled (enabled, enabled);
	rearrangeCtrl.setEnabled (enabled);
}


void NiawgSystem::rearrange (UINT width, UINT height, fontMap fonts)
{
	niawgHeader.rearrange (width, height, fonts);
	controlNiawg.rearrange (width, height, fonts);
	niawgScript.rearrange (width, height, fonts);
	rearrangeCtrl.rearrange (width, height, fonts);
}

void NiawgSystem::handleSaveConfig (ConfigStream& saveFile)
{
	saveFile << "NIAWG_INFORMATION\n";
	saveFile << "/*Control Niawg:*/ "<< controlNiawg.GetCheck () << "\n";
	saveFile << "END_NIAWG_INFORMATION\n";
	rearrangeCtrl.handleSaveConfig (saveFile);
}

bool NiawgSystem::getControlNiawgFromConfig ( ConfigStream& openfile, Version ver )
{
	if (ver < Version ("4.12")) { return true; }
	bool opt;
	if (ver >= Version ("5.0")) {
		openfile >> opt;
	}
	else
	{
		openfile.get ();
		opt = bool(openfile.get ());
	}
	return opt;
}

void NiawgSystem::handleOpenConfig (ConfigStream& openfile, Version ver)
{
	controlNiawg.SetCheck ( ProfileSystem::stdGetFromConfig (openfile, "NIAWG_INFORMATION",
							NiawgSystem::getControlNiawgFromConfig, Version ("4.12")) );
	ProfileSystem::standardOpenConfig (openfile, "REARRANGEMENT_INFORMATION", &rearrangeCtrl);
}
