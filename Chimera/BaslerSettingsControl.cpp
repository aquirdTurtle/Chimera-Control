#include "stdafx.h"
#include "BaslerSettingsControl.h"
#include "Version.h"
#include "constants.h"
#include "ProfileSystem.h"

BaslerSettingsControl::BaslerSettingsControl ( )
{}


void BaslerSettingsControl::initialize ( POINT& pos, int& id, CWnd* parent, int picWidth, int picHeight, POINT cameraDims )
{
	int width = 300;
	lastTime = 0;
	statusText.sPos = { pos.x, pos.y, pos.x + width, pos.y += 50 };
	statusText.Create ( "Camera Status: IDLE", WS_CHILD | WS_VISIBLE | WS_BORDER, statusText.sPos, parent, id++ );
	////
	repText.sPos = { pos.x, pos.y, pos.x + 200, pos.y + 25 };
	repText.Create ( "Total Picture Number:", WS_CHILD | WS_VISIBLE, repText.sPos, parent, id++ );
	//
	repEdit.sPos = { pos.x + 200, pos.y, pos.x + 300, pos.y += 25 };
	repEdit.Create ( WS_CHILD | WS_VISIBLE, repEdit.sPos, parent, IDC_BASLER_REPETITIONS_EDIT );
	repEdit.SetWindowTextA ( "100" );

	cameraMode.sPos = { pos.x, pos.y, pos.x + 300, pos.y + 100 };
	cameraMode.Create ( WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SORT, cameraMode.sPos, parent,
						IDC_BASLER_CAMERA_MODE_COMBO );
	pos.y += 25;
	cameraMode.AddString ( "Finite-Acquisition" );
	cameraMode.AddString ( "Continuous-Acquisition" );
	cameraMode.SelectString ( 0, "Continuous-Acquisition" );
	repEdit.EnableWindow ( false );

	exposureText.sPos = { pos.x, pos.y, pos.x + 200, pos.y + 25 };
	std::string exposureTimeText;
	#ifdef USB_CAMERA
	exposureTimeText = "Exposure Time (us):";
	#elif defined FIREWIRE_CAMERA
	exposureTimeText = "Raw Time (# X 20 = us):";
	#endif
	exposureText.Create ( exposureTimeText.c_str ( ), WS_CHILD | WS_VISIBLE, exposureText.sPos, parent, id++ );

	exposureEdit.sPos = { pos.x + 200, pos.y, pos.x + 300, pos.y += 25 };
	exposureEdit.Create ( WS_CHILD | WS_VISIBLE, exposureEdit.sPos, parent, id++ );
	exposureEdit.SetWindowTextA ( "1000" );

	exposureModeCombo.sPos = { pos.x, pos.y, pos.x + 300, pos.y + 100 };
	exposureModeCombo.Create ( WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SORT, exposureModeCombo.sPos, parent,
							   IDC_BASLER_EXPOSURE_MODE_COMBO );

	pos.y += 25;
	exposureModeCombo.AddString ( "Auto-Exposure-Continuous" );
	exposureModeCombo.AddString ( "Auto-Exposure-Off" );
	exposureModeCombo.AddString ( "Auto-Exposure-Once" );
	exposureModeCombo.SelectString ( 0, "Auto Exposure Off" );

	/// image dimensions
	//
	leftText.sPos = { pos.x, pos.y, pos.x + width / 3, pos.y + 25 };
	leftText.Create ( "Left", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, leftText.sPos, parent, id++ );

	//
	rightText.sPos = { pos.x + width / 3, pos.y, pos.x + 2 * width / 3, pos.y + 25 };
	rightText.Create ( std::string ( "Right (/" + str ( cameraDims.x ) + ")" ).c_str ( ), 
					   WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, rightText.sPos, parent, id++ );

	//
	horizontalBinningText.sPos = { pos.x + 2 * width / 3, pos.y, pos.x + width, pos.y += 25 };
	horizontalBinningText.Create ( "H. Bin", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, horizontalBinningText.sPos,
								   parent, id++ );

	//
	leftEdit.sPos = { pos.x, pos.y, pos.x + width / 3, pos.y + 25 };
	leftEdit.Create ( WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_CENTER, leftEdit.sPos, parent, id++ );
	leftEdit.SetWindowTextA ( "1" );
	//
	rightEdit.sPos = { pos.x + width / 3, pos.y, pos.x + 2 * width / 3, pos.y + 25 };
	rightEdit.Create ( WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_CENTER, rightEdit.sPos, parent, id++ );
	rightEdit.SetWindowTextA ( str ( cameraDims.x ).c_str ( ) );
	//
	horizontalBinningEdit.sPos = { pos.x + 2 * width / 3, pos.y, pos.x + width, pos.y += 25 };
	horizontalBinningEdit.Create ( WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_CENTER, horizontalBinningEdit.sPos, parent,
								   id++ );
	horizontalBinningEdit.SetWindowTextA ( "1" );
	//
	topText.sPos = { pos.x, pos.y, pos.x + width / 3, pos.y + 25 };
	topText.Create ( str ( "Top (/" + str ( cameraDims.y ) + ")" ).c_str ( ), WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY,
					 topText.sPos, parent, id++ );

	//
	bottomText.sPos = { pos.x + width / 3, pos.y, pos.x + 2 * width / 3, pos.y + 25 };
	bottomText.Create ( std::string ( "Bottom" ).c_str ( ), WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, bottomText.sPos,
						parent, id++ );
	//
	verticalBinningText.sPos = { pos.x + 2 * width / 3, pos.y, pos.x + width, pos.y += 25 };
	verticalBinningText.Create ( "V. Bin", WS_CHILD | WS_VISIBLE | ES_CENTER | ES_READONLY, verticalBinningText.sPos,
								 parent, id++ );
	//
	topEdit.sPos = { pos.x, pos.y, pos.x + width / 3, pos.y + 25 };
	topEdit.Create ( WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_CENTER, topEdit.sPos, parent, id++ );
	topEdit.SetWindowTextA ( str ( cameraDims.y ).c_str ( ) );
	//
	bottomEdit.sPos = { pos.x + width / 3, pos.y, pos.x + 2 * width / 3, pos.y + 25 };
	bottomEdit.Create ( WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_CENTER, bottomEdit.sPos, parent, id++ );
	bottomEdit.SetWindowTextA ( "1" );
	//
	verticalBinningEdit.sPos = { pos.x + 2 * width / 3, pos.y, pos.x + width, pos.y += 25 };
	verticalBinningEdit.Create ( WS_TABSTOP | WS_CHILD | WS_VISIBLE | ES_CENTER, verticalBinningEdit.sPos, parent, id++ );
	verticalBinningEdit.SetWindowTextA ( "1" );

	triggerCombo.sPos = { pos.x, pos.y, pos.x + 300, pos.y + 100 };
	triggerCombo.Create ( WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SORT, triggerCombo.sPos, parent,
						  IDC_BASLER_TRIGGER_MODE_COMBO );
	triggerCombo.AddString ( "External-Trigger" );
	triggerCombo.AddString ( "Automatic-Software-Trigger" );
	triggerCombo.AddString ( "Manual-Software-Trigger" );
	triggerCombo.SelectString ( 0, "Automatic-Software-Trigger" );
	pos.y += 25;

	frameRateText.sPos = { pos.x, pos.y, pos.x + 150, pos.y + 25 };
	frameRateText.Create ( "Frame Rate (pics/s): ", WS_CHILD | WS_VISIBLE, frameRateText.sPos, parent, id++ );

	frameRateEdit.sPos = { pos.x + 150, pos.y, pos.x + 225, pos.y + 25 };
	frameRateEdit.Create ( WS_CHILD | WS_VISIBLE, frameRateEdit.sPos, parent, id++ );
	#ifdef _DEBUG
	frameRateEdit.SetWindowTextA ( "0.1" );
	#else
	frameRateEdit.SetWindowTextA ( "30" );
	#endif

	realFrameRate.sPos = { pos.x + 225, pos.y, pos.x + 300, pos.y += 25 };
	realFrameRate.Create ( "", WS_CHILD | WS_VISIBLE, realFrameRate.sPos, parent, id++ );

	gainCombo.sPos = { pos.x, pos.y, pos.x + 300, pos.y + 100 };
	gainCombo.Create ( WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_SORT, gainCombo.sPos, parent, id++ );
	gainCombo.AddString ( "Auto-Gain-Continuous" );
	gainCombo.AddString ( "Auto-Gain-Once" );
	gainCombo.AddString ( "Auto-Gain-Off" );
	gainCombo.SelectString ( 0, "Auto-Gain-Off" );
	pos.y += 25;

	gainText.sPos = { pos.x, pos.y, pos.x + 150, pos.y + 25 };
	gainText.Create ( "Raw Gain (260-?): ", WS_CHILD | WS_VISIBLE, gainText.sPos, parent, id++ );

	gainEdit.sPos = { pos.x + 150, pos.y, pos.x + 300, pos.y += 25 };
	gainEdit.Create ( WS_CHILD | WS_VISIBLE, gainEdit.sPos, parent, id++ );
	gainEdit.SetWindowTextA ( "260" );

	realGainText.sPos = { pos.x, pos.y, pos.x + 150, pos.y + 25 };
	realGainText.Create ( "Real Gain: ", WS_CHILD | WS_VISIBLE, realGainText.sPos, parent, id++ );

	realGainStatus.sPos = { pos.x + 150, pos.y, pos.x + 300, pos.y += 25 };
	realGainStatus.Create ( "", WS_CHILD | WS_VISIBLE, realGainStatus.sPos, parent, id++ );
}

void BaslerSettingsControl::rearrange(int width, int height, fontMap fonts)
{
	statusText.rearrange(width, height, fonts);
	// exposure
	exposureText.rearrange(width, height, fonts);
	exposureModeCombo.rearrange(width, height, fonts);
	exposureEdit.rearrange( width, height, fonts);
	setExposure.rearrange( width, height, fonts);
	// trigger
	triggerCombo.rearrange( width, height, fonts);

	// Dimensions & Binning
	leftText.rearrange( width, height, fonts);
	rightText.rearrange( width, height, fonts);
	horizontalBinningText.rearrange( width, height, fonts);
	topText.rearrange( width, height, fonts);
	bottomText.rearrange( width, height, fonts);
	verticalBinningText.rearrange( width, height, fonts);
	leftEdit.rearrange( width, height, fonts);
	rightEdit.rearrange( width, height, fonts);
	horizontalBinningEdit.rearrange( width, height, fonts);
	topEdit.rearrange( width, height, fonts);
	bottomEdit.rearrange( width, height, fonts);
	verticalBinningEdit.rearrange( width, height, fonts);

	// camera mode: continuous or set #
	cameraMode.rearrange( width, height, fonts);
	repText.rearrange( width, height, fonts);
	repEdit.rearrange( width, height, fonts);

	frameRateText.rearrange( width, height, fonts);
	frameRateEdit.rearrange( width, height, fonts);
	realFrameRate.rearrange( width, height, fonts);

	gainText.rearrange( width, height, fonts);
	gainCombo.rearrange( width, height, fonts);
	gainEdit.rearrange( width, height, fonts);
	realGainText.rearrange( width, height, fonts);
	realGainStatus.rearrange( width, height, fonts);
}

// assumes called on every 10 pics.
void BaslerSettingsControl::handleFrameRate()
{
	ULONG currentTime = GetTickCount();
	ULONG timePerPic = (currentTime - lastTime)/10.0;
	if (timePerPic == 0)
	{
		// avoid dividing by 0.
		timePerPic++;
	}
	realFrameRate.SetWindowTextA(cstr(1000.0 / timePerPic));
	lastTime = currentTime;
}


void BaslerSettingsControl::setStatus(std::string status)
{
	statusText.SetWindowTextA(status.c_str());
}


void BaslerSettingsControl::updateExposure( double exposure )
{
	exposureEdit.SetWindowTextA( cstr( exposure ) );
}


void BaslerSettingsControl::handleCameraMode()
{
	int sel = cameraMode.GetCurSel();
	CString text;
	cameraMode.GetLBText( sel, text );
	if (text == "Finite-Acquisition")
	{
		repEdit.EnableWindow();
	}
	else
	{
		repEdit.EnableWindow( false );
	}
}


baslerSettings BaslerSettingsControl::getCurrentSettings()
{
	loadCurrentSettings ( );
	return currentSettings;
}


/*
Updates the internal object with gui settings
*/
baslerSettings BaslerSettingsControl::loadCurrentSettings ( )
{
	isReady = false;
	int selection = exposureModeCombo.GetCurSel ( );
	CString text;
	if ( selection == -1 )
	{
		thrower ( "ERROR: Please select an exposure mode for the basler camera." );
	}
	exposureModeCombo.GetLBText ( selection, text );
	currentSettings.exposureMode = BaslerAutoExposure::fromStr(std::string(text));
	if ( currentSettings.exposureMode == BaslerAutoExposure::mode::Off )
	{
		try
		{
			exposureEdit.GetWindowTextA ( text );
			currentSettings.exposureTime = std::stod ( std::string ( text ) );
			if ( currentSettings.exposureTime <= 0 )
			{
				throw std::invalid_argument ( "err!" );
			}
		}
		catch ( std::invalid_argument& )
		{
			thrower ( "Error! Please input a valid double for the exposure time." );
		}
	}

	int sel = cameraMode.GetCurSel ( );
	cameraMode.GetLBText ( sel, text );
	currentSettings.acquisitionMode = BaslerAcquisition::fromStr( std::string(text) );
	if ( currentSettings.acquisitionMode == BaslerAcquisition::mode::Finite )
	{
		try
		{
			repEdit.GetWindowTextA ( text );
			currentSettings.repCount = std::stoi ( std::string ( text ) );
			if ( currentSettings.repCount == 0 )
			{
				thrower ( "ERROR! Repetition count must be strictly positive." );
			}
		}
		catch ( std::invalid_argument& )
		{
			thrower ( "Error! Please input a valid positive integer for the rep count." );
		}
	}

	// in case called before initialized
	if ( !leftEdit )
	{
		return currentSettings;
	}
	// If new dimensions are set, we don't have data for the new dimensions.
	// set all of the image parameters
	CString tempStr;
	leftEdit.GetWindowTextA ( tempStr );
	try
	{
		int val = currentSettings.dimensions.left = std::stoi ( std::string ( tempStr ) );
		#ifdef USB_CAMERA
			// round down to nearest multiple of 16
		currentSettings.dimensions.leftBorder = int ( val / 16 ) * 16;
		leftEdit.SetWindowTextA ( cstr ( currentSettings.dimensions.leftBorder ) );
		#endif
	}
	catch ( std::invalid_argument& )
	{

		thrower ( "Left border argument not an integer!\r\n" );
	}
	leftEdit.RedrawWindow ( );

	horizontalBinningEdit.GetWindowTextA ( tempStr );
	try
	{
		currentSettings.dimensions.horizontalBinning = std::stoi ( std::string ( tempStr ) );
	}
	catch ( std::invalid_argument& )
	{
		thrower ( "Horizontal binning argument not an integer!\r\n" );
	}
	horizontalBinningEdit.RedrawWindow ( );

	rightEdit.GetWindowTextA ( tempStr );
	try
	{
		currentSettings.dimensions.right = std::stoi ( std::string ( tempStr ) );
		#ifdef USB_CAMERA
		int roundVal = 1;
		if ( currentSettings.dimensions.horPixelsPerBin == 1 )
		{
			roundVal = 16;
		}
		else if ( currentSettings.dimensions.horPixelsPerBin == 2 )
		{
			roundVal = 32;
		}
		else if ( currentSettings.dimensions.horPixelsPerBin == 4 )
		{
			roundVal = 64;
		}
		// round up to nearest multiple of 16 or 32 or 64 depending on binning.
		int width = currentSettings.dimensions.rightBorder - currentSettings.dimensions.leftBorder + 1;
		width = ( ( width + roundVal - 1 ) / roundVal ) * roundVal;
		currentSettings.dimensions.rightBorder = currentSettings.dimensions.leftBorder + width - 1;
		// compensate in case the extra rounding over-shoots, which is possible if roundVal > 16.
		if ( currentSettings.dimensions.rightBorder > cameraDims.x - 1 )
		{
			currentSettings.dimensions.rightBorder -= roundVal;
		}
		rightEdit.SetWindowTextA ( cstr ( currentSettings.dimensions.rightBorder ) );
		#endif
	}
	catch ( std::invalid_argument& )
	{
		thrower ( "Right border argument not an integer!\r\n" );
	}
	rightEdit.RedrawWindow ( );
	//
	topEdit.GetWindowTextA ( tempStr );
	try
	{
		int val = currentSettings.dimensions.top = std::stoi ( std::string ( tempStr ) );
		#ifdef USB_CAMERA
			// round down to nearest multiple of 16
		currentSettings.dimensions.topBorder = int ( val / 16 ) * 16;
		topEdit.SetWindowTextA ( cstr ( currentSettings.dimensions.topBorder ) );
		#endif
	}
	catch ( std::invalid_argument& )
	{
		thrower ( "Top border argument not an integer!\r\n" );
	}
	topEdit.RedrawWindow ( );
	//
	verticalBinningEdit.GetWindowTextA ( tempStr );
	try
	{
		currentSettings.dimensions.verticalBinning = std::stoi ( std::string ( tempStr ) );
	}
	catch ( std::invalid_argument& )
	{
		thrower ( "Vertical binning argument not an integer!\r\n" );
	}
	verticalBinningEdit.RedrawWindow ( );
	//
	bottomEdit.GetWindowTextA ( tempStr );
	try
	{
		currentSettings.dimensions.bottom = std::stoi ( std::string ( tempStr ) );

		#ifdef USB_CAMERA
			// round up to nearest multiple of 16 or 32 or 64 depending on binning.
		int roundVal = 1;
		if ( currentSettings.dimensions.vertPixelsPerBin == 1 )
		{
			roundVal = 16;
		}
		else if ( currentSettings.dimensions.vertPixelsPerBin == 2 )
		{
			roundVal = 32;
		}
		else if ( currentSettings.dimensions.vertPixelsPerBin == 4 )
		{
			roundVal = 64;
		}
		int height = currentSettings.dimensions.bottomBorder - currentSettings.dimensions.topBorder + 1;
		height = ( ( height + roundVal - 1 ) / roundVal ) * roundVal;
		currentSettings.dimensions.bottomBorder = currentSettings.dimensions.topBorder + height - 1;
		// compensate in case the extra rounding over-shoots, which is possible if roundVal > 16.
		if ( currentSettings.dimensions.bottomBorder > cameraDims.y - 1 )
		{
			currentSettings.dimensions.bottomBorder -= roundVal;
		}
		bottomEdit.SetWindowTextA ( cstr ( currentSettings.dimensions.bottomBorder ) );
		#endif
	}
	catch ( std::invalid_argument& )
	{
		thrower ( "Bottom border argument not an integer!\r\n" );
	}
	bottomEdit.RedrawWindow ( );



	#ifdef USB_CAMERA
		currentSettings.dimensions.checkConsistency ( "ace" );
	#endif
	#ifdef FIREWIRE_CAMERA
		currentSettings.dimensions.checkConsistency ( "scout" );
	#endif
	if (currentSettings.dimensions.horizontalBinning > 4 || currentSettings.dimensions.verticalBinning > 4)
	{
		thrower( "ERROR: Binning on a camera cannot exceed 4 pixels per bin!\r\n" );
	}

	selection = triggerCombo.GetCurSel();
	triggerCombo.GetLBText( selection, text );
	
	currentSettings.triggerMode = BaslerTrigger::fromStr( std::string(text) );

	frameRateEdit.GetWindowTextA( text );
	try
	{
		currentSettings.frameRate = std::stod( std::string( text ) );
	}
	catch (std::invalid_argument& err)
	{
		thrower( std::string("ERROR! Please enter a valid float for the frame rate. ") + err.what() );
	}
	isReady = true;
	return currentSettings;
}


void BaslerSettingsControl::handleOpeningConfig ( std::ifstream& configFile, Version ver )
{
	ProfileSystem::checkDelimiterLine ( configFile, "Basler-Settings" );
	baslerSettings newSettings;
	std::string txt;
	
	configFile >> txt;
	newSettings.acquisitionMode = BaslerAcquisition::fromStr ( txt );
	std::string test;
	configFile >> test;
	newSettings.dimensions.left = std::stoi(test);
	configFile >> test;
	newSettings.dimensions.top = std::stoi(test);
	configFile >> test;
	newSettings.dimensions.right = std::stoi(test);
	configFile >> test;
	newSettings.dimensions.bottom = std::stoi(test);

	configFile >> newSettings.dimensions.horizontalBinning;
	configFile >> newSettings.dimensions.verticalBinning;
	configFile >> txt;
	newSettings.exposureMode = BaslerAutoExposure::fromStr(txt);
	configFile >> newSettings.exposureTime;
	configFile >> newSettings.frameRate;
	configFile >> newSettings.rawGain;
	configFile >> newSettings.repCount;
	configFile >> txt;
	newSettings.triggerMode = BaslerTrigger::fromStr ( txt );
	setSettings ( newSettings );
}

void BaslerSettingsControl::handleSavingConfig ( std::ofstream& configFile )
{ 
	loadCurrentSettings( );
	configFile << "Basler-Settings\n";
	configFile << BaslerAcquisition::toStr(currentSettings.acquisitionMode) << "\n";
	configFile << currentSettings.dimensions.left << "\n";
	configFile << currentSettings.dimensions.top << "\n";
	configFile << currentSettings.dimensions.right << "\n";
	configFile << currentSettings.dimensions.bottom << "\n";
	configFile << currentSettings.dimensions.horizontalBinning << "\n";
	configFile << currentSettings.dimensions.verticalBinning << "\n";
	configFile << BaslerAutoExposure::toStr(currentSettings.exposureMode) << "\n";
	configFile << currentSettings.exposureTime << "\n";
	configFile << currentSettings.frameRate << "\n";
	configFile << currentSettings.rawGain << "\n";
	configFile << currentSettings.repCount << "\n";
	configFile << BaslerTrigger::toStr(currentSettings.triggerMode) << "\n";
}



void BaslerSettingsControl::setSettings ( baslerSettings newSettings )
{
	currentSettings = newSettings;
	cameraMode.SelectString ( 0, BaslerAcquisition::toStr ( currentSettings.acquisitionMode ).c_str ( ) );
	leftEdit.SetWindowTextA ( cstr ( currentSettings.dimensions.left ) );
	topEdit.SetWindowTextA ( cstr ( currentSettings.dimensions.top ) );
	rightEdit.SetWindowTextA ( cstr ( currentSettings.dimensions.right ) );
	bottomEdit.SetWindowTextA ( cstr ( currentSettings.dimensions.bottom ) );
	horizontalBinningEdit.SetWindowTextA ( cstr ( currentSettings.dimensions.horizontalBinning ) );
	verticalBinningEdit.SetWindowTextA ( cstr ( currentSettings.dimensions.verticalBinning ) );
	exposureModeCombo.SelectString ( 0, BaslerAutoExposure::toStr ( currentSettings.exposureMode ).c_str ( ) );
	exposureEdit.SetWindowTextA ( cstr ( currentSettings.exposureTime ) );
	frameRateEdit.SetWindowTextA ( cstr ( currentSettings.frameRate ) );
	gainEdit.SetWindowTextA ( cstr ( currentSettings.rawGain ) );
	repEdit.SetWindowText ( cstr ( currentSettings.repCount ) );
	triggerCombo.SelectString ( 0, BaslerTrigger::toStr ( currentSettings.triggerMode ).c_str ( ) );
}


void BaslerSettingsControl::handleGain()
{
	// todo
}


void BaslerSettingsControl::handleExposureMode()
{
	int sel = exposureModeCombo.GetCurSel();
	CString text;
	exposureModeCombo.GetLBText(sel, text);
	if (text == "Auto Exposure Off")
	{
		exposureEdit.EnableWindow();
	}
	else
	{
		exposureEdit.EnableWindow(false);
	}
}


