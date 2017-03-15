
#include "stdafx.h"
#include "MainOptionsControl.h"

void MainOptionsControl::initialize(int& id, POINT& loc, CWnd* parent, std::unordered_map<std::string, CFont*> fonts,
	std::vector<CToolTipCtrl*>& tooltips)
{
	header.ID = id++;
	header.sPos = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	header.Create("MAIN OPTIONS", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, header.sPos, parent, header.ID);
	header.SetFont(fonts["Heading Font"]);
	loc.y += 20;
	if (id != IDC_MAIN_OPTIONS_RANGE_BEGIN)
	{
		throw;
	}
	//
	this->connectToMaster.ID = id++;
	connectToMaster.sPos = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	connectToMaster.Create("Connect to Master Computer?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, connectToMaster.sPos, parent, connectToMaster.ID);
	connectToMaster.SetFont(fonts["Normal Font"]);
	connectToMaster.SetCheck(0);
	currentOptions.connectToMaster = false;
	loc.y += 20;
	//
	this->getVariables.ID = id++;
	getVariables.sPos = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	getVariables.Create("Get Variables from Master Computer?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, getVariables.sPos, parent, getVariables.ID);
	getVariables.SetFont(fonts["Normal Font"]);
	getVariables.SetCheck(0);
	currentOptions.getVariables= false;
	loc.y += 20;
	//
	this->controlIntensity.ID = id++;
	controlIntensity.sPos = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	controlIntensity.Create("Program Intensity?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, controlIntensity.sPos, parent, controlIntensity.ID);
	controlIntensity.SetFont(fonts["Normal Font"]);
	controlIntensity.SetCheck(0);
	currentOptions.programIntensity = false;
	loc.y += 20;
	if (id - 1 != IDC_MAIN_OPTIONS_RANGE_END)
	{
		throw;
	}
	return;
}

bool MainOptionsControl::handleEvent(UINT id, MainWindow* comm)
{
	if (id == this->connectToMaster.ID)
	{
		BOOL checked = connectToMaster.GetCheck();
		if (checked)
		{
			connectToMaster.SetCheck(0);
			this->currentOptions.connectToMaster = false;
		}
		else
		{
			connectToMaster.SetCheck(1);
			this->currentOptions.connectToMaster = true;
		}
		comm->updateConfigurationSavedStatus(false);
	}
	else if (id == this->controlIntensity.ID)
	{
		BOOL checked = controlIntensity.GetCheck();
		if (checked)
		{
			controlIntensity.SetCheck(0);
			this->currentOptions.programIntensity = false;
		}
		else
		{
			controlIntensity.SetCheck(1);
			this->currentOptions.programIntensity = true;
		}
		comm->updateConfigurationSavedStatus(false);
	}
	else if (id == this->getVariables.ID)
	{
		BOOL checked = getVariables.GetCheck();
		if (checked)
		{
			getVariables.SetCheck(0);
			this->currentOptions.getVariables = false;
		}
		else
		{
			getVariables.SetCheck(1);
			this->currentOptions.getVariables = true;
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
