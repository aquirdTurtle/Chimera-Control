#include "stdafx.h"
#include "Servo.h"

void Servo::initialize( POINT& pos, cToolTips& toolTips, CWnd* parent, int& id, std::string name,
						UINT aiNum, UINT aoNum )
{
	aiInputChannel = aiNum;
	aoControlChannel = aoNum;
	servoNameDisp.sPos = {pos.x, pos.y, pos.x += 100, pos.y + 20};
	servoNameDisp.Create(name.c_str(), NORM_HEADER_OPTIONS, servoNameDisp.sPos, parent, id++);
	servoValueLabel.sPos = { pos.x, pos.y, pos.x += 100, pos.y + 20 };
	servoValueLabel.Create("Servo-Value:", NORM_STATIC_OPTIONS, servoValueLabel.sPos, parent, id++);
	servoValueEdit.sPos = { pos.x, pos.y, pos.x += 50, pos.y + 20 };
	servoValueEdit.Create(NORM_EDIT_OPTIONS, servoValueEdit.sPos, parent, id++);
	aiInputDisp.sPos = { pos.x, pos.y, pos.x += 50, pos.y + 20 };
	aiInputDisp.Create("1", NORM_STATIC_OPTIONS, aiInputDisp.sPos, parent, id++ );
	aoOutputDisp.sPos = { pos.x, pos.y, pos.x += 50, pos.y + 20 };
	aoOutputDisp.Create( "15", NORM_STATIC_OPTIONS, aoOutputDisp.sPos, parent, id++ );
	controlValueLabel.sPos = { pos.x, pos.y, pos.x += 50, pos.y + 20 };
	controlValueLabel.Create( "Control:", NORM_STATIC_OPTIONS, controlValueLabel.sPos, parent, id++ );
	controlValueDisp.sPos = { pos.x, pos.y, pos.x += 80, pos.y += 20 };
	controlValueDisp.Create( "0", NORM_STATIC_OPTIONS, controlValueDisp.sPos, parent, id++ );
	pos.x -= 480;
}


void Servo::rearrange( UINT width, UINT height, fontMap fonts )
{
	servoNameDisp.rearrange(width, height, fonts);
	servoValueLabel.rearrange( width, height, fonts );
	servoValueEdit.rearrange( width, height, fonts );
	aiInputDisp.rearrange( width, height, fonts );
	aoOutputDisp.rearrange( width, height, fonts );
	controlValueLabel.rearrange( width, height, fonts );
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