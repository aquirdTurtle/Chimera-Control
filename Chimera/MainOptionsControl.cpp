
#include "stdafx.h"
#include "MainOptionsControl.h"

void MainOptionsControl::rearrange(int width, int height, fontMap fonts)
{
	header.rearrange("", "", width, height, fonts);
	controlIntensity.rearrange("", "", width, height, fonts);
}


void MainOptionsControl::handleSaveConfig(std::ofstream& saveFile)
{
	saveFile << "MAIN_OPTIONS\n";
	saveFile << currentOptions.programIntensity << "\n";
	saveFile << "END_MAIN_OPTIONS\n";
}


void MainOptionsControl::handleOpenConfig(std::ifstream& openFile, double version)
{
	ProfileSystem::checkDelimiterLine(openFile, "MAIN_OPTIONS");
	openFile >> currentOptions.programIntensity;
	ProfileSystem::checkDelimiterLine(openFile, "END_MAIN_OPTIONS");
}


void MainOptionsControl::initialize(int& id, POINT& loc, CWnd* parent, fontMap fonts, cToolTips& tooltips)
{
	header.sPos = { loc.x, loc.y, loc.x + 480, loc.y + 20 };
	header.Create("MAIN OPTIONS", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, header.sPos, parent, id++ );
	header.SetFont(fonts["Heading Font"]);
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
	if (id == controlIntensity.GetDlgCtrlID())
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
