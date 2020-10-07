// created by Mark O. Brown
#include "stdafx.h"
#include "CameraCalibration.h"
#include <GeneralUtilityFunctions/commonFunctions.h>

void CameraCalibration::initialize( QPoint& pos, IChimeraQtWindow* parent ){
	auto& px = pos.rx (), & py = pos.ry ();
	header = new QLabel ("Camera-Bkgd:", parent);
	header->setGeometry (px, py, 140, 20);
	calButton = new QPushButton ("Calibrate", parent);
	calButton->setGeometry (px+140, py, 100, 20);
	parent->connect (calButton, &QPushButton::released, [parent]() {
		commonFunctions::calibrateCameraBackground (parent); });

	autoCalButton = new QCheckBox ("Auto-Cal", parent);
	autoCalButton->setGeometry (px+240, py, 120, 20);
	useButton = new QCheckBox ("Use-Cal", parent);
	useButton->setGeometry (px + 360, py, 120, 20);
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

