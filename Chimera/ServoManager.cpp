#include "stdafx.h"
#include "ServoManager.h"


void ServoManager::initialize( POINT& pos, cToolTips& toolTips, CWnd* parent, int& id,
							   AiSystem* ai_in, AoSystem* ao_in )
{
	header.sPos = {pos.x, pos.y, pos.x + 480, pos.y += 20};
	header.Create( "SERVOS", NORM_HEADER_OPTIONS, header.sPos, parent, id++ );
	calibrateButton.sPos = { pos.x, pos.y, pos.x + 480, pos.y += 20 };
	calibrateButton.Create( "Calibrate Powers", NORM_PUSH_OPTIONS, calibrateButton.sPos, parent, IDC_SERVO_CAL );
	sidemotServo.initialize( pos, toolTips, parent, id, "Sidemot", 1, 15 );
	diagMotServo.initialize( pos, toolTips, parent, id, "Diag. Mot", 1, 15 );
	ai = ai_in;
	ao = ao_in;
}


void ServoManager::rearrange( UINT width, UINT height, fontMap fonts )
{
	header.rearrange( width, height, fonts );
	calibrateButton.rearrange( width, height, fonts );
	sidemotServo.rearrange( width, height, fonts );
	diagMotServo.rearrange( width, height, fonts );
}


void ServoManager::calibrateAll( )
{
	calibrate( sidemotServo );
	calibrate( diagMotServo );
}


void ServoManager::calibrate( Servo& s )
{
	double tolerance = 0.1;
	double sp = s.getSetPoint();
	UINT aiNum = s.getAiInputChannel( );
	UINT aoNum = s.getAoControlChannel( );
	while ( true )
	{
		double avgVal = ai->getSingleChannelValue(aiNum, 10);
		if ( (sp - avgVal) / sp < tolerance )
		{
			break;
		}
		else
		{
			
		}
	}
}

