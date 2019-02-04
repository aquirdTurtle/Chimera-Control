#include "stdafx.h"
#include "AiSystem.h"

AiSystem::AiSystem( ) : daqmx( ANALOG_IN_SAFEMODE ) 
{
}


/*
	We use a PCI card for analog input currently.
*/
void AiSystem::initDaqmx( )
{
	daqmx.createTask( "Analog-Input", analogInTask0 );
	daqmx.createAiVoltageChan( analogInTask0, "Dev8/ai0:7", "", DAQmx_Val_Diff, -10.0, 10.0, DAQmx_Val_Volts, NULL );
}


std::string AiSystem::getSystemStatus( )
{
	long answer = daqmx.getProductCategory( "dev8" );
	std::string answerStr = "AI System: Connected... device category = " + str( answer );
	return answerStr;
}


void AiSystem::refreshDisplays( )
{
	for ( auto dispInc : range(voltDisplays.size()))
	{
		voltDisplays[dispInc].SetWindowTextA( cstr(currentValues[dispInc]) );
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
	continuousQueryCheck.Create( "Query Cont.", NORM_CHECK_OPTIONS, continuousQueryCheck.sPos, parent, id++ );
	queryBetweenVariations.sPos = { loc.x, loc.y, loc.x += 160, loc.y += 25 };
	queryBetweenVariations.Create( "Query Btwn Vars?", NORM_CHECK_OPTIONS, queryBetweenVariations.sPos, 
								   parent, id++ );
	loc.x -= 480;
	// there's a single label first, hence the +1.
	long dacInc = 0, collumnInc = 0;
	for ( auto& disp : voltDisplays )
	{
		if ( dacInc == voltDisplays.size( ) / 2)
		{
			collumnInc++;
			// go to second or third collumn
			loc.y -= 25 * voltDisplays.size( ) / 2;
		}
		disp.sPos = { loc.x + 20 + collumnInc * 240, loc.y, loc.x + (collumnInc + 1) * 240, loc.y += 25 };
		disp.colorState = 0;
		disp.Create( "0", NORM_STATIC_OPTIONS, disp.sPos, parent, id++ );
		dacInc++;
	}

	collumnInc = 0;
	loc.y -= 25 * voltDisplays.size( ) / 2;

	for ( auto dacInc : range( NUMBER_AI_CHANNELS ) )
	{
		auto& label = dacLabels[dacInc];
		if ( dacInc == NUMBER_AI_CHANNELS / 2 )
		{
			collumnInc++;
			// go to second or third collumn
			loc.y -= 25 * NUMBER_AI_CHANNELS / 2;
		}
		// create label
		label.sPos = { loc.x + collumnInc * 240, loc.y, loc.x + 20 + collumnInc * 240, loc.y += 25 };
		label.Create( cstr( dacInc ), WS_CHILD | WS_VISIBLE | SS_CENTER,
								  dacLabels[dacInc].sPos, parent, ID_DAC_FIRST_EDIT + dacInc );
	}
}


bool AiSystem::wantsContinuousQuery( )
{
	return continuousQueryCheck.GetCheck( );
}


void AiSystem::refreshCurrentValues( )
{
	currentValues = getSingleSnapArray( 100 );
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

