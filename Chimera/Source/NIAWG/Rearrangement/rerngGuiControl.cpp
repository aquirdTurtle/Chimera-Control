// created by Mark O. Brown
#include "stdafx.h"
#include "rerngGuiControl.h"
#include "ConfigurationSystems/ConfigSystem.h"
#include <boost/lexical_cast.hpp>

void rerngGuiControl::initialize (QPoint& loc, QWidget* parent ){
	auto& px = loc.rx (), & py = loc.ry ();
	header = new QLabel ("REARRANGEMENT OPTIONS", parent);
	header->setGeometry (px, py, 640, 25);
	experimentIncludesRerng = new QCheckBox ("Experiment Has Rerng?", parent);
	experimentIncludesRerng->setGeometry (px, py += 25, 320, 25);
	parent->connect (experimentIncludesRerng, &QCheckBox::stateChanged, [this]() {updateActive (); });

	flashingRateText = new QLabel ("Flashing Rate (MHz)", parent);
	flashingRateText->setGeometry (px, py+=25, 200, 25);
	flashingRateEdit = new QLineEdit ("1", parent);
	flashingRateEdit->setGeometry (px + 200, py, 120, 25);

	moveSpeedText = new QLabel ("Move Speed (ms)", parent);
	moveSpeedText->setGeometry (px, py += 25, 200, 25);
	moveSpeedEdit = new QLineEdit ("0.06", parent);
	moveSpeedEdit->setGeometry (px+200, py, 120, 25);

	py -= 75;
	px += 320;
	movingBiasText = new QLabel ("Moving Tweezer Bias (/1)", parent);
	movingBiasText->setGeometry (px, py += 25, 200, 25);
	movingBiasEdit = new QLineEdit ("0.3", parent);
	movingBiasEdit->setGeometry (px+200, py, 120, 25);

	deadTimeText = new QLabel ("Dead Time (ns)", parent);
	deadTimeText->setGeometry (px, py += 25, 200, 25);
	deadTimeEdit = new QLineEdit ("0", parent);
	deadTimeEdit->setGeometry (px+200, py, 120, 25);

	staticMovingRatioText = new QLabel ("Static/Moving Ratio", parent);
	staticMovingRatioText->setGeometry (px, py += 25, 200, 25);
	staticMovingRatioEdit = new QLineEdit ("1", parent);
	staticMovingRatioEdit->setGeometry (px+200, py, 120, 25);
	px -= 320;
	outputRearrangeEvents = new QCheckBox ("Output Event Info?", parent);
	outputRearrangeEvents->setGeometry (px, py+=25, 320, 25);
	outputIndividualEvents = new QCheckBox ("Output Individual Event Info?", parent);
	outputIndividualEvents->setGeometry (px, py+=25, 320, 25);
	py -= 50;
	preprogramMoves = new QCheckBox ("Preprogram Moves?", parent);
	preprogramMoves->setGeometry (px + 320, py+=25, 320, 25);
	useCalibration = new QCheckBox ("Use Calibration?", parent);
	useCalibration->setGeometry (px + 320, py += 25, 320, 25);

	finalMoveTimeText = new QLabel ("Final-Move-Time (ms): ", parent);
	finalMoveTimeText->setGeometry (px, py += 25, 200, 25);
	finalMoveTimeEdit = new QLineEdit ("1", parent);
	finalMoveTimeEdit->setGeometry (px+200, py, 120, 25);

	rerngModeCombo = new QComboBox (parent);
	rerngModeCombo->setGeometry (px, py += 25, 320, 25);
	for ( auto m : rerngMode::allModes )
	{
		rerngModeCombo->addItem (rerngMode::toStr (m).c_str ());
	}
	rerngModeCombo->setCurrentIndex (0);

	fastMoveTime = new QLabel ("Fast-Move (us):", parent);
	fastMoveTime->setGeometry (px+320, py, 200, 25);
	fastMoveTimeEdit = new QLineEdit ("2", parent);
	fastMoveTimeEdit->setGeometry (px+520, py, 120, 25);
	py += 25;

	//updateActive ( );
}


void rerngGuiControl::setEnabled (bool enabled)
{
	experimentIncludesRerng->setEnabled (enabled);
	if (enabled)
	{
		updateActive ();
		return;
	}
	flashingRateText->setEnabled (enabled);
	flashingRateEdit->setEnabled (enabled);
	moveSpeedText->setEnabled (enabled);
	moveSpeedEdit->setEnabled (enabled);
	movingBiasText->setEnabled (enabled);
	movingBiasEdit->setEnabled (enabled);
	deadTimeText->setEnabled (enabled);
	deadTimeEdit->setEnabled (enabled);
	staticMovingRatioText->setEnabled (enabled);
	staticMovingRatioEdit->setEnabled (enabled);
	preprogramMoves->setEnabled (enabled);
	useCalibration->setEnabled (enabled);
	outputRearrangeEvents->setEnabled (enabled);
	outputIndividualEvents->setEnabled (enabled);
	finalMoveTimeText->setEnabled (enabled);
	finalMoveTimeEdit->setEnabled (enabled);
	fastMoveTime->setEnabled (enabled);
	fastMoveTimeEdit->setEnabled (enabled);
	rerngModeCombo->setEnabled (enabled);
}


rerngGuiOptions rerngGuiControl::getParams( )
{
	rerngGuiOptions tempParams;
	tempParams.active = experimentIncludesRerng->isChecked( );
	tempParams.outputInfo = outputRearrangeEvents->isChecked ( );
	tempParams.outputIndv = outputIndividualEvents->isChecked ( );
	tempParams.preprogram = preprogramMoves->isChecked ( );
	tempParams.useCalibration = useCalibration->isChecked( );
	try
	{
		tempParams.flashingRate = str (flashingRateEdit->text ());
		tempParams.moveSpeed = str (moveSpeedEdit->text () );
		tempParams.moveBias = str (movingBiasEdit->text ());
		tempParams.deadTime = str (deadTimeEdit->text () );
		tempParams.staticMovingRatio = str (staticMovingRatioEdit->text () );
		tempParams.finalMoveTime = str (finalMoveTimeEdit->text () );
		tempParams.fastMoveTime = str (fastMoveTimeEdit->text () );
		tempParams.rMode = rerngMode::fromStr ( str (rerngModeCombo->currentText () ) );
	}
	catch ( boost::bad_lexical_cast&)
	{
		throwNested ( "Failed to convert rearrangement parameters to correct format! check that the inputs are the "
				 "correct types please." );
	}
	return tempParams;
}

void rerngGuiControl::updateActive ( )
{
	auto params = getParams ( );
	flashingRateEdit->setEnabled ( 0 );
	moveSpeedEdit->setEnabled ( 0 );
	movingBiasEdit->setEnabled ( 0 );
	deadTimeEdit->setEnabled ( 0 );
	staticMovingRatioEdit->setEnabled ( 0 );
	preprogramMoves->setEnabled ( 0 );
	useCalibration->setEnabled ( 0 );
	outputRearrangeEvents->setEnabled ( 0 );
	outputIndividualEvents->setEnabled ( 0 );
	finalMoveTimeEdit->setEnabled ( 0 );
	fastMoveTimeEdit->setEnabled ( 0 );
	rerngModeCombo->setEnabled ( 0 );
	if ( params.active )
	{
		outputRearrangeEvents->setEnabled ( 1 );
		outputIndividualEvents->setEnabled ( 1 );
		rerngModeCombo->setEnabled ( 1 );
		switch ( params.rMode )
		{
			case rerngMode::mode::Lazy:
			{
				break;
			}
			case rerngMode::mode::Antoine:
			{
				break;
			}
			case rerngMode::mode::StandardFlashing:
			{
				flashingRateEdit->setEnabled ( 1 );
				moveSpeedEdit->setEnabled ( 1 );
				movingBiasEdit->setEnabled ( 1 );
				deadTimeEdit->setEnabled ( 1 );
				staticMovingRatioEdit->setEnabled ( 1 );
				preprogramMoves->setEnabled ( 1 );
				useCalibration->setEnabled ( 1 );
				finalMoveTimeEdit->setEnabled ( 1 );
				break;
			}
			case rerngMode::mode::Ultrafast:
			{
				movingBiasEdit->setEnabled ( 1 );
				deadTimeEdit->setEnabled ( 1 );
				preprogramMoves->setEnabled ( 1 );
				useCalibration->setEnabled ( 1 );
				fastMoveTimeEdit->setEnabled ( 1 );
				break;
			}
		}
	}
}


rerngGuiOptions rerngGuiControl::getSettingsFromConfig (ConfigStream& config)
{
	rerngGuiOptions info;
	std::string tmpStr;
	config >> info.active >> info.flashingRate >> info.moveBias >> info.moveSpeed;
	if (config.ver < Version ("2.3"))
	{
		std::string garbage;
		config >> garbage >> garbage;
	}
	if (config.ver > Version ("2.2"))
	{
		config >> info.deadTime >> info.staticMovingRatio;
	}
	else
	{
		info.deadTime = str ("0");
		info.staticMovingRatio = str ("1");
	}
	if (config.ver > Version ("2.5"))
	{
		config >> info.outputInfo;
	}
	else
	{
		info.outputInfo = false;
	}
	if (config.ver > Version ("2.10"))
	{
		config >> info.outputIndv;
	}
	else
	{
		info.outputIndv = false;
	}
	if (config.ver > Version ("2.11"))
	{
		config >> info.preprogram >> info.useCalibration;
	}
	else
	{
		info.preprogram = false;
		info.useCalibration = false;
	}
	if (config.ver > Version ("2.12"))
	{
		config >> tmpStr;
		info.finalMoveTime = tmpStr;
	}
	else
	{
		info.finalMoveTime = str (1e-3);
	}
	if (config.ver > Version ("3.1") && config.ver < Version ("3.6"))
	{
		config >> tmpStr;
	}
	if (config.ver >= Version ("3.6"))
	{
		config >> tmpStr;
		info.rMode = rerngMode::fromStr (tmpStr);
	}
	if (config.ver > Version ("3.1"))
	{
		config >> tmpStr;
		info.fastMoveTime = tmpStr;
	}
	else
	{
		info.fastMoveTime = str (1e-6);
	}
	if (config.ver < Version ("3.6") && config.ver >= Version ("3.4"))
	{
		config >> tmpStr;
	}
	return info;
}


void rerngGuiControl::handleOpenConfig(ConfigStream& openFile)
{
	
	setParams (getSettingsFromConfig (openFile));
}


void rerngGuiControl::handleSaveConfig( ConfigStream& saveFile )
{
 	saveFile << "REARRANGEMENT_INFORMATION\n";
	// conversions happen in getParams.
	rerngGuiOptions info = getParams( );
 	saveFile << "/*Rearrangement Active?*/\t" << info.active 
 			 << "\n/*Flashing Rate:*/\t\t\t" << info.flashingRate 
 			 << "\n/*Move Bias:*/\t\t\t\t" << info.moveBias 
			 << "\n/*Move Speed:*/\t\t\t\t" << info.moveSpeed 
			 << "\n/*Dead Time:*/\t\t\t\t" << info.deadTime 
			 << "\n/*Static to Moving Ratio:*/\t" << info.staticMovingRatio 
			 << "\n/*Output Info?*/\t\t\t" << info.outputInfo 
			 << "\n/*Output Indv?*/\t\t\t" << info.outputIndv 
			 << "\n/*Preprogram?*/\t\t\t\t" << info.preprogram 
			 << "\n/*Use Cal?*/\t\t\t\t" << info.useCalibration 
			 << "\n/*Final Move Time:*/\t\t" << info.finalMoveTime 
			 << "\n/*Rearrange Mode:*/\t\t\t" << rerngMode::toStr ( info.rMode ) 
			 << "\n/*Fast Move Time:*/\t\t\t" << info.fastMoveTime
			 << "\nEND_REARRANGEMENT_INFORMATION\n";
}


void rerngGuiControl::setParams( rerngGuiOptions params )
{
	experimentIncludesRerng->setChecked( params.active );
	// convert back to MHz from Hz
	flashingRateEdit->setText( cstr(params.flashingRate.expressionStr) );
	movingBiasEdit->setText ( cstr( params.moveBias.expressionStr ) );
	// convert back to ms from s
	moveSpeedEdit->setText ( cstr( params.moveSpeed.expressionStr ) );
	outputRearrangeEvents->setChecked ( params.outputInfo );
	// convert back to ns
	deadTimeEdit->setText ( cstr( params.deadTime.expressionStr ) );
	staticMovingRatioEdit->setText ( cstr( params.staticMovingRatio.expressionStr ) );
	
	outputRearrangeEvents->setChecked ( params.outputInfo );
	outputIndividualEvents->setChecked ( params.outputIndv );

	useCalibration->setChecked ( params.useCalibration );
	preprogramMoves->setChecked ( params.preprogram );
	// convert back to ms
	finalMoveTimeEdit->setText ( cstr( params.finalMoveTime.expressionStr ) );
	auto index = rerngModeCombo->findData (rerngMode::toStr (params.rMode).c_str ());
	rerngModeCombo->setCurrentIndex (index);
	// these are displayed in us.
	fastMoveTimeEdit->setText ( cstr( params.fastMoveTime.expressionStr ) );
}


