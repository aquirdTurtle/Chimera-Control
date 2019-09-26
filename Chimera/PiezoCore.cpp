#include "stdafx.h"
#include "PiezoCore.h"
#include "Version.h"

PiezoCore::PiezoCore ( bool safemode, std::string sn, std::string delim ) : flume (safemode, sn), 
	configDelim( delim )
{
	
}

void PiezoCore::updateExprVals ( std::vector<piezoChan<Expression>> newVals )
{
	experimentVals = newVals;
}

void PiezoCore::programAll ( piezoChan<double> vals )
{
	programXNow ( vals.x );
	programYNow ( vals.y );
	programZNow ( vals.z );
}

void PiezoCore::exprProgramPiezo ( UINT sequenceNumber, UINT variationNumber )
{
	if ( sequenceNumber >= experimentVals.size ( ) )
	{
		thrower ( "Tried to program piezo with sequence which doesn't seem to exist!" );
	}
	programXNow ( experimentVals[ sequenceNumber ].x.getValue ( variationNumber ) );
	programYNow ( experimentVals[ sequenceNumber ].y.getValue ( variationNumber ) );
	programZNow ( experimentVals[ sequenceNumber ].z.getValue ( variationNumber ) );
}

void PiezoCore::evaluateVariations (std::vector<std::vector<parameterType>>& params, UINT totalVariations )
{
	for ( auto seqNum : range ( params.size ( ) ) )
	{
		for ( auto variation : range ( totalVariations ) )
		{
			experimentVals[ seqNum ].x.assertValid ( params[ seqNum ], PIEZO_PARAMETER_SCOPE );
			experimentVals[ seqNum ].y.assertValid ( params[ seqNum ], PIEZO_PARAMETER_SCOPE );
			experimentVals[ seqNum ].z.assertValid ( params[ seqNum ], PIEZO_PARAMETER_SCOPE );
			experimentVals[ seqNum ].x.internalEvaluate ( params[ seqNum ], variation );
			experimentVals[ seqNum ].y.internalEvaluate ( params[ seqNum ], variation );
			experimentVals[ seqNum ].z.internalEvaluate ( params[ seqNum ], variation );
		}
	}
}

piezoChan<std::string> PiezoCore::getPiezoValsFromConfig ( std::ifstream& file, Version ver )
{
	piezoChan<std::string> valVec;
	std::getline ( file, valVec.x );
	std::getline ( file, valVec.y );
	std::getline ( file, valVec.z );
	file.get ( );
	return valVec;
}

void PiezoCore::initialize ( )
{
	flume.open ( );
}

double PiezoCore::getCurrentXVolt ( )
{
	return flume.getXAxisVoltage ( );
}
double PiezoCore::getCurrentYVolt ( )
{
	return flume.getYAxisVoltage ( );
}
double PiezoCore::getCurrentZVolt ( )
{
	return flume.getZAxisVoltage ( );
}
void PiezoCore::programXNow ( double val )
{
	flume.setXAxisVoltage ( val );
}

void PiezoCore::programYNow ( double val )
{
	flume.setYAxisVoltage ( val );
}

void PiezoCore::programZNow ( double val )
{
	flume.setZAxisVoltage ( val );
}

std::string PiezoCore::getDeviceInfo ( )
{
	return flume.getDeviceInfo ( );
}


std::string PiezoCore::getDeviceList ( )
{
	return flume.list ( );
}
