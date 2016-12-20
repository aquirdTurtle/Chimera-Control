
#include "stdafx.h"
#include "MainOptionsControl.h"

void MainOptionsControl::initialize(int& id, POINT& loc, CWnd* parent)
{
	header.ID = id++;
	header.position = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	header.Create("MAIN OPTIONS", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, header.position, parent, header.ID);
	header.SetFont(&eHeadingFont);
	loc.y += 20;
	if (id != IDC_MAIN_OPTIONS_RANGE_BEGIN)
	{
		throw;
	}
	//
	this->connectToMaster.ID = id++;
	connectToMaster.position = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	connectToMaster.Create("Connect to Master Computer?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, connectToMaster.position, parent, connectToMaster.ID);
	connectToMaster.SetFont(&eNormalFont);
	connectToMaster.SetCheck(0);
	currentOptions.connectToMaster = false;
	loc.y += 20;
	//
	this->getVariables.ID = id++;
	getVariables.position = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	getVariables.Create("Get Variables from Master Computer?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, getVariables.position, parent, getVariables.ID);
	getVariables.SetFont(&eNormalFont);
	getVariables.SetCheck(0);
	currentOptions.getVariables= false;
	loc.y += 20;
	//
	this->controlIntensity.ID = id++;
	controlIntensity.position = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	controlIntensity.Create("Program Intensity?", WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_RIGHT, controlIntensity.position, parent, controlIntensity.ID);
	controlIntensity.SetFont(&eNormalFont);
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
	return this->currentOptions;
}

void MainOptionsControl::setOptions(mainOptions options)
{
	return;
}
