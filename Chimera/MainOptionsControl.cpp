
#include "stdafx.h"
#include "MainOptionsControl.h"

void MainOptionsControl::rearrange(int width, int height, fontMap fonts)
{
	header.rearrange("", "", width, height, fonts);
	connectToMaster.rearrange("", "", width, height, fonts);
	getVariables.rearrange("", "", width, height, fonts);
	controlIntensity.rearrange("", "", width, height, fonts);
}

void MainOptionsControl::initialize(int& id, POINT& loc, CWnd* parent, fontMap fonts,
	std::vector<CToolTipCtrl*>& tooltips)
{
	header.sPos = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	header.Create("MAIN OPTIONS", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, header.sPos, parent, id++ );
	header.SetFont(fonts["Heading Font"]);
	loc.y += 20;
	
	//
	connectToMaster.sPos = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	connectToMaster.Create("Connect to Master Computer?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, 
						   connectToMaster.sPos, parent, id++ );
	connectToMaster.SetFont(fonts["Normal Font"]);
	currentOptions.connectToMaster = false;
	loc.y += 20;
	idVerify(connectToMaster, IDC_MAIN_OPTIONS_RANGE_BEGIN);
	//
	getVariables.sPos = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	getVariables.Create("Get Variables from Master Computer?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, 
						getVariables.sPos, parent, id++ );
	getVariables.SetFont(fonts["Normal Font"]);
	currentOptions.getVariables= false;
	loc.y += 20;
	//
	controlIntensity.sPos = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	controlIntensity.Create("Program Intensity?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, controlIntensity.sPos, 
							parent, id++);
	controlIntensity.SetFont(fonts["Normal Font"]);
	currentOptions.programIntensity = false;
	loc.y += 20;	
	idVerify(controlIntensity, IDC_MAIN_OPTIONS_RANGE_END);
}

bool MainOptionsControl::handleEvent(UINT id, MainWindow* comm)
{
	if (id == connectToMaster.GetDlgCtrlID())
	{
		BOOL checked = connectToMaster.GetCheck();
		if (checked)
		{
			connectToMaster.SetCheck(0);
			currentOptions.connectToMaster = false;
		}
		else
		{
			connectToMaster.SetCheck(1);
			currentOptions.connectToMaster = true;
		}
		comm->updateConfigurationSavedStatus(false);
	}
	else if (id == controlIntensity.GetDlgCtrlID())
	{
		BOOL checked = controlIntensity.GetCheck();
		if (checked)
		{
			controlIntensity.SetCheck(0);
			currentOptions.programIntensity = false;
		}
		else
		{
			controlIntensity.SetCheck(1);
			currentOptions.programIntensity = true;
		}
		comm->updateConfigurationSavedStatus(false);
	}
	else if (id == getVariables.GetDlgCtrlID())
	{
		BOOL checked = getVariables.GetCheck();
		if (checked)
		{
			getVariables.SetCheck(0);
			currentOptions.getVariables = false;
		}
		else
		{
			getVariables.SetCheck(1);
			currentOptions.getVariables = true;
		}
		comm->updateConfigurationSavedStatus(false);
	}
	return TRUE;
}

mainOptions MainOptionsControl::getOptions()
{
	return currentOptions;
}

void MainOptionsControl::setOptions(mainOptions options)
{
	// todo
}
