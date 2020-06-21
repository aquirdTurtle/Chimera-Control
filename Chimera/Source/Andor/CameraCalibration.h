// created by Mark O. Brown
#pragma once
#include "Control.h"
#include "CustomMfcControlWrappers/myButton.h"
#include "PrimaryWindows/IChimeraWindowWidget.h"
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
/*
Home of gui options for the camera calibration.
*/
class CameraCalibration
{
	public:
		void initialize( POINT& pos, IChimeraWindowWidget* parent );
		bool autoCal( );
		void rearrange( int width, int height, fontMap fonts );
		bool use( );
		void setAutoCal(bool option);
		void setUse(bool option);
	private:
		QLabel* header;
		QPushButton* calButton;
		QCheckBox* autoCalButton;
		QCheckBox* useButton;
};