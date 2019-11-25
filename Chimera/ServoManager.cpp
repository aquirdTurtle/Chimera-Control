// created by Mark O. Brown
#include "stdafx.h"
#include "ServoManager.h"
#include "TextPromptDialog.h"
#include "boost/lexical_cast.hpp"

void ServoManager::initialize( POINT& pos, cToolTips& toolTips, CWnd* parent, int& id,
							   AiSystem* ai_in, AoSystem* ao_in, DioSystem* ttls_in, ParameterSystem* globals_in )
{
	servosHeader.sPos = {pos.x, pos.y, pos.x + 480, pos.y + 20};
	servosHeader.Create( "SERVOS & MONITORS", NORM_HEADER_OPTIONS, servosHeader.sPos, parent, id++ );
 	servoButton.sPos = { pos.x + 480, pos.y, pos.x + 780, pos.y + 20 };
	servoButton.Create( "Servo-Once", NORM_PUSH_OPTIONS, servoButton.sPos, parent, IDC_SERVO_CAL );
	servoButton.setToolTip ( "Force the servo to calibrate.", toolTips, parent );

	autoServoButton.sPos = { pos.x + 780, pos.y, pos.x + 960, pos.y += 20 };
	autoServoButton.Create( "Auto-Servo", NORM_CHECK_OPTIONS, autoServoButton.sPos, parent, id++ );
	autoServoButton.setToolTip ( "Automatically calibrate all servos after F1.", toolTips, parent );

	servoList.sPos = { pos.x, pos.y, pos.x + 960, pos.y += 340 };
	servoList.Create ( NORM_LISTVIEW_OPTIONS, servoList.sPos, parent, IDC_SERVO_LISTVIEW );
	servoList.InsertColumn ( 0,  "Name", 150 );
	servoList.InsertColumn ( 1,  "Active?", 50 );
	servoList.InsertColumn ( 2,  "Set (V)" );
	servoList.InsertColumn ( 3,  "Ctrl (V)");
	servoList.InsertColumn ( 4,  "Res (V)");
	servoList.InsertColumn ( 5,  "Ai", 35 );
	servoList.InsertColumn ( 6,  "Ao" );
	servoList.InsertColumn ( 7,  "DO-Config", 90 );
	servoList.InsertColumn ( 8,  "Tolerance", 80 );
	servoList.InsertColumn ( 9,  "Gain", 45 );
	servoList.InsertColumn ( 10,  "Monitor?", 70 );
	servoList.InsertColumn ( 11, "AO-Config", 150);
	servoList.insertBlankRow ( );
	servoList.setToolTip ( "Name: The name of the servo, gets incorperated into the name of the servo_variable.\n"
						   "Active: Whether the servo will calibrate when you auto-servoing or after servo-once\n"
						   "Set-Point: The servo\'s set point, in volts.\n"
						   "Control-Value: The current control value (in volts) which was found to make the input match the set-point\n"
						   "Ai: The analog input that the servo will watch & match to the set-point\n"
						   "Ao: The analog output the servo will use to try to change the value of the analog input\n"
						   "DO-Config: The digital output configuration the sevo will set before servoing. If a ttl is "
								"not listed here, it will be zero\'d.\n"
						   "Tolerance: The tolerance of the servo. The servo will stop servoing when value falls within tol of the set point\n"
						   "Gain: The gain of the servo loop.\n"
						   "Monitor: If monitoring, the servo only checks that the value is within the tolerance of the set point and raises a warning if not.\n"
						   "AO-Config: The digital output configuration the sevo will set before servoing. If a ao is "
								"not listed here, it will be zero\'d.\n", toolTips, parent );
	servoList.fontType = fontTypes::SmallCodeFont;
	servoList.SetTextBkColor ( _myRGBs["Interactable-Bkgd"] );
	servoList.SetTextColor ( _myRGBs[ "AuxWin-Text" ] );
	servoList.SetBkColor ( _myRGBs[ "Interactable-Bkgd" ] );

	ai = ai_in;
	ao = ao_in;
	ttls = ttls_in;
	globals = globals_in;
}


std::vector<servoInfo> ServoManager::getServoInfo ( )
{
	return servos;
}


void ServoManager::handleSaveMasterConfig( std::stringstream& configStream )
{
	configStream << autoServoButton.GetCheck ( ) << "\n" << servos.size ( );
	for ( auto& servo : servos )
	{
		handleSaveMasterConfigIndvServo ( configStream, servo );
	}
}

void ServoManager::handleOpenMasterConfig( std::stringstream& configStream, Version version )
{
	if ( version < Version( "2.1" ) )
	{
		// this was before the servo manager.
		return;
	}
	if ( version < Version ( "2.5" ) )
	{
		double tolerance;
		configStream >> tolerance;
	}
	bool autoServo;
	configStream >> autoServo;
	autoServoButton.SetCheck( autoServo );
	LONG numServosInFile;
	configStream >> numServosInFile;
	servos.clear ( );
	for ( auto servoNum : range ( numServosInFile ) )
	{
		servos.push_back ( handleOpenMasterConfigIndvServo ( configStream, version ) );
	}
	refreshAllServos ( );
}


void ServoManager::deleteServo ( )
{
	/// get the item and subitem
	POINT cursorPos;
	GetCursorPos ( &cursorPos );
	servoList.ScreenToClient ( &cursorPos );
	int subitemIndicator = servoList.HitTest ( cursorPos );
	LVHITTESTINFO myItemInfo;
	memset ( &myItemInfo, 0, sizeof ( LVHITTESTINFO ) );
	myItemInfo.pt = cursorPos;
	int itemIndicator = servoList.SubItemHitTest ( &myItemInfo );
	if ( itemIndicator == -1 || itemIndicator == servos.size ( ) )
	{
		// user didn't click in a deletable item.
		return;
	}
	int answer;
	if ( UINT ( itemIndicator ) < servos.size ( ) )
	{
		answer = promptBox ( "Delete Servo " + servos[ itemIndicator ].servoName + "?", MB_YESNO );
		if ( answer == IDYES )
		{
			servoList.DeleteItem ( itemIndicator );
			servos.erase ( servos.begin ( ) + itemIndicator );
		}

	}
	else if ( UINT ( itemIndicator ) > servos.size ( ) )
	{
		answer = promptBox ( "You appear to have found a bug with the listview control... there are too many lines "
							 "in this control. Clear this line?", MB_YESNO );
		if ( answer == IDYES )
		{
			servoList.DeleteItem ( itemIndicator );
		}
	}
}



void ServoManager::handleListViewClick ( )
{
	LVHITTESTINFO myItemInfo = { 0 };
	GetCursorPos ( &myItemInfo.pt );
	servoList.ScreenToClient ( &myItemInfo.pt );
	servoList.SubItemHitTest ( &myItemInfo );
	int subitem, itemIndicator;
	itemIndicator = myItemInfo.iItem;
	if ( itemIndicator == -1 )
	{
		return;
	}
	CString text = servoList.GetItemText ( itemIndicator, 0 );
	if ( text == "___" )
	{	// add a variable		
		servos.resize ( servos.size ( ) + 1 );
		servos.back ( ).servoName = "noname";
		// make a new "new" row.
		servoList.InsertItem ( "noname", itemIndicator, 0 );
		servoList.SetItem ( "-", itemIndicator, 1 );
	}
	auto& servo = servos[ itemIndicator ];
	subitem = myItemInfo.iSubItem;
	/// Handle different subitem clicks
	switch ( subitem )
	{
		case 0:
		{	/// person name
			std::string newName;
			TextPromptDialog dialog ( &newName, "Please enter a name for the servo." );
			dialog.DoModal ( );
			// make name lower case
			std::transform ( newName.begin ( ), newName.end ( ), newName.begin ( ), ::tolower );
			if ( newName == "" )
			{
				// probably canceled.
				break;
			}
			for ( auto s : servos )
			{
				if ( s.servoName == newName )
				{
					thrower ( "An servo with name " + newName + " already exists!" );
				}
			}
			servo.servoName = newName;
			break;
		}
		case 1:
		{	// active, toggles.			
			servo.active = !servo.active;
			break;
		}
		case 2:
		{
			std::string setPointTxt;
			TextPromptDialog dialog ( &setPointTxt, "Please enter a set point for the servo." );
			dialog.DoModal ( );
			try
			{
				servo.setPoint = boost::lexical_cast<double>( setPointTxt );
			}
			catch ( boost::bad_lexical_cast& )
			{
				throwNested ( "Failed to convert text to a double!" );
			}
			// set point
			break;
		}
		case 3: // control value, unresponsive
		case 4: // result value, unresponsive
			break;
		case 5:
		{	// ai
			std::string aiTxt;
			TextPromptDialog dialog ( &aiTxt, "Please enter the analog-input for the servo to look at." );
			dialog.DoModal ( );
			try
			{
				servo.aiInputChannel = boost::lexical_cast<UINT>( aiTxt );
			}
			catch ( boost::bad_lexical_cast& )
			{
				throwNested ( "Failed to convert text to an unsigned int!" );
			}
			break;
		}
		case 6:
		{	// ao			
			std::string aoTxt;
			TextPromptDialog dialog ( &aoTxt, "Please enter a the analog-output for the servo to use for control." );
			dialog.DoModal ( );
			try
			{
				servo.aoControlChannel = boost::lexical_cast<UINT>( aoTxt );
			}
			catch ( boost::bad_lexical_cast& )
			{
				throwNested ( "Failed to convert text to an unsigned int!" );
			}
			break;
		}
		case 7:
		{	// Digital-output config
			std::string doTxt;
			TextPromptDialog dialog ( &doTxt, "Please enter the digital outputs that must be on for the calibration. "
									  "Everthing else will be off. Please separate the row from the number with a "
									  "space and each ttl with a space. E.g. \"A 1 D 0\"" );
			dialog.DoModal ( );
			std::stringstream tmpStream(doTxt);
			std::string rowTxt;
			servo.ttlConfig.clear ( );
			while ( tmpStream >> rowTxt )
			{
				try
				{
					std::pair<DioRows::which, UINT> ttl;
					ttl.first = DioRows::fromStr ( rowTxt );
					tmpStream >> ttl.second;
					servo.ttlConfig.push_back ( ttl );
				}
				catch ( Error& )
				{
					throwNested ( "Error In trying to set the servo ttl config!" );
				}
			}
			break;
 		}
		case 8:
		{	// tolerance
			std::string tolTxt;
			TextPromptDialog dialog ( &tolTxt, "Please enter a tolerance (V) for the servo." );
			dialog.DoModal ( );
			try
			{
				servo.tolerance = boost::lexical_cast<double>( tolTxt );
			}
			catch ( boost::bad_lexical_cast& )
			{
				throwNested ( "Failed to convert text to a double!" );
			}
			break;
		}
		case 9:
		{	// gain 
			std::string gainTxt;
			TextPromptDialog dialog ( &gainTxt, "Please enter a gain factor for the servo." );
			dialog.DoModal ( );
			try
			{
				servo.gain = boost::lexical_cast<double>( gainTxt );
			}
			catch ( boost::bad_lexical_cast& )
			{
				throwNested ( "Failed to convert text to a double!" );
			}
			break;
		}
		case 10:
		{	// monitor only toggle
			servo.monitorOnly = !servo.monitorOnly;
			break;
		}
		case 11:
		{	// Analog-output config

			std::string aoTxt;
			TextPromptDialog dialog (&aoTxt, "Please enter the Analog outputs that must be on for the calibration. "
				"Everthing else will be off. Please separate the dac identifier from the value with a space. "
				"E.g. \"dac20 0.4 dac4 -10\"");
			dialog.DoModal ();
			std::stringstream tmpStream (aoTxt);
			std::string dacIdTxt;
			servo.aoConfig.clear ();
			while (tmpStream >> dacIdTxt)
			{
				try
				{
					auto id = AoSystem::getBasicDacIdentifier (dacIdTxt);
					if (id == -1)
					{
						thrower ("Dac Identifier \"" + dacIdTxt + "\" failed to convert to a basic dac id!");
					}
					std::pair<UINT, double> dacSetting;
					dacSetting.first = id;
					tmpStream >> dacSetting.second;
					servo.aoConfig.push_back (dacSetting);
				}
				catch (Error&)
				{
					throwNested ("Error In trying to set the servo dac config!");
				}
			}
			break;
		}
 	}
 	refreshAllServos ( );
}


servoInfo ServoManager::handleOpenMasterConfigIndvServo ( std::stringstream& configStream, Version version )
{
	servoInfo tmpInfo;
	configStream >> tmpInfo.servoName;
	if ( version > Version ( "2.3" ) )
	{
		configStream >> tmpInfo.aiInputChannel >> tmpInfo.aoControlChannel;
	}
	else
	{
		tmpInfo.aiInputChannel = tmpInfo.aoControlChannel = 0;
	}
	configStream >> tmpInfo.active >> tmpInfo.setPoint;
	if ( version > Version ( "2.3" ) )
	{
		UINT numSettings;
		configStream >> numSettings;
		tmpInfo.ttlConfig.resize ( numSettings );
		for ( auto& ttl : tmpInfo.ttlConfig )
		{
			std::string rowStr;
			configStream >> rowStr >> ttl.second;
			ttl.first = DioRows::fromStr ( rowStr );
		}
	}
	if (version > Version ("2.6")) 
	{
		UINT numSettings;
		configStream >> numSettings;
		tmpInfo.aoConfig.resize (numSettings);
		for (auto& ao : tmpInfo.aoConfig)
		{
			UINT dacID;
			configStream >> dacID >> ao.second;
			ao.first = dacID;
		}
	}
	if ( version > Version ( "2.4" ) )
	{
		configStream >> tmpInfo.tolerance >> tmpInfo.gain;
	}
	if ( version > Version ( "2.5" ) )
	{
		configStream >> tmpInfo.monitorOnly;
	}
	return tmpInfo;
}


void ServoManager::refreshAllServos ( )
{
	servoList.DeleteAllItems ( );
	UINT count = 0;
	for ( auto& servo : servos )
	{
		updateServoInfo ( servo, count++ );
	}
	servoList.insertBlankRow ( );
}


void ServoManager::updateServoInfo ( servoInfo& s, UINT which )
{
	servoList.InsertItem ( s.servoName, which, 0 );
	servoList.SetItem ( s.active ? "Yes" : "No", which, 1 );
	servoList.SetItem ( str ( s.setPoint ), which, 2 );
	servoList.SetItem ( s.monitorOnly ? "--" : str ( s.controlValue ), which, 3 );
	servoList.SetItem ( str (s.mostRecentResult), which, 4);
	servoList.SetItem ( str ( s.aiInputChannel ), which, 5 );
	servoList.SetItem ( s.monitorOnly ? "--" : str ( s.aoControlChannel ), which, 6 );
	std::string digitalOutConfigString;
	for ( auto val : s.ttlConfig )
	{
		digitalOutConfigString += DioRows::toStr( val.first ) + " " + str ( val.second ) + " ";
	}
	servoList.SetItem ( str ( digitalOutConfigString ), which, 7 );
	servoList.SetItem ( str ( s.tolerance ), which, 8 );
	servoList.SetItem ( s.monitorOnly ? "--" : str ( s.gain ), which, 9 );
	servoList.SetItem ( s.monitorOnly ? "Yes" : "No", which, 10 );
	std::string aoString;
	for (auto ao : s.aoConfig)
	{
		aoString += "dac" + str (ao.first) + " " + str (ao.second, 4) + " ";
	}
	servoList.SetItem (aoString, which, 11);
	
}


void ServoManager::handleSaveMasterConfigIndvServo ( std::stringstream& configStream, servoInfo& servo )
{
	configStream << servo.servoName << " " << servo.aiInputChannel << " " << servo.aoControlChannel << " "
		<< servo.active << " " << servo.setPoint << " " << servo.ttlConfig.size ( ) << " ";
	for ( auto& ttl : servo.ttlConfig )
	{
		configStream << DioRows::toStr(ttl.first) << " " << str(ttl.second) << " ";
	}
	configStream << servo.aoConfig.size () << " ";
	for (auto& dac : servo.aoConfig)
	{
		configStream << dac.first << " " << dac.second << " ";
	}
	configStream << servo.tolerance << " " << servo.gain << " " << servo.monitorOnly << "\n";
}


void ServoManager::rearrange( UINT width, UINT height, fontMap fonts )
{
	servosHeader.rearrange( width, height, fonts );
	servoButton.rearrange( width, height, fonts );
	autoServoButton.rearrange( width, height, fonts );
	servoList.rearrange ( width, height, fonts );
}


bool ServoManager::autoServo( )
{
	return autoServoButton.GetCheck( );
}


void ServoManager::runAll( )
{
	UINT count = 0;
	// made this asynchronous to facilitate updating gui while 
	for ( auto& servo : servos )
	{
		try
		{
			ServoManager::calibrate (servo, count++);
		}
		catch (Error & e) 
		{
			errBox (e.trace());
			// but continue to try the other ones. 
		}
	}
	refreshAllServos ();
	ttls->zeroBoard ( );
	ao->zeroDacs(ttls);
}


void ServoManager::calibrate( servoInfo& s, UINT which )
{
	if ( !s.active )
	{
		return;
	}
	double sp = s.setPoint;
	// helps auto calibrate the servo for lower servo powers
	ttls->zeroBoard ( );
	ao->zeroDacs (ttls);
	//ao->forceDacs (ttls);
	for (auto dac : s.aoConfig)
	{
		ao->setSingleDac (dac.first, dac.second, ttls);
	}
	for ( auto ttl : s.ttlConfig )
	{
		ttls->ftdi_ForceOutput (ttl.first, ttl.second, 1);
	}
	Sleep (20);
	// give some time for the lasers to settle..
	UINT attemptLimit = 100;
	UINT count = 0;
	UINT aiNum = s.aiInputChannel;
	UINT aoNum = s.aoControlChannel;
	if ( s.monitorOnly )
	{	// handle "servos" which are only monitoring values, not trying to change them. 
		double avgVal = ai->getSingleChannelValue ( aiNum, 100 );
		double avgVal2 = ai->getSingleChannelValue (aiNum, 100);

		s.mostRecentResult = avgVal2;
		double percentDif = ( sp - avgVal2) / sp;
		if ( fabs ( percentDif )  < s.tolerance )
		{
			// Value looks good, nothing to report.
		}
		else
		{
			errBox ( s.servoName + " Monitor: Value has drifted out of tolerance!" );
		}
		// And the rest of the function is handling the servo part. 
		return;
	}
	s.controlValue = globals->getVariableValue (str (s.servoName + "__servo_value", 13, false, true));
	// start the dac where it was last.
	ao->setSingleDac (aoNum, s.controlValue, ttls);
	while ( count++ < attemptLimit )
	{
		double avgVal = ai->getSingleChannelValue(aiNum, 10);
		s.mostRecentResult = avgVal;
		double percentDif = (sp - avgVal) / sp;
		if ( fabs(percentDif)  < s.tolerance )
		{
			// found a good value.
			break;
		}
		else
		{
			// modify dac value.
			s.controlValue = ao->getDacValue( aoNum );
			double diff = s.gain * percentDif > 0.05 ? 0.05 : s.gain * percentDif;
			s.controlValue += diff;
			try
			{
				ao->setSingleDac( aoNum, s.controlValue, ttls );
			}
			catch ( Error& )
			{
				// happens if servo value gives result out of range of dacs.
				auto r = ao->getDacRange ( aoNum );
				try
				{
					if ( s.controlValue < r.first )
					{
						ao->setSingleDac ( aoNum, r.first, ttls );
					}
					else if ( s.controlValue > r.second )
					{
						ao->setSingleDac ( aoNum, r.second, ttls );
					}
				}
				catch ( Error& )
				{
					// something went wrong...
					count = attemptLimit;
					break;
				}
			}
			// There's a little break built in here in order to let the laser power settle a little. 
			// Not sure how necessary this is.
			Sleep( 20 );
			setControlDisplay ( which, ao->getDacValue( aoNum ) );
			setResDisplay (which, s.mostRecentResult);
		}
	}
	auto dacVal = ao->getDacValue ( aoNum );
	setControlDisplay ( which, dacVal );
	setResDisplay (which, s.mostRecentResult);
	s.servoed = (count < attemptLimit);
	if ( !s.servoed )
	{
		errBox( "" + s.servoName + " servo failed to servo!" );
		// and don't adjust the variable value with what is probably a bad value. 
	}
	else
	{
		globals->adjustVariableValue( str(s.servoName + "__servo_value",13, false, true), dacVal );
	}
}




void ServoManager::setControlDisplay (UINT which, double value )
{
	servoList.SetItem ( str ( value ), which, 3 );
	servoList.RedrawWindow ( );
}

void ServoManager::setResDisplay (UINT which, double value)
{
	servoList.SetItem (str (value), which, 4);
	servoList.RedrawWindow ();
}


