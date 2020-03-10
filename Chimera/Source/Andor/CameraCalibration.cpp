// created by Mark O. Brown
#include "stdafx.h"
#include "CameraCalibration.h"

void CameraCalibration::initialize( POINT& pos, int& id, CWnd* parent, cToolTips& tooltips )
{
	header.sPos = { pos.x, pos.y, pos.x + 140, pos.y + 20 };
	header.Create( "Camera-Bkgd:", NORM_HEADER_OPTIONS, header.sPos, parent, id++ );
	calButton.sPos = { pos.x + 140, pos.y, pos.x + 240, pos.y + 20 };
	calButton.Create( "Calibrate", NORM_PUSH_OPTIONS, calButton.sPos, parent, IDC_CAMERA_CALIBRATION_BUTTON );
	autoCalButton.sPos = { pos.x + 240, pos.y, pos.x + 360, pos.y + 20 };
	autoCalButton.Create( "Auto-Cal", NORM_CHECK_OPTIONS, autoCalButton.sPos, parent, id++ );
	useButton.sPos = { pos.x + 360, pos.y, pos.x + 480, pos.y += 20 };
	useButton.Create( "Use-Cal", NORM_CHECK_OPTIONS, useButton.sPos, parent, id++ );
}


void CameraCalibration::setAutoCal(bool option)
{
	autoCalButton.SetCheck(option);
}


void CameraCalibration::setUse(bool option)
{
	useButton.SetCheck(option);
}


void CameraCalibration::rearrange( int width, int height, fontMap fonts )
{
	header.rearrange( width, height, fonts );
	calButton.rearrange( width, height, fonts );
	autoCalButton.rearrange( width, height, fonts );
	useButton.rearrange( width, height, fonts );
}


bool CameraCalibration::use( )
{
	return useButton.GetCheck( );
}


bool CameraCalibration::autoCal( )
{
	return autoCalButton.GetCheck( );
}
