// created by Mark O. Brown

#include "stdafx.h"
#include "MainOptionsControl.h"
#include "ConfigurationSystems/ProfileSystem.h"
#include <boost/lexical_cast.hpp>

void MainOptionsControl::initialize( int& id, POINT& loc, CWnd* parent, cToolTips& tooltips )
{
	header.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 20 };
	header.Create( "MAIN OPTIONS", NORM_HEADER_OPTIONS, header.sPos, parent, id++ );
	header.fontType = fontTypes::HeadingFont;
	randomizeRepsButton.sPos = { loc.x, loc.y, loc.x + 480 , loc.y += 25 };
	randomizeRepsButton.Create( "Randomize Repetitions?", NORM_CHECK_OPTIONS, randomizeRepsButton.sPos, parent, id++ );
	randomizeRepsButton.EnableWindow( false );

	randomizeVariationsButton.sPos = { loc.x, loc.y, loc.x + 480 , loc.y += 25 };
	randomizeVariationsButton.Create( "Randomize Variations?", NORM_CHECK_OPTIONS, randomizeVariationsButton.sPos, 
									  parent, id++ );

	atomThresholdForSkipText.sPos = { loc.x, loc.y, loc.x + 240 , loc.y + 25 };
	atomThresholdForSkipText.Create("Atom Threshold for load-skip:", NORM_STATIC_OPTIONS, atomThresholdForSkipText.sPos,
									 parent, id++ );
	atomThresholdForSkipEdit.sPos = { loc.x + 240, loc.y, loc.x + 480 , loc.y += 25 };
	atomThresholdForSkipEdit.Create( NORM_EDIT_OPTIONS, atomThresholdForSkipEdit.sPos, parent, id++ );
	atomThresholdForSkipEdit.SetWindowText("-1");

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

void MainOptionsControl::handleSaveConfig(ConfigStream& saveFile)
{
	saveFile << "MAIN_OPTIONS"
			 << "\n/*Randomize Reps?*/ " << randomizeRepsButton.GetCheck() 
			 << "\n/*Randomize Variations?*/ " << randomizeVariationsButton.GetCheck();
	CString txt;
	atomThresholdForSkipEdit.GetWindowTextA( txt );
	saveFile << "\n/*Atom Threshold for Load Skip*/ " << txt;
	saveFile << "\nEND_MAIN_OPTIONS\n";
}

mainOptions MainOptionsControl::getSettingsFromConfig (ConfigStream& openFile )
{
	mainOptions options;
	if ( openFile.ver < Version ( "2.1" ) )
	{
		// rearrange option used to be stored here.
		std::string garbage;
		openFile >> garbage;
	}
	openFile >> options.randomizeReps;
	openFile >> options.randomizeVariations;
	if (openFile.ver > Version ( "2.9" ) )
	{
		std::string txt;
		openFile >> txt;
		try
		{
			options.atomSkipThreshold = boost::lexical_cast<unsigned long>( txt );
		}
		catch ( boost::bad_lexical_cast& )
		{
			errBox ( "atom threshold for load skip failed to convert to an unsigned long! The code will force "
					 "the threshold to the maximum threshold." );
			options.atomSkipThreshold = -1;
		}
	}
	else
	{
		options.atomSkipThreshold = UINT_MAX;
	}
	return options;
}


void MainOptionsControl::setOptions ( mainOptions opts )
{
	currentOptions = opts;
	randomizeRepsButton.SetCheck ( currentOptions.randomizeReps );
	randomizeVariationsButton.SetCheck ( currentOptions.randomizeVariations );
	atomThresholdForSkipEdit.SetWindowTextA ( cstr ( currentOptions.atomSkipThreshold ) );
}


mainOptions MainOptionsControl::getOptions()
{
	currentOptions.randomizeReps = randomizeRepsButton.GetCheck();
	currentOptions.randomizeVariations = randomizeVariationsButton.GetCheck();
	CString txt;
	atomThresholdForSkipEdit.GetWindowTextA( txt );
	try
	{
		currentOptions.atomSkipThreshold = boost::lexical_cast<unsigned long>( str( txt ) );
	}
	catch ( boost::bad_lexical_cast& )
	{
		throwNested ( "failed to convert atom threshold for load-skip to an unsigned long!" );
	}
	return currentOptions;
}

