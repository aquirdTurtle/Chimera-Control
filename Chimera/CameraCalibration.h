// created by Mark O. Brown
#pragma once
#include "Control.h"
#include "myButton.h"
/*
Home of gui options for the camera calibration.
*/
class CameraCalibration
{
	public:
		void initialize( cameraPositions& pos, int& id, CWnd* parent, cToolTips& tooltips );
		bool autoCal( );
		void rearrange( AndorRunModes::mode cameraMode, AndorTriggerMode::mode triggerMode, int width, int height, fontMap fonts );
		bool use( );
		void setAutoCal(bool option);
		void setUse(bool option);
	private:
		Control<CStatic> header;
		Control<CleanPush> calButton;
		Control<CleanCheck> autoCalButton;
		Control<CleanCheck> useButton;
};