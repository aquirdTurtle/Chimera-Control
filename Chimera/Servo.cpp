#include "stdafx.h"
#include "Servo.h"

void Servo::initialize( POINT& pos, cToolTips& toolTips, CWnd* parent, int& id, std::string name,
						UINT aiNum, UINT aoNum )
{
	aiInputChannel = aiNum;
	aoControlChannel = aoNum;
	servoName = name;

	std::vector<LONG> positions = { 0, 110, 170, 270, 320, 370, 480 };
	UINT posCount = 0;
	servoNameDisp.sPos = { pos.x + positions[posCount], pos.y, pos.x + positions[++posCount], pos.y + 20 };
	servoNameDisp.Create( name.c_str(), NORM_HEADER_OPTIONS, servoNameDisp.sPos, parent, id++);
	activeCheck.sPos = { pos.x + positions[posCount], pos.y, pos.x + positions[++posCount], pos.y + 20 };
	activeCheck.Create( "", NORM_CHECK_OPTIONS, activeCheck.sPos, parent, id++ );
	servoValueEdit.sPos = { pos.x + positions[posCount], pos.y, pos.x + positions[++posCount], pos.y + 20 };
	servoValueEdit.Create( NORM_EDIT_OPTIONS, servoValueEdit.sPos, parent, id++);
	aiInputDisp.sPos = { pos.x + positions[posCount], pos.y, pos.x + positions[++posCount], pos.y + 20 };
	aiInputDisp.Create( cstr(aiInputChannel), NORM_STATIC_OPTIONS, aiInputDisp.sPos, parent, id++ );
	aoOutputDisp.sPos = { pos.x + positions[posCount], pos.y, pos.x + positions[++posCount], pos.y + 20 };
	aoOutputDisp.Create( cstr(aoControlChannel), NORM_STATIC_OPTIONS, aoOutputDisp.sPos, parent, id++ );
	controlValueDisp.sPos = { pos.x + positions[posCount], pos.y, pos.x + positions[++posCount], pos.y += 20 };
	controlValueDisp.Create( "0", NORM_STATIC_OPTIONS, controlValueDisp.sPos, parent, id++ );	
}


bool Servo::isActive( )
{
	return activeCheck.GetCheck( );
}


void Servo::setControlDisp( double val )
{
	auto st = str( val );
	controlValueDisp.SetWindowText( st.c_str( ) );
}


void Servo::rearrange( UINT width, UINT height, fontMap fonts )
{
	servoNameDisp.rearrange(width, height, fonts);
	activeCheck.rearrange( width, height, fonts );
 	servoValueEdit.rearrange( width, height, fonts );
	aiInputDisp.rearrange( width, height, fonts );
	aoOutputDisp.rearrange( width, height, fonts );
	controlValueDisp.rearrange( width, height, fonts );
}


double Servo::getSetPoint( )
{
	return servoValueEdit.getWindowTextAsDouble( );
}


UINT Servo::getAiInputChannel( )
{
	return aiInputChannel;
}


UINT Servo::getAoControlChannel( )
{
	return aoControlChannel;
}