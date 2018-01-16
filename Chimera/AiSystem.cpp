#include "stdafx.h"
#include "AiSystem.h"

/*
	We use a PCI card for analog input currently.
*/
void AiSystem::initDaqmx( )
{
	daqmx.createTask( "Analog-Input", analogInTask0 );
	daqmx.createAiVoltageChan( analogInTask0, "Dev???/ai0:15", "", DAQmx_Val_Cfg_Default, -10.0, 10.0, DAQmx_Val_Volts, NULL );
}


void AiSystem::refreshDisplays( )
{
	for ( auto& dispInc : range(voltDisplays.size()))
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
	title.sPos = {loc.x, loc.y, loc.x + 480, loc.y += 25};
	title.Create( "Analog-Input", NORM_HEADER_OPTIONS, title.sPos, parent, id );
	getValuesButton.sPos = { loc.x, loc.y, loc.x += 160, loc.y + 25 };
	getValuesButton.Create( "Get Values", NORM_PUSH_OPTIONS, getValuesButton.sPos, parent, id++);
	continuousQueryCheck.sPos = { loc.x, loc.y, loc.x += 160, loc.y + 25 };
	continuousQueryCheck.Create( "Query Continuously", NORM_CHECK_OPTIONS, continuousQueryCheck.sPos, parent, id++ );
	queryBetweenVariations.sPos = { loc.x, loc.y, loc.x += 160, loc.y += 25 };
	queryBetweenVariations.Create( "Query Between Variations?", NORM_CHECK_OPTIONS, queryBetweenVariations.sPos, 
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


void AiSystem::refreshCurrentValues( )
{
	currentValues = getSingleSnapArray( );
}


void AiSystem::armAquisition( UINT numSnapshots )
{
	// may need to use numSnapshots X NUM_AI_CHANNELS?
	daqmx.configSampleClkTiming( analogInTask0, "", 10000.0, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, numSnapshots );
	daqmx.startTask( analogInTask0 );
	aquisitionData = std::vector<float64>( NUMBER_AI_CHANNELS * numSnapshots );
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

std::vector<float64> AiSystem::getSingleSnap( )
{
	daqmx.configSampleClkTiming( analogInTask0, "", 10000.0, DAQmx_Val_Rising, DAQmx_Val_FiniteSamps, 1 );
	daqmx.startTask( analogInTask0 );
	std::vector<float64> data( NUMBER_AI_CHANNELS );
	int32 sampsRead;
	daqmx.readAnalogF64( analogInTask0, data, sampsRead );
	return data;
}


std::array<float64, NUMBER_AI_CHANNELS> AiSystem::getSingleSnapArray( )
{
	std::vector<float64> data = getSingleSnap( );
	std::array<float64, NUMBER_AI_CHANNELS> retData;
	for ( auto dataInc : range( NUMBER_AI_CHANNELS ) )
	{
		retData[dataInc] = data[dataInc];
	}
	return retData;
}

