﻿// created by Mark O. Brown
#include "stdafx.h"
#include "AiSystem.h"
#include <qtimer.h>

AiSystem::AiSystem( ) : daqmx( ANALOG_IN_SAFEMODE ) {
}

/*
 *	We use a PCI card for analog input currently.
 */
void AiSystem::initDaqmx( ){
	daqmx.createTask( "Analog-Input", analogInTask0 );
	daqmx.createAiVoltageChan( analogInTask0, cstr(boardName + "/ai0:7"), "", DAQmx_Val_Diff, -10.0, 10.0, DAQmx_Val_Volts, NULL );
}


std::string AiSystem::getSystemStatus( ){
	long answer = daqmx.getProductCategory( cstr(boardName) );
	std::string answerStr = "AI System: Connected... device category = " + str( answer );
	return answerStr;
}


void AiSystem::refreshDisplays( ){
	for ( auto dispInc : range(voltDisplays.size())){
		voltDisplays[dispInc]->setText( str(currentValues[dispInc], 4).c_str() );
	}
}


void AiSystem::initialize (POINT& loc, IChimeraQtWindow* parent) {
	initDaqmx ();
	title = new QLabel ("ANALOG-INPUT", parent);
	title->setGeometry ({ QPoint{loc.x, loc.y}, QPoint{loc.x + 480, loc.y += 25} });

	getValuesButton = new CQPushButton ("Get Values", parent);
	getValuesButton->setGeometry (loc.x, loc.y, 160, 25);
	parent->connect (getValuesButton, &QPushButton::released, [this]() { refreshCurrentValues (); refreshDisplays (); });
	loc.x += 160;
	continuousQueryCheck = new CQCheckBox ("Qry Cont.", parent);
	continuousQueryCheck->setGeometry (loc.x, loc.y, 160, 25);
	loc.x += 160;
	queryBetweenVariations = new CQCheckBox ("Qry Btwn Vars", parent);
	queryBetweenVariations->setGeometry (loc.x, loc.y, 160, 25);
	loc.y += 25;
	loc.x -= 320; 
	continuousIntervalLabel = new QLabel ("Cont. Interval:", parent);
	continuousIntervalLabel->setGeometry (loc.x, loc.y, 160, 20);
	continuousInterval = new CQLineEdit (qstr (AiSettings ().continuousModeInterval), parent);
	continuousInterval->setGeometry (loc.x + 160, loc.y, 80, 20);
	QTimer::singleShot (1000, this, &AiSystem::handleTimer);
	avgNumberLabel = new QLabel ("# To Avg:", parent);
	avgNumberLabel->setGeometry (loc.x + 240, loc.y, 160, 20);

	avgNumberEdit = new CQLineEdit (qstr (AiSettings ().numberMeasurementsToAverage), parent);
	avgNumberEdit->setGeometry (loc.x + 400, loc.y, 80, 20);
	loc.y += 20;
	// there's a single label first, hence the +1.
	long dacInc = 0, collumnInc = 0, numCols=4;
	LONG colSize = LONG(480 / numCols);
	for ( auto& disp : voltDisplays ){
		if ( dacInc == (collumnInc + 1) * NUMBER_AI_CHANNELS / numCols ){	// then next column. 
			collumnInc++;
			loc.y -= 20 * NUMBER_AI_CHANNELS / numCols;
		}
		disp = new QLabel ("0", parent);
		disp->setGeometry (loc.x + 20 + collumnInc * colSize, loc.y, colSize-20, 20);
		loc.y += 20;
		dacInc++;
	}
	collumnInc = 0;
	loc.y -= LONG (20 * voltDisplays.size( ) / numCols);

	for ( auto dacInc : range( NUMBER_AI_CHANNELS ) ){
		auto& label = dacLabels[dacInc];
		if ( dacInc == (collumnInc + 1) * NUMBER_AI_CHANNELS / numCols)	{	// then next column
			collumnInc++;
			loc.y -= 20 * NUMBER_AI_CHANNELS / numCols;
		}
		label = new QLabel (cstr (dacInc), parent);
		label->setGeometry (loc.x + collumnInc * colSize, loc.y, 20, 20);
		QFont font = label->font ();
		font.setUnderline (true);
		label->setFont (font);
		loc.y += 20;
	}
}

void AiSystem::handleTimer () {
	if (continuousQueryCheck->isChecked ()) {
		refreshCurrentValues (); 
		refreshDisplays ();
	}
	int interval = 1000;
	try {
		interval = boost::lexical_cast<int>(str(continuousInterval->text ()));
	}
	catch (boost::bad_lexical_cast&) { // just go with 1s if the input is invalid.
	}
	QTimer::singleShot (interval, this, &AiSystem::handleTimer);
}

AiSettings AiSystem::getAiSettings (){
	AiSettings settings;
	settings.queryBtwnVariations = queryBetweenVariations->isChecked ();
	settings.queryContinuously = continuousQueryCheck->isChecked ();
	try{
		settings.continuousModeInterval = boost::lexical_cast<int>(str(continuousInterval->text()));
	}
	catch (ChimeraError &) { errBox ("Failed to convert ai-system number of measurements to average string to int!"); };
	try{
		settings.numberMeasurementsToAverage = boost::lexical_cast<unsigned>(str(avgNumberEdit->text()));
		if (settings.numberMeasurementsToAverage < 2){
			settings.numberMeasurementsToAverage = 2;
			setAiSettings (settings);
		}
	}
	catch (ChimeraError &) { errBox ("Failed to convert ai-system number of measurements to average string to unsigned int!"); };
	return settings;
}

AiSettings AiSystem::getSettingsFromConfig (ConfigStream& file){
	AiSettings settings;
	file >> settings.queryBtwnVariations;
	file >> settings.queryContinuously;
	file >> settings.numberMeasurementsToAverage;
	file >> settings.continuousModeInterval;
	return settings;
}

void AiSystem::handleSaveConfig (ConfigStream& file){
	auto settings = getAiSettings ();
	file << configDelim 
		<< "\n/*Query Between Variations?*/ " << settings.queryBtwnVariations 
		<< "\n/*Query Continuously?*/ " << settings.queryContinuously 
		<< "\n/*Average Number:*/ " << settings.numberMeasurementsToAverage 
		<< "\n/*Contiuous Mode Interval:*/ " << settings.continuousModeInterval 
		<< "\nEND_" + configDelim + "\n";
}

void AiSystem::setAiSettings (AiSettings settings){
	queryBetweenVariations->setChecked (settings.queryBtwnVariations);
	continuousQueryCheck->setChecked (settings.queryContinuously);
	avgNumberEdit->setText (qstr(settings.numberMeasurementsToAverage));
	continuousInterval->setText (qstr (settings.continuousModeInterval));
}


bool AiSystem::wantsContinuousQuery( ){
	return continuousQueryCheck->isChecked( );
}


void AiSystem::refreshCurrentValues( ){
	currentValues = getSingleSnapArray( getAiSettings().numberMeasurementsToAverage );
}


void AiSystem::armAquisition( unsigned numSnapshots ){
	// may need to use numSnapshots X NUM_AI_CHANNELS?
	daqmx.configSampleClkTiming( analogInTask0, "", 10000.0, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, numSnapshots );
	daqmx.startTask( analogInTask0 );
	aquisitionData = std::vector<float64>( NUMBER_AI_CHANNELS * numSnapshots );
}

std::vector<float64> AiSystem::getCurrentValues( ){
	return { currentValues.begin( ), currentValues.end( ) };
}

void AiSystem::getAquisitionData( ){
	int32 sampsRead;
	daqmx.readAnalogF64( analogInTask0, aquisitionData, sampsRead );
}

bool AiSystem::wantsQueryBetweenVariations( ){
	return queryBetweenVariations->isChecked( );
}

std::vector<float64> AiSystem::getSingleSnap( unsigned n_to_avg ){
	try{
		daqmx.configSampleClkTiming( analogInTask0, "", 10000.0, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, n_to_avg );
		daqmx.startTask( analogInTask0 );
		// don't understand why but need 2 samples min???
		std::vector<float64> tmpdata( NUMBER_AI_CHANNELS*n_to_avg );
		int32 sampsRead;
		daqmx.readAnalogF64( analogInTask0, tmpdata, sampsRead );
		daqmx.stopTask( analogInTask0 );
		std::vector<float64> data( NUMBER_AI_CHANNELS );
		unsigned count = 0;
		for ( auto& d : data ){
			d = 0;
			for ( auto i : range( n_to_avg ) ){
				d += tmpdata[count++];
			}
			d /= n_to_avg;
		}
		return data;
	}
	catch (ChimeraError &){
		throwNested ("Error exception thrown while getting Ai system single snap!");
	}
}


double AiSystem::getSingleChannelValue( unsigned chan, unsigned n_to_avg ){
	auto all = getSingleSnap( n_to_avg );
	return all[chan];
}


std::array<float64, NUMBER_AI_CHANNELS> AiSystem::getSingleSnapArray( unsigned n_to_avg ){
	std::vector<float64> data = getSingleSnap( n_to_avg );
	std::array<float64, NUMBER_AI_CHANNELS> retData;
	for ( auto dataInc : range( NUMBER_AI_CHANNELS ) ){
		retData[dataInc] = data[dataInc];
	}
	return retData;
}

void AiSystem::logSettings (DataLogger& log, ExpThreadWorker* threadworker){
}
