#include "stdafx.h"
#include "RearrangeControl.h"

rearrangeParams RearrangeControl::getParams( )
{
	rearrangeParams tempParams;
	tempParams.active = experimentIncludesRearrangement.GetCheck( );
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
		
		deadTimeEdit.GetWindowTextA( tempTxt );
		tempParams.deadTime = std::stod( str(tempTxt) );
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
	deadTimeText.Create( "Dead Time (us)", WS_CHILD | WS_VISIBLE | ES_READONLY, deadTimeText.sPos,
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
}


void RearrangeControl::handleOpenConfig( std::ifstream& openFile, double version)
{
	if ( version < 2.09999 )
	{
		return;
	}
	ProfileSystem::checkDelimiterLine( openFile, "REARRANGEMENT_INFORMATION" );
	rearrangeParams info;
	openFile >> info.active;
	openFile >> info.flashingRate;
	openFile >> info.moveBias;
	openFile >> info.moveSpeed;

	if ( version < 2.3 )
	{
		std::string garbage;
 		openFile >> garbage;
		openFile >> garbage;
	}
	if ( version > 2.2 )
	{
		openFile >> info.deadTime;
		openFile >> info.staticMovingRatio;		
	}
	else
	{
		info.deadTime = 0;
		info.staticMovingRatio = 1;
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
	newFile << "0" << "\n";
	newFile << "1" << "\n";
	newFile << "END_REARRANGEMENT_INFORMATION\n";
}


void RearrangeControl::handleSaveConfig( std::ofstream& newFile )
{
 	newFile << "REARRANGEMENT_INFORMATION\n";
 	rearrangeParams info = getParams( );
 	newFile << info.active << "\n";
 	newFile << info.flashingRate << "\n";
 	newFile << info.moveBias << "\n";
	newFile << info.moveSpeed << "\n";
	newFile << info.deadTime << "\n";
	newFile << info.staticMovingRatio << "\n";
	newFile << "END_REARRANGEMENT_INFORMATION\n";
}


void RearrangeControl::setParams( rearrangeParams params )
{
	experimentIncludesRearrangement.SetCheck( params.active );
	flashingRateEdit.SetWindowTextA( cstr(1e-6*params.flashingRate) );
	movingBiasEdit.SetWindowTextA( cstr( params.moveBias ) );
	moveSpeedEdit.SetWindowTextA( cstr( 1e3*params.moveSpeed ) );
	

}
