// created by Mark O. Brown
#include "stdafx.h"
#include "CameraCalibration.h"
#include <GeneralUtilityFunctions/commonFunctions.h>

void CameraCalibration::initialize( POINT& pos, IChimeraQtWindow* parent )
{
	header = new QLabel ("Camera-Bkgd:", parent);
	header->setGeometry (pos.x, pos.y, 140, 20);
	calButton = new QPushButton ("Calibrate", parent);
	calButton->setGeometry (pos.x+140, pos.y, 100, 20);
	parent->connect (calButton, &QPushButton::released, [parent]() {
		commonFunctions::calibrateCameraBackground (parent); });

	autoCalButton = new QCheckBox ("Auto-Cal", parent);
	autoCalButton->setGeometry (pos.x+240, pos.y, 120, 20);
	useButton = new QCheckBox ("Use-Cal", parent);
	useButton->setGeometry (pos.x + 360, pos.y, 120, 20);
}


void CameraCalibration::setAutoCal(bool option)
{
	autoCalButton->setChecked(option);
}


void CameraCalibration::setUse(bool option)
{
	useButton->setChecked(option);
}

bool CameraCalibration::use( )
{
	return useButton->isChecked( );
}


bool CameraCalibration::autoCal( )
{
	return autoCalButton->isChecked ( );
}

