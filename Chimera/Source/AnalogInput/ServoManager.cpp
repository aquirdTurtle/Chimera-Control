// created by Mark O. Brown
#include "stdafx.h"
#include "ServoManager.h"
#include "ExcessDialogs/TextPromptDialog.h"
#include "boost/lexical_cast.hpp"
#include "ExperimentThread/Communicator.h"

void ServoManager::handleDraw (NMHDR* pNMHDR, LRESULT* pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);
	*pResult = CDRF_DODEFAULT;
	// First thing - check the draw stage. If it's the control's prepaint
	// stage, then tell Windows we want messages for every item.
	switch (pLVCD->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
			*pResult = CDRF_NOTIFYITEMDRAW;
			break;
		case CDDS_ITEMPREPAINT:
		{
			int item = pLVCD->nmcd.dwItemSpec;
			if (item < 0)
			{
				return;
			}
			if (item >= servos.size ())
			{
				pLVCD->clrText = _myRGBs["AuxWin-Text"];
				pLVCD->clrTextBk = _myRGBs["Interactable-Bkgd"];
			}
			else
			{
				if (servos[item].currentlyServoing)
				{
					pLVCD->clrTextBk = _myRGBs["Solarized Orange"];
				}
				else
				{
					pLVCD->clrTextBk = _myRGBs[servos[item].servoed ? "Interactable-Bkgd" : "Red"];
				}
				pLVCD->clrText = _myRGBs["Text"];
			}
			// Tell Windows to paint the control itself.
			*pResult = CDRF_NOTIFYSUBITEMDRAW;
		}
		case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
		{	// handle subitem stuff...
			switch (pLVCD->iSubItem)
			{
				case 3:
				case 5: 
					pLVCD->clrText = _myRGBs["Text"];
					break;
				case 4:
				{
					auto servoInfos = getServoInfo ();
					if (pLVCD->nmcd.dwItemSpec < 0 || pLVCD->nmcd.dwItemSpec >= servoInfos.size ())
					{
						// at least happens on the new item row
						pLVCD->clrText = _myRGBs["AuxWin-Text"];
						break;
					}
					auto chng = fabs(servoInfos [pLVCD->nmcd.dwItemSpec].changeInCtrl);
					// these ranges are arbitrary and you should feel free to change them as seems reasonable.
					if (chng < 0.01)
					{	// "Stable" reading.
						pLVCD->clrText = _myRGBs["Green"];
					}
					else if (chng < 0.05)
					{	// "Warning" - Value changed significantly
						pLVCD->clrText = _myRGBs["Orange"];
					}
					else
					{	// "Stronger Warning" - Value changed a lot
						pLVCD->clrText = _myRGBs["Red"];
					}
					break;
				}
				default:
					pLVCD->clrText = _myRGBs["AuxWin-Text"];
					break;
			}
		}
	}
}

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
	servoList.InsertColumn ( 4,  "dCtrl (%)", 60);
	servoList.InsertColumn ( 5,  "Res (V)", 50);
	servoList.InsertColumn ( 6,  "dRes (%)", 60);
	servoList.InsertColumn ( 7,  "Ai", 35 );
	servoList.InsertColumn ( 8,  "Ao" );
	servoList.InsertColumn ( 9,  "DO-Config", 90 );
	servoList.InsertColumn ( 10,  "Tolerance", 70 );
	servoList.InsertColumn ( 11,  "Gain", 45 );
	servoList.InsertColumn ( 12,  "Monitor?", 70 );
	servoList.InsertColumn ( 13, "AO-Config", 150);
	servoList.InsertColumn ( 14, "Avgs", 50);

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
	refreshListview ( );
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
			TextPromptDialog dialog ( &newName, "Please enter a name for the servo.", servo.servoName);
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
			TextPromptDialog dialog ( &setPointTxt, "Please enter a set point for the servo.", str(servo.setPoint) );
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
		case 4: // control change, unresponsive
		case 5: // result value, unresponsive
		case 6: // result change, unresponsive
			break;
		case 7:
		{	// ai
			std::string aiTxt;
			TextPromptDialog dialog ( &aiTxt, "Please enter the analog-input for the servo to look at.", str(servo.aiInputChannel) );
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
		case 8:
		{	// ao			
			std::string aoTxt;
			TextPromptDialog dialog ( &aoTxt, "Please enter a the analog-output for the servo to use for control.", str(servo.aoControlChannel) );
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
		case 9:
		{	// Digital-output config
			std::string doTxt;
			TextPromptDialog dialog ( &doTxt, "Please enter the digital outputs that must be on for the calibration. "
									  "Everthing else will be off. Please separate the row from the number with a "
									  "space and each ttl with a space. E.g. \"A 1 D 0\"", 
									  servoTtlConfigToString (servo.ttlConfig));
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
		case 10:
		{	// tolerance
			std::string tolTxt;
			TextPromptDialog dialog ( &tolTxt, "Please enter a tolerance (V) for the servo.", str(servo.tolerance) );
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
		case 11:
		{	// gain 
			std::string gainTxt;
			TextPromptDialog dialog ( &gainTxt, "Please enter a gain factor for the servo.", str(servo.gain) );
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
		case 12:
		{	// monitor only toggle
			servo.monitorOnly = !servo.monitorOnly;
			break;
		}
		case 13:
		{	// Analog-output config
			std::string aoTxt;
			TextPromptDialog dialog (&aoTxt, "Please enter the Analog outputs that must be on for the calibration. "
				"Everthing else will be off. Please separate the dac identifier from the value with a space. "
				"E.g. \"dac20 0.4 dac4 -10\"", servoDacConfigToString (servo.aoConfig));
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
		case 14:
		{
			std::string numAvgsTxt;
			TextPromptDialog dialog (&numAvgsTxt, "Please enter a number (>1) of ai averages for the servo.", str(servo.avgNum));
			dialog.DoModal ();
			try
			{
				servo.avgNum = boost::lexical_cast<UINT>(numAvgsTxt);
			}
			catch (boost::bad_lexical_cast&)
			{
				throwNested ("Failed to convert text to a double!");
			}
			break;
		}
	}
 	refreshListview ( );
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
	if (version > Version ("2.7"))
	{
		configStream >> tmpInfo.avgNum;
	}
	return tmpInfo;
}


void ServoManager::refreshListview ( )
{
	servoList.DeleteAllItems ( );
	UINT count = 0;
	for ( auto& servo : servos )
	{
		addServoToListview ( servo, count++ );
	}
	servoList.insertBlankRow ( );
}


void ServoManager::addServoToListview ( servoInfo& s, UINT which )
{
	servoList.InsertItem ( s.servoName, which, 0 );
	servoList.SetItem ( s.active ? "Yes" : "No", which, 1 );
	servoList.SetItem ( str ( s.setPoint ), which, 2 );
	servoList.SetItem ( s.monitorOnly ? "--" : str ( s.controlValue ), which, 3 );
	servoList.SetItem ( (s.changeInCtrl < 0 ? "" : "+") + str (s.changeInCtrl*100), which, 4);
	servoList.SetItem ( str ( s.mostRecentResult ), which, 5);
	servoList.SetItem ( str ( s.aiInputChannel ), which, 6 );
	servoList.SetItem ( s.monitorOnly ? "--" : str ( s.aoControlChannel ), which, 7 );
	servoList.SetItem (servoTtlConfigToString (s.ttlConfig), which, 8 );
	servoList.SetItem ( str ( s.tolerance ), which, 9 );
	servoList.SetItem ( s.monitorOnly ? "--" : str ( s.gain ), which, 10 );
	servoList.SetItem ( s.monitorOnly ? "Yes" : "No", which, 11 );
	servoList.SetItem (servoDacConfigToString(s.aoConfig), which, 12);
	servoList.SetItem (str (s.avgNum), which, 13);
}

std::string ServoManager::servoDacConfigToString (std::vector<std::pair<UINT, double>> aoConfig)
{
	std::string aoString;
	for (auto ao : aoConfig)
	{
		aoString += "dac" + str (ao.first) + " " + str (ao.second, 4) + " ";
	}
	return aoString;
}


std::string ServoManager::servoTtlConfigToString (std::vector<std::pair<DioRows::which, UINT> > ttlConfig)
{
	std::string digitalOutConfigString;
	for (auto val : ttlConfig)
	{
		digitalOutConfigString += DioRows::toStr (val.first) + " " + str (val.second) + " ";
	}
	return digitalOutConfigString;
}


void ServoManager::handleSaveMasterConfigIndvServo ( std::stringstream& configStream, servoInfo& servo )
{
	configStream << servo.servoName << " " << servo.aiInputChannel << " " << servo.aoControlChannel << " "
		<< servo.active << " " << servo.setPoint << " " << servo.ttlConfig.size ( ) << " " 
		<< servoTtlConfigToString (servo.ttlConfig);
	configStream << servo.aoConfig.size () << " ";
	for (auto& dac : servo.aoConfig)
	{
		configStream << dac.first << " " << dac.second << " ";
	}
	configStream << servo.tolerance << " " << servo.gain << " " << servo.monitorOnly << " " << servo.avgNum << "\n";
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


void ServoManager::runAll( Communicator& comm)
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
			comm.sendError (e.trace ());
			// but continue to try the other ones. 
		}
	}
	refreshListview ();
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
	s.currentlyServoing = true;
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
		double avgVal = ai->getSingleChannelValue ( aiNum, s.avgNum );
		s.mostRecentResult = avgVal;
		double percentDif = ( sp - avgVal) / sp;
		if ( fabs ( percentDif )  < s.tolerance )
		{
			// Value looks good, nothing to report.
		}
		else
		{
			errBox ( s.servoName + " Monitor: Value has drifted out of tolerance!" );
		}
		// And the rest of the function is handling the servo part. 
		s.currentlyServoing = false;
		return;
	}
	s.controlValue = globals->getVariableValue (str (s.servoName + "__servo_value", 13, false, true));
	// start the dac where it was last.
	auto oldVal = s.controlValue;
	ao->setSingleDac (aoNum, s.controlValue, ttls);
	while ( count++ < attemptLimit )
	{
		double avgVal = ai->getSingleChannelValue(aiNum, s.avgNum);
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
			s.changeInCtrl = (s.controlValue - oldVal) / oldVal;
			setControlDisplay ( which, ao->getDacValue( aoNum ) );
			setResDisplay (which, s.mostRecentResult);
			setChangeVal (which, s.changeInCtrl);
		}
	}
	auto dacVal = ao->getDacValue ( aoNum );
	s.changeInCtrl = (s.controlValue - oldVal) / oldVal; 
	setControlDisplay ( which, dacVal );
	setResDisplay (which, s.mostRecentResult);
	setChangeVal (which, s.changeInCtrl);
	

	s.servoed = (count < attemptLimit);
	s.currentlyServoing = false;
	if ( !s.servoed )
	{
		thrower( "" + s.servoName + " servo failed to servo!" );
		// and don't adjust the variable value with what is probably a bad value. 
	}
	else
	{
		globals->adjustVariableValue( str(s.servoName + "__servo_value",13, false, true), dacVal );
	}
}


void ServoManager::setChangeVal(UINT which, double change)
{
	servoList.SetItem ((change < 0 ? "" : "+") + str (change*100), which, 4);
	servoList.RedrawWindow ();
}

void ServoManager::setControlDisplay (UINT which, double value )
{
	servoList.SetItem ( str ( value ), which, 3 );
	servoList.RedrawWindow ( );
}

void ServoManager::setResDisplay (UINT which, double value)
{
	servoList.SetItem (str (value), which, 5);
	servoList.RedrawWindow ();
}


