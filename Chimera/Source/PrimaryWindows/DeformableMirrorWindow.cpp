//Created by Max Kolanz
#include "stdafx.h"
#include "DeformableMirrorWindow.h"
#include "MainWindow.h"
#include "GeneralUtilityFunctions/commonFunctions.h"
#include "ExcessDialogs/openWithExplorer.h"
#include "ExcessDialogs/saveWithExplorer.h"
#include "ConfigurationSystems/ProfileSystem.h"

IMPLEMENT_DYNAMIC (DeformableMirrorWindow, IChimeraWindow)

BEGIN_MESSAGE_MAP (DeformableMirrorWindow, IChimeraWindow)
	ON_WM_SIZE ()
	ON_WM_CTLCOLOR ()
	ON_COMMAND (IDC_DM_PROGRAMNOW, &handleProgramDmNow)
	ON_COMMAND (IDC_DM_ADD_ZERNIKE + 15, &handleAddAbberations)
	ON_CBN_SELENDOK (IDC_DM_PROFILE_COMBO, &handleNewDmProfile)
	ON_CONTROL_RANGE (EN_CHANGE, IDC_DM_EDIT_START, IDC_DM_EDIT_END, &DeformableMirrorWindow::handlePistonChange)
END_MESSAGE_MAP ()

DeformableMirrorWindow::DeformableMirrorWindow() : IChimeraWindow(), dm(DM_SERIAL, DM_SAFEMODE)
{}

BOOL DeformableMirrorWindow::OnInitDialog() 
{
	SetWindowText ("Deformable Mirror Window");
	POINT pos = { 0,0 };
	int id = 1000;
	UINT ID = IDC_DM_PROGRAMNOW;
	statBox.initialize(pos, id, this, 480, toolTips);
	dm.initialize(pos, this, dm.getActNum(), DM_SERIAL, 65, ID);
	return IChimeraWindow::OnInitDialog ();
}

void DeformableMirrorWindow::OnSize(UINT nType, int cx, int cy)
{
	SetRedraw(false);
	statBox.rearrange(cx, cy, mainWin->getFonts());
	dm.rearrange(cx, cy, mainWin->getFonts());
	SetRedraw();
	RedrawWindow();
}

void DeformableMirrorWindow::OnPaint()
{
	CDialog::OnPaint();
	if (!mainWin->masterIsRunning())
	{
		CRect size;
		GetClientRect(&size);
		CDC* cdc = GetDC();
		// for some reason I suddenly started needing to do this. I know that memDC redraws the background, but it used to 
		// work without this and I don't know what changed. I used to do:
		cdc->SetBkColor(_myRGBs["Main-Bkgd"]);
		long width = size.right - size.left, height = size.bottom - size.top;
		// each dc gets initialized with the rect for the corresponding plot. That way, each dc only overwrites the area 
		// for a single plot.
		Mirror->setCurrentDims(width, height);
		Mirror->drawPlot(cdc, _myBrushes["Main-Bkgd"], _myBrushes["Interactable-Bkgd"]);
		ReleaseDC(cdc);
	}
}

HBRUSH DeformableMirrorWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH result;	
	result = dm.handleColorMessage(pWnd, pDC);
	if (result != NULL)
	{
		return result;
	}
	return IChimeraWindow::OnCtlColor (pDC, pWnd, nCtlColor);
}

void DeformableMirrorWindow::handleProgramDmNow() {
	dm.ProgramNow();
}

void DeformableMirrorWindow::handleNewDmProfile() {
	try {
		dm.loadProfile();
	}
	catch(Error &err){
		reportErr(err.trace());
	}
}

void DeformableMirrorWindow::handlePistonChange(UINT id) {
	dm.reColor(id);
}

void DeformableMirrorWindow::handleAddAbberations() {
	try 
	{
		dm.add_Changes();
	}
	catch (Error & err) 
	{
		reportErr(err.trace());
	}
}

void DeformableMirrorWindow::windowOpenConfig(ConfigStream& configFile) 
{
	try 
	{
		if (configFile.ver >= Version("4.7"))
		{
			DMOutputForm form;
			ProfileSystem::stdGetFromConfig (configFile, dm.getCore(), form);
			dm.setCoreInfo(form);
			dm.openConfig();
		}
	}
	catch (Error&)
	{
		throwNested("Auxiliary Window failed to read parameters from the configuration file.");
	}
}

void DeformableMirrorWindow::windowSaveConfig(ConfigStream& newFile) 
{
	dm.handleSaveConfig(newFile);
}

void DeformableMirrorWindow::fillExpDeviceList (DeviceList& list)
{
	list.list.push_back (dm.getCore());
}