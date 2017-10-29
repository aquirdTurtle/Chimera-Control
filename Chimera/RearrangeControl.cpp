#include "stdafx.h"
#include "RearrangeControl.h"

rearrangeParams RearrangeControl::getParams( )
{
	rearrangeParams tempParams;
	tempParams.active = experimentIncludesRearrangement.GetCheck( );
	tempParams.outputInfo = outputRearrangeEvents.GetCheck( );
	tempParams.outputIndv = outputIndividualEvents.GetCheck( );
	CString tempTxt;
	try
	{
		flashingRateEdit.GetWindowTextA( tempTxt );
		// convert to Hz from MHz
		tempParams.flashingRate = 1e6 * std::stod( str( tempTxt ) );
		moveSpeedEdit.GetWindowTextA( tempTxt );
		// convert to s from ms
		tempParams.moveSpeed = 1e-3 * std::stod( str( tempTxt ) );
		movingBiasEdit.GetWindowTextA( tempTxt );

		tempParams.moveBias = std::stod( str( tempTxt ) );
		
		// convert from ns to s
		deadTimeEdit.GetWindowTextA( tempTxt );
		tempParams.deadTime = std::stod( str(tempTxt) ) * 1e-9;

		staticMovingRatioEdit.GetWindowTextA( tempTxt );
		tempParams.staticMovingRatio = std::stod( str( tempTxt ) );
		
		

	}
	catch ( std::invalid_argument&)
	{
		thrower( "ERROR: Failed to convert rearrangement parameters to correct format! check that the inputs are the "
				 "correct types please." );
	}
	return tempParams;
}


void RearrangeControl::rearrange( int width, int height, fontMap fonts )
{
	header.rearrange(width, height, fonts);
	experimentIncludesRearrangement.rearrange( width, height, fonts );
	flashingRateText.rearrange( width, height, fonts );
	flashingRateEdit.rearrange( width, height, fonts );
	moveSpeedText.rearrange( width, height, fonts );
	moveSpeedEdit.rearrange( width, height, fonts );
	movingBiasText.rearrange( width, height, fonts );
	movingBiasEdit.rearrange( width, height, fonts );
	deadTimeText.rearrange( width, height, fonts );
	deadTimeEdit.rearrange( width, height, fonts );
	staticMovingRatioEdit.rearrange( width, height, fonts );
	staticMovingRatioText.rearrange( width, height, fonts );
	outputRearrangeEvents.rearrange( width, height, fonts );
	outputIndividualEvents.rearrange( width, height, fonts );
}


void RearrangeControl::initialize( int& id, POINT& loc, CWnd* parent, cToolTips& tooltips )
{
	header.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 25 };
	header.Create( "Rearrangement Parameters", WS_CHILD | WS_VISIBLE | ES_READONLY, header.sPos, parent, id++ );
	header.fontType = HeadingFont;

	experimentIncludesRearrangement.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 25 };
	experimentIncludesRearrangement.Create( "Experiment Includes Rearrangement?", WS_CHILD | WS_VISIBLE
											| BS_AUTOCHECKBOX | WS_TABSTOP, experimentIncludesRearrangement.sPos, parent, id++ );

	flashingRateText.sPos = { loc.x, loc.y, loc.x + 240, loc.y + 25 };
	flashingRateText.Create( "Flashing Rate (MHz)", WS_CHILD | WS_VISIBLE | ES_READONLY, flashingRateText.sPos, parent,
							 id++ );
	flashingRateEdit.sPos = { loc.x + 240, loc.y, loc.x + 480, loc.y += 25 };
	flashingRateEdit.Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP, flashingRateEdit.sPos, parent, id++ );
	flashingRateEdit.SetWindowTextA( "1" );

	moveSpeedText.sPos = { loc.x, loc.y, loc.x + 240, loc.y + 25 };
	moveSpeedText.Create( "Move Speed (ms)", WS_CHILD | WS_VISIBLE | ES_READONLY, moveSpeedText.sPos, parent, id++ );
	moveSpeedEdit.sPos = { loc.x + 240, loc.y, loc.x + 480, loc.y += 25 };
	moveSpeedEdit.Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP, moveSpeedEdit.sPos, parent, id++ );
	moveSpeedEdit.SetWindowTextA( "0.06" );

	movingBiasText.sPos = { loc.x, loc.y, loc.x + 240, loc.y + 25 };
	movingBiasText.Create( "Moving Tweezer Bias (/1)", WS_CHILD | WS_VISIBLE | ES_READONLY, movingBiasText.sPos,
						   parent, id++ );
	movingBiasEdit.sPos = { loc.x + 240, loc.y, loc.x + 480, loc.y += 25 };
	movingBiasEdit.Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP, movingBiasEdit.sPos, parent, id++ );
	movingBiasEdit.SetWindowTextA( "0.3" );

	deadTimeText.sPos = { loc.x, loc.y, loc.x + 240, loc.y + 25 };
	deadTimeText.Create( "Dead Time (ns)", WS_CHILD | WS_VISIBLE | ES_READONLY, deadTimeText.sPos,
						 parent, id++ );
	deadTimeEdit.sPos = { loc.x + 240, loc.y, loc.x + 480, loc.y += 25 };
	deadTimeEdit.Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP, deadTimeEdit.sPos, parent, id++ );
	deadTimeEdit.SetWindowTextA( "0" );

	staticMovingRatioText.sPos = { loc.x, loc.y, loc.x + 240, loc.y + 25 };
	staticMovingRatioText.Create( "Static / Moving Ratio", WS_CHILD | WS_VISIBLE | ES_READONLY,
								  staticMovingRatioText.sPos, parent, id++ );

	staticMovingRatioEdit.sPos = { loc.x + 240, loc.y, loc.x + 480, loc.y += 25 };
	staticMovingRatioEdit.Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP, staticMovingRatioEdit.sPos, parent, id++ );
	staticMovingRatioEdit.SetWindowTextA( "1" );

	outputRearrangeEvents.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 25 };
	outputRearrangeEvents.Create( "Output Event Info", NORM_CHECK_OPTIONS, outputRearrangeEvents.sPos, parent, id++ );

	outputIndividualEvents.sPos = { loc.x, loc.y, loc.x + 480, loc.y += 25 };
	outputIndividualEvents.Create( "Output Individual Events to Experiment Status?", NORM_CHECK_OPTIONS,
								   outputIndividualEvents.sPos, parent, id++ );

}


void RearrangeControl::handleOpenConfig( std::ifstream& openFile, int versionMajor, int versionMinor )
{
	if ( (versionMajor == 2 &&  versionMinor < 1) || versionMajor < 2)
	{
		return;
	}
	ProfileSystem::checkDelimiterLine( openFile, "REARRANGEMENT_INFORMATION" );
	rearrangeParams info;
	openFile >> info.active;
	openFile >> info.flashingRate;
	openFile >> info.moveBias;
	openFile >> info.moveSpeed;

	if ( (versionMajor == 2 && versionMinor < 3) || versionMajor < 2 )
	{
		std::string garbage;
 		openFile >> garbage;
		openFile >> garbage;
	}
	if ( (versionMajor == 2 && versionMinor > 2) || versionMajor > 2 )
	{
		openFile >> info.deadTime;
		openFile >> info.staticMovingRatio;		
	}
	else
	{
		info.deadTime = 0;
		info.staticMovingRatio = 1;
	}
	if ( (versionMajor == 2 && versionMinor > 5) || versionMajor > 2 )
	{
		openFile >> info.outputInfo;
	}
	else
	{
		info.outputInfo = false;
	}
	if ( (versionMajor == 2 && versionMinor > 10) || versionMajor > 2 )
	{
		openFile >> info.outputIndv;
	}
	else
	{
		info.outputIndv = false;
	}
	setParams( info );
	ProfileSystem::checkDelimiterLine( openFile, "END_REARRANGEMENT_INFORMATION" );
}


void RearrangeControl::handleNewConfig( std::ofstream& newFile )
{
	newFile << "REARRANGEMENT_INFORMATION\n";
	newFile << 0 << "\n";
	newFile << 1 << "\n";
	newFile << 1e-3*0.3 << "\n";
	newFile << 1e6*0.06 << "\n";
	newFile << 75e-9 << "\n";
	newFile << "1\n";
	newFile << "0\n";
	newFile << "0\n";
	newFile << "END_REARRANGEMENT_INFORMATION\n";
}


void RearrangeControl::handleSaveConfig( std::ofstream& saveFile )
{
 	saveFile << "REARRANGEMENT_INFORMATION\n";
 	rearrangeParams info = getParams( );
 	saveFile << info.active << "\n";
 	saveFile << info.flashingRate << "\n";
 	saveFile << info.moveBias << "\n";
	saveFile << info.moveSpeed << "\n";
	saveFile << info.deadTime << "\n";
	saveFile << info.staticMovingRatio << "\n";
	saveFile << info.outputInfo << "\n";
	saveFile << info.outputIndv << "\n";
	saveFile << "END_REARRANGEMENT_INFORMATION\n";
}


void RearrangeControl::setParams( rearrangeParams params )
{
	experimentIncludesRearrangement.SetCheck( params.active );
	// convert back to MHz from Hz
	flashingRateEdit.SetWindowTextA( cstr(1e-6*params.flashingRate) );
	movingBiasEdit.SetWindowTextA( cstr( params.moveBias ) );
	// convert back to ms from s
	moveSpeedEdit.SetWindowTextA( cstr( 1e3*params.moveSpeed ) );
	outputRearrangeEvents.SetCheck( params.outputInfo );
	// convert back to us
	deadTimeEdit.SetWindowTextA( cstr( params.deadTime * 1e9) );
	staticMovingRatioEdit.SetWindowTextA( cstr( params.staticMovingRatio ) );
	
	outputRearrangeEvents.SetCheck( params.outputInfo );
	outputIndividualEvents.SetCheck( params.outputIndv );

}
