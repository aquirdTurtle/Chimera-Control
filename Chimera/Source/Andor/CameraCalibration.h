// created by Mark O. Brown
#pragma once
#include "Control.h"
#include "PrimaryWindows/IChimeraQtWindow.h"
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
		void initialize( QPoint& pos, IChimeraQtWindow* parent );
		bool autoCal( );
		bool use( );
		void setAutoCal(bool option);
		void setUse(bool option);
	private:
		QLabel* header;
		QPushButton* calButton;
		QCheckBox* autoCalButton;
		QCheckBox* useButton;
};