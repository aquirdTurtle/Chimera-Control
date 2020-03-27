// created by Mark O. Brown
#include "stdafx.h"
#include "AiSystem.h"

AiSystem::AiSystem( ) : daqmx( ANALOG_IN_SAFEMODE ) 
{
}


/*
 *	We use a PCI card for analog input currently.
 */
void AiSystem::initDaqmx( )
{
	daqmx.createTask( "Analog-Input", analogInTask0 );
	daqmx.createAiVoltageChan( analogInTask0, cstr(boardName + "/ai0:7"), "", DAQmx_Val_Diff, -10.0, 10.0, DAQmx_Val_Volts, NULL );
}


std::string AiSystem::getSystemStatus( )
{
	long answer = daqmx.getProductCategory( cstr(boardName) );
	std::string answerStr = "AI System: Connected... device category = " + str( answer );
	return answerStr;
}


void AiSystem::refreshDisplays( )
{
	for ( auto dispInc : range(voltDisplays.size()))
	{
		voltDisplays[dispInc].SetWindowTextA( str(currentValues[dispInc], 4).c_str() );
	}
}


void AiSystem::rearrange( int width, int height, fontMap fonts )
{
	title.rearrange(width, height, fonts);
	for ( auto& label : dacLabels )
	{
		label.rearrange( width, height, fonts );
	}
	for ( auto& disp : voltDisplays )
	{
		disp.rearrange( width, height, fonts );
	}
	getValuesButton.rearrange( width, height, fonts );
	continuousQueryCheck.rearrange( width, height, fonts );
	queryBetweenVariations.rearrange( width, height, fonts );
	continuousInterval.rearrange (width, height, fonts);
	continuousIntervalLabel.rearrange( width, height, fonts );
	avgNumberEdit.rearrange (width, height, fonts);
	avgNumberLabel.rearrange (width, height, fonts);
}


void AiSystem::initialize( POINT& loc, CWnd* parent, int& id )
{
	initDaqmx( );
	title.sPos = {loc.x, loc.y, loc.x + 480, loc.y += 25};
	title.Create( "ANALOG-INPUT", NORM_HEADER_OPTIONS, title.sPos, parent, id );
	title.fontType = fontTypes::HeadingFont;
	getValuesButton.sPos = { loc.x, loc.y, loc.x += 160, loc.y + 25 };
	getValuesButton.Create( "Get Values", NORM_PUSH_OPTIONS, getValuesButton.sPos, parent, ID_GET_ANALOG_IN_VALUES );
	continuousQueryCheck.sPos = { loc.x, loc.y, loc.x += 160, loc.y + 25 };
	continuousQueryCheck.Create( "Qry Cont.", NORM_CHECK_OPTIONS, continuousQueryCheck.sPos, parent, id++ );
	queryBetweenVariations.sPos = { loc.x, loc.y, loc.x += 160, loc.y += 25 };
	queryBetweenVariations.Create( "Qry Btwn Vars", NORM_CHECK_OPTIONS, queryBetweenVariations.sPos, parent, id++ );
	loc.x -= 480; 
	continuousIntervalLabel.sPos = { loc.x, loc.y, loc.x + 160, loc.y + 20 };
	continuousIntervalLabel.Create ("Cont. Interval:", NORM_STATIC_OPTIONS, continuousIntervalLabel.sPos, parent, id++);
	continuousInterval.sPos = { loc.x + 160, loc.y, loc.x + 240, loc.y + 20 };
	continuousInterval.Create (NORM_EDIT_OPTIONS, continuousInterval.sPos, parent, id++);
	continuousInterval.SetWindowText (cstr (AiSettings ().continuousModeInterval));

	avgNumberLabel.sPos = { loc.x + 240, loc.y, loc.x + 400, loc.y + 20 };
	avgNumberLabel.Create ("# To Avg:", NORM_STATIC_OPTIONS, avgNumberLabel.sPos, parent, id++);
	avgNumberEdit.sPos = { loc.x + 400, loc.y, loc.x + 480, loc.y += 20 };
	avgNumberEdit.Create (NORM_EDIT_OPTIONS, avgNumberEdit.sPos, parent, id++);
	avgNumberEdit.SetWindowText (cstr (AiSettings ().numberMeasurementsToAverage));
	
	// there's a single label first, hence the +1.
	long dacInc = 0, collumnInc = 0, numCols=4;
	LONG colSize = LONG(480 / numCols);
	for ( auto& disp : voltDisplays )
	{
		if ( dacInc == (collumnInc + 1) * NUMBER_AI_CHANNELS / numCols )
		{	// then next column. 
			collumnInc++;
			loc.y -= 20 * NUMBER_AI_CHANNELS / numCols;
		}
		disp.sPos = { loc.x + 20 + collumnInc * colSize, loc.y, loc.x + (collumnInc + 1) * colSize, loc.y += 20 };
		disp.colorState = 0;
		disp.Create( "0", NORM_STATIC_OPTIONS, disp.sPos, parent, id++ );
		dacInc++;
	}
	collumnInc = 0;
	loc.y -= 20 * voltDisplays.size( ) / numCols;

	for ( auto dacInc : range( NUMBER_AI_CHANNELS ) )
	{
		auto& label = dacLabels[dacInc];
		if ( dacInc == (collumnInc + 1) * NUMBER_AI_CHANNELS / numCols)
		{	// then next column
			collumnInc++;
			loc.y -= 20 * NUMBER_AI_CHANNELS / numCols;
		}
		label.sPos = { loc.x + collumnInc * colSize, loc.y, loc.x + 20 + collumnInc * colSize, loc.y += 20 };
		label.Create( cstr( dacInc ), WS_CHILD | WS_VISIBLE | SS_CENTER, dacLabels[dacInc].sPos, parent, ID_DAC_FIRST_EDIT + dacInc );
	}
}

AiSettings AiSystem::getAiSettings ()
{
	AiSettings settings;
	settings.queryBtwnVariations = queryBetweenVariations.GetCheck ();
	settings.queryContinuously = continuousQueryCheck.GetCheck ();
	try
	{
		settings.continuousModeInterval = continuousInterval.getWindowTextAsDouble();
	}
	catch (Error &) { errBox ("Failed to convert ai-system number of measurements to average string to uint!"); };
	try
	{
		settings.numberMeasurementsToAverage = avgNumberEdit.getWindowTextAsUINT ();
		if (settings.numberMeasurementsToAverage < 2)
		{
			settings.numberMeasurementsToAverage = 2;
			setAiSettings (settings);
		}
	}
	catch (Error &) { errBox ("Failed to convert ai-system number of measurements to average string to uint!"); };
	return settings;
}

AiSettings AiSystem::getAiSettingsFromConfig (ConfigStream& file, Version ver)
{
	AiSettings settings;
	file >> settings.queryBtwnVariations;
	file >> settings.queryContinuously;
	file >> settings.numberMeasurementsToAverage;
	file >> settings.continuousModeInterval;
	return settings;
}

void AiSystem::handleSaveConfig (ConfigStream& file)
{
	auto settings = getAiSettings ();
	file << configDelim 
		<< "\n/*Query Between Variations?*/ " << settings.queryBtwnVariations 
		<< "\n/*Query Continuously?*/ " << settings.queryContinuously 
		<< "\n/*Average Number:*/ " << settings.numberMeasurementsToAverage 
		<< "\n/*Contiuous Mode Interval:*/ " << settings.continuousModeInterval 
		<< "\nEND_" + configDelim + "\n";
}

void AiSystem::setAiSettings (AiSettings settings)
{
	queryBetweenVariations.SetCheck (settings.queryBtwnVariations);
	continuousQueryCheck.SetCheck (settings.queryContinuously);
	avgNumberEdit.SetWindowText (cstr(settings.numberMeasurementsToAverage));
	continuousInterval.SetWindowText (cstr (settings.continuousModeInterval));
}


bool AiSystem::wantsContinuousQuery( )
{
	return continuousQueryCheck.GetCheck( );
}


void AiSystem::refreshCurrentValues( )
{
	currentValues = getSingleSnapArray( getAiSettings().numberMeasurementsToAverage );
}


void AiSystem::armAquisition( UINT numSnapshots )
{
	// may need to use numSnapshots X NUM_AI_CHANNELS?
	daqmx.configSampleClkTiming( analogInTask0, "", 10000.0, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, numSnapshots );
	daqmx.startTask( analogInTask0 );
	aquisitionData = std::vector<float64>( NUMBER_AI_CHANNELS * numSnapshots );
}


std::vector<float64> AiSystem::getCurrentValues( )
{
	return { currentValues.begin( ), currentValues.end( ) };
}


void AiSystem::getAquisitionData( )
{
	int32 sampsRead;
	daqmx.readAnalogF64( analogInTask0, aquisitionData, sampsRead );
}


bool AiSystem::wantsQueryBetweenVariations( )
{
	return queryBetweenVariations.GetCheck( );
}


std::vector<float64> AiSystem::getSingleSnap( UINT n_to_avg )
{
	try
	{
		daqmx.configSampleClkTiming( analogInTask0, "", 10000.0, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, n_to_avg );
		daqmx.startTask( analogInTask0 );
		// don't understand why but need 2 samples min???
		std::vector<float64> tmpdata( NUMBER_AI_CHANNELS*n_to_avg );
		int32 sampsRead;
		daqmx.readAnalogF64( analogInTask0, tmpdata, sampsRead );
		daqmx.stopTask( analogInTask0 );
		std::vector<float64> data( NUMBER_AI_CHANNELS );
		UINT count = 0;
		for ( auto& d : data )
		{
			d = 0;
			for ( auto i : range( n_to_avg ) )
			{
				d += tmpdata[count++];
			}
			d /= n_to_avg;
		}
		return data;
	}
	catch (Error &)
	{
		throwNested ("Error exception thrown while getting Ai system single snap!");
	}
}


double AiSystem::getSingleChannelValue( UINT chan, UINT n_to_avg )
{
	auto all = getSingleSnap( n_to_avg );
	return all[chan];
}


std::array<float64, NUMBER_AI_CHANNELS> AiSystem::getSingleSnapArray( UINT n_to_avg )
{
	std::vector<float64> data = getSingleSnap( n_to_avg );
	std::array<float64, NUMBER_AI_CHANNELS> retData;
	for ( auto dataInc : range( NUMBER_AI_CHANNELS ) )
	{
		retData[dataInc] = data[dataInc];
	}
	return retData;
}

