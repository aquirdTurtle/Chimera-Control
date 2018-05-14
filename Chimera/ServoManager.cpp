#include "stdafx.h"
#include "ServoManager.h"


void ServoManager::initialize( POINT& pos, cToolTips& toolTips, CWnd* parent, int& id,
							   AiSystem* ai_in, AoSystem* ao_in, DioSystem* ttls_in, ParameterSystem* globals_in )
{
	servosHeader.sPos = {pos.x, pos.y, pos.x + 480, pos.y += 20};
	servosHeader.Create( "SERVOS", NORM_HEADER_OPTIONS, servosHeader.sPos, parent, id++ );
	servoButton.sPos = { pos.x, pos.y, pos.x + 300, pos.y + 20 };
	servoButton.Create( "Servo-Once", NORM_PUSH_OPTIONS, servoButton.sPos, parent, IDC_SERVO_CAL );
	autoServoButton.sPos = { pos.x + 300, pos.y, pos.x + 480, pos.y += 20 };
	autoServoButton.Create( "Auto-Servo", NORM_CHECK_OPTIONS, autoServoButton.sPos, parent, id++ );

	toleranceLabel.sPos = { pos.x, pos.y, pos.x + 240, pos.y + 20 };
	toleranceLabel.Create("Tolerance (V):", NORM_STATIC_OPTIONS, toleranceLabel.sPos, parent, id++ );
	toleranceEdit.sPos = { pos.x + 240, pos.y, pos.x + 480, pos.y += 20 };
	toleranceEdit.Create( NORM_EDIT_OPTIONS, toleranceEdit.sPos, parent, id++ );
	toleranceEdit.SetWindowTextA( "0.05" );
	std::vector<LONG> positions = { 0, 110, 170, 270, 320, 370, 480 };

	UINT posCount = 0;
	nameHeader.sPos = { pos.x + positions[posCount], pos.y, pos.x + positions[++posCount], pos.y + 20 };
	nameHeader.Create( "Name", NORM_STATIC_OPTIONS, nameHeader.sPos, parent, id++ );
	activeHeader.sPos = { pos.x + positions[posCount], pos.y, pos.x + positions[++posCount], pos.y + 20 };
	activeHeader.Create("Active?", NORM_STATIC_OPTIONS, activeHeader.sPos, parent, id++ );
	valueHeader.sPos = { pos.x + positions[posCount], pos.y, pos.x + positions[++posCount], pos.y + 20 };
	valueHeader.Create("Set-Point", NORM_STATIC_OPTIONS, valueHeader.sPos, parent, id++ );
	aiNumberHeader.sPos = { pos.x + positions[posCount], pos.y, pos.x + positions[++posCount], pos.y + 20 };
	aiNumberHeader.Create("Ai", NORM_STATIC_OPTIONS, aiNumberHeader.sPos, parent, id++ );
	aoNumberHeader.sPos = { pos.x + positions[posCount], pos.y, pos.x + positions[++posCount], pos.y + 20 };
	aoNumberHeader.Create("Ao", NORM_STATIC_OPTIONS, aoNumberHeader.sPos, parent, id++ );
	controlHeader.sPos = { pos.x + positions[posCount], pos.y, pos.x + positions[++posCount], pos.y + 20 };
	controlHeader.Create("Control", NORM_STATIC_OPTIONS, controlHeader.sPos, parent, id++ );
	pos.y += 20;
	sidemotServo.initialize( pos, toolTips, parent, id, "Sidemot", 6, 22 );
	diagMotServo.initialize( pos, toolTips, parent, id, "DiagMot", 1, 15 );

	ai = ai_in;
	ao = ao_in;
	ttls = ttls_in;
	globals = globals_in;
}

void ServoManager::handleSaveMasterConfig( std::stringstream& configStream )
{
	configStream << toleranceEdit.getWindowTextAsDouble( ) << " " << autoServoButton.GetCheck( ) << "\n";
	configStream << numServos << "\n";
	sidemotServo.handleSaveMasterConfig( configStream );
	diagMotServo.handleSaveMasterConfig( configStream );
}

void ServoManager::handleOpenMasterConfig( std::stringstream& configStream, Version version )
{
	if ( version < Version( "2.1" ) )
	{
		// this was before the servo manager.
		return;
	}
	double tolerance;
	configStream >> tolerance;
	toleranceEdit.SetWindowTextA( cstr( tolerance ) );
	bool autoServo;
	configStream >> autoServo;
	autoServoButton.SetCheck( autoServo );
	LONG numServosInFile;
	configStream >> numServosInFile;
	if ( numServosInFile > 0 )
	{
		sidemotServo.handleOpenMasterConfig( configStream, version );
		numServosInFile--;
	}
	if ( numServosInFile > 0 )
	{
		diagMotServo.handleOpenMasterConfig( configStream, version );
		numServosInFile--;
	}
	while ( numServosInFile > 0 )
	{
		// eat extra info...
		Servo tmpServo;
		tmpServo.handleOpenMasterConfig( configStream, version );
		numServosInFile--;
	}
}

/*
void ServoManager::handleNewMasterConfig( )
{

}
*/

void ServoManager::rearrange( UINT width, UINT height, fontMap fonts )
{
	servosHeader.rearrange( width, height, fonts );
	servoButton.rearrange( width, height, fonts );
	autoServoButton.rearrange( width, height, fonts );
	nameHeader.rearrange( width, height, fonts );
	activeHeader.rearrange( width, height, fonts );
	valueHeader.rearrange( width, height, fonts );
	aiNumberHeader.rearrange( width, height, fonts );
	aoNumberHeader.rearrange( width, height, fonts );
	controlHeader.rearrange( width, height, fonts );

	toleranceLabel.rearrange( width, height, fonts );;
	toleranceEdit.rearrange( width, height, fonts );;
	attemptLimitLabel.rearrange( width, height, fonts );;
	attemptLimitEdit.rearrange( width, height, fonts );;

	sidemotServo.rearrange( width, height, fonts );
	diagMotServo.rearrange( width, height, fonts );
}


bool ServoManager::autoServo( )
{
	return autoServoButton.GetCheck( );
}


void ServoManager::runAll( )
{
	calibrate( sidemotServo );
	calibrate( diagMotServo );
}


void ServoManager::calibrate( Servo& s )
{
	if ( !s.isActive( ) )
	{
		return;
	}
	double tolerance = toleranceEdit.getWindowTextAsDouble();
	double gain = 0.05;
	double sp = s.getSetPoint();
	UINT attemptLimit = 100;
	UINT count = 0;
	UINT aiNum = s.getAiInputChannel( );
	UINT aoNum = s.getAoControlChannel( );
	while ( count++ < attemptLimit )
	{
		double avgVal = ai->getSingleChannelValue(aiNum, 10);
		double percentDif = (sp - avgVal) / sp;
		if ( fabs(percentDif)  < tolerance )
		{
			// found a good value.
			break;
		}
		else
		{
			// modify dac value.
			double currVal = ao->getDacValue( aoNum );
			try
			{
				ao->setSingleDac( aoNum, currVal + gain * percentDif, ttls );
			}
			catch ( Error& err )
			{
				// happens if servo value gives result out of range of dacs.
				count = attemptLimit;
				break;
			}
			// there's a break built in here in order to let the laser settle.
			Sleep( 100 );
			s.setControlDisp( ao->getDacValue( aoNum ) );
		}
	}
	s.servoed = count != attemptLimit;
	if ( !s.servoed )
	{
		errBox( "ERROR: servo failed to servo!" );
	}
	else
	{
		globals->adjustVariableValue( str(s.servoName + "__servo_value",13, false, true), ao->getDacValue( aoNum ) );
	}
}

