
#include "stdafx.h"
#include "MainOptionsControl.h"

void MainOptionsControl::initialize( int& id, POINT& loc, CWnd* parent, cToolTips& tooltips )
{
	header.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	header.Create( "MAIN OPTIONS", NORM_HEADER_OPTIONS, header.sPos, parent, id++ );
	header.fontType = HeadingFont;
	randomizeRepsButton.sPos = { loc.x, loc.y, loc.x + 480 , loc.y += 25 };
	randomizeRepsButton.Create( "Randomize Repetitions?", NORM_CHECK_OPTIONS, randomizeRepsButton.sPos, parent, id++ );
	randomizeRepsButton.EnableWindow( false );
	randomizeVariationsButton.sPos = { loc.x, loc.y, loc.x + 480 , loc.y += 25 };
	randomizeVariationsButton.Create( "Randomize Variations?", NORM_CHECK_OPTIONS, randomizeVariationsButton.sPos, 
									  parent, id++ );
	atomThresholdForSkipText.sPos = { loc.x, loc.y, loc.x + 240 , loc.y + 25 };
	atomThresholdForSkipText.Create("Atom Threshold for skip:", NORM_STATIC_OPTIONS, atomThresholdForSkipText.sPos,
									 parent, id++ );
	atomThresholdForSkipEdit.sPos = { loc.x + 240, loc.y, loc.x + 480 , loc.y += 25 };
	atomThresholdForSkipEdit.Create( NORM_EDIT_OPTIONS, atomThresholdForSkipEdit.sPos, parent, id++ );

	currentOptions.randomizeReps = false;
	currentOptions.randomizeVariations = true;
}

void MainOptionsControl::rearrange( int width, int height, fontMap fonts )
{
	header.rearrange( width, height, fonts );
	randomizeRepsButton.rearrange( width, height, fonts );
	randomizeVariationsButton.rearrange( width, height, fonts );
	atomThresholdForSkipText.rearrange( width, height, fonts );
	atomThresholdForSkipEdit.rearrange( width, height, fonts );
}


void MainOptionsControl::handleNewConfig( std::ofstream& newFile )
{
	newFile << "MAIN_OPTIONS\n";
	newFile << 0 << "\n";
	// default is to randomize variations.
	newFile << 1 << "\n"; 
	newFile << -1 << "\n";
	newFile << "END_MAIN_OPTIONS\n";
}


void MainOptionsControl::handleSaveConfig(std::ofstream& saveFile)
{
	saveFile << "MAIN_OPTIONS\n";
	saveFile << randomizeRepsButton.GetCheck() << "\n";
	saveFile << randomizeVariationsButton.GetCheck() << "\n";
	CString txt;
	atomThresholdForSkipEdit.GetWindowTextA( txt );
	saveFile << txt << "\n";
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
	if ( (versionMajor == 2 && versionMinor > 9) || versionMajor > 2)
	{
		std::string txt;
		openFile >> txt;
		try
		{
			currentOptions.atomThresholdForSkip = std::stoul( txt );
		}
		catch ( std::invalid_argument& )
		{
			thrower( "ERROR: atom threshold for skip failed to convert to an unsigned long!" );
		}
		atomThresholdForSkipEdit.SetWindowTextA( cstr(txt) );
	}
	else
	{
		currentOptions.atomThresholdForSkip = UINT_MAX;
	}
 	ProfileSystem::checkDelimiterLine(openFile, "END_MAIN_OPTIONS");
}


mainOptions MainOptionsControl::getOptions()
{
	currentOptions.randomizeReps = randomizeRepsButton.GetCheck();
	currentOptions.randomizeVariations = randomizeVariationsButton.GetCheck();
	CString txt;
	atomThresholdForSkipEdit.GetWindowTextA( txt );
	try
	{
		currentOptions.atomThresholdForSkip = std::stoul( str( txt ) );
	}
	catch ( std::invalid_argument& )
	{
		thrower( "ERROR: failed to convert atom threshold for skip to an unsigned long!" );
	}
	return currentOptions;
}

