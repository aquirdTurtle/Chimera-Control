#include "stdafx.h"
#include "CameraCalibration.h"

void CameraCalibration::initialize( cameraPositions& pos, int& id, CWnd* parent, cToolTips& tooltips )
{
	header.setPositions(pos, 0, 0, 480, 20, true, false, true);
	header.Create( "Calibrate-Camera-Background", NORM_HEADER_OPTIONS, header.seriesPos, parent, id++ );
	calButton.setPositions( pos, 0, 0, 160, 20, false, false, true );
	calButton.Create( "Calibrate", NORM_PUSH_OPTIONS, calButton.seriesPos, parent, IDC_CAMERA_CALIBRATION_BUTTON );
	autoCalButton.setPositions( pos, 160, 0, 160, 20, false, false, true );
	autoCalButton.Create( "Auto-Cal", NORM_CHECK_OPTIONS, autoCalButton.seriesPos, parent, id++ );
	useButton.setPositions( pos, 320, 0, 160, 20, true, false, true );
	useButton.Create( "Use-Cal", NORM_CHECK_OPTIONS, useButton.seriesPos, parent, id++ );
}


void CameraCalibration::rearrange( std::string cameraMode, std::string triggerMode, int width, int height, 
								   fontMap fonts )
{
	header.rearrange(cameraMode, triggerMode, width, height, fonts );
	calButton.rearrange( cameraMode, triggerMode, width, height, fonts );
	autoCalButton.rearrange( cameraMode, triggerMode, width, height, fonts );
	useButton.rearrange( cameraMode, triggerMode, width, height, fonts );
}


bool CameraCalibration::use( )
{
	return useButton.GetCheck( );
}


bool CameraCalibration::autoCal( )
{
	return autoCalButton.GetCheck( );
}
