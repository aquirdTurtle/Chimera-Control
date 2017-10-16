
#include "stdafx.h"
#include "MainOptionsControl.h"

void MainOptionsControl::initialize( int& id, POINT& loc, CWnd* parent, cToolTips& tooltips )
{
	header.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	header.Create( "MAIN OPTIONS", NORM_HEADER_OPTIONS, header.sPos, parent, id++ );
	header.fontType = HeadingFont;
	randomizeRepsButton.sPos = { loc.x, loc.y, loc.x + 480 , loc.y += 25 };
	randomizeRepsButton.Create( "Randomize Repetitions?", NORM_CHECK_OPTIONS, randomizeRepsButton.sPos, 
									   parent, id++ );
	randomizeRepsButton.EnableWindow( false );
	randomizeVariationsButton.sPos = { loc.x, loc.y, loc.x + 480 , loc.y += 25 };
	randomizeVariationsButton.Create( "Randomize Variations?", NORM_CHECK_OPTIONS, randomizeVariationsButton.sPos, 
									  parent, id++ );
	currentOptions.randomizeReps = false;
	currentOptions.randomizeVariations = true;
}

void MainOptionsControl::rearrange( int width, int height, fontMap fonts )
{
	header.rearrange( width, height, fonts );
	randomizeRepsButton.rearrange( width, height, fonts );
	randomizeVariationsButton.rearrange( width, height, fonts );
}


void MainOptionsControl::handleNewConfig( std::ofstream& newFile )
{
	newFile << "MAIN_OPTIONS\n";
	newFile << 0 << "\n";
	// default is to randomize variations.
	newFile << 1 << "\n"; 
	newFile << "END_MAIN_OPTIONS\n";
}


void MainOptionsControl::handleSaveConfig(std::ofstream& saveFile)
{
	saveFile << "MAIN_OPTIONS\n";
	saveFile << randomizeRepsButton.GetCheck() << "\n";
	saveFile << randomizeVariationsButton.GetCheck() << "\n";
	saveFile << "END_MAIN_OPTIONS\n";
}


void MainOptionsControl::handleOpenConfig(std::ifstream& openFile, int versionMajor, int versionMinor )
{
 	ProfileSystem::checkDelimiterLine(openFile, "MAIN_OPTIONS");
	if ( (versionMajor == 2 && versionMinor < 1) || versionMajor < 2)
	{
		// rearrange option used to be stored here.
		std::string garbage;
		openFile >> garbage;
	}
	openFile >> currentOptions.randomizeReps;
	randomizeRepsButton.SetCheck( currentOptions.randomizeReps );
	openFile >> currentOptions.randomizeVariations;
	randomizeVariationsButton.SetCheck( currentOptions.randomizeVariations );
 	ProfileSystem::checkDelimiterLine(openFile, "END_MAIN_OPTIONS");
}


mainOptions MainOptionsControl::getOptions()
{
	currentOptions.randomizeReps = randomizeRepsButton.GetCheck();
	currentOptions.randomizeVariations = randomizeVariationsButton.GetCheck();
	return currentOptions;
}

