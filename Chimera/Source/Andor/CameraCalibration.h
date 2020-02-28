// created by Mark O. Brown
#pragma once
#include "Control.h"
#include "CustomMfcControlWrappers/myButton.h"

/*
Home of gui options for the camera calibration.
*/
class CameraCalibration
{
	public:
		void initialize( POINT& pos, int& id, CWnd* parent, cToolTips& tooltips );
		bool autoCal( );
		void rearrange( int width, int height, fontMap fonts );
		bool use( );
		void setAutoCal(bool option);
		void setUse(bool option);
	private:
		Control<CStatic> header;
		Control<CleanPush> calButton;
		Control<CleanCheck> autoCalButton;
		Control<CleanCheck> useButton;
};