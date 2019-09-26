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

void PiezoCore::setCtrl ( bool ctrl )
{
	ctrlOption = ctrl;
}

bool PiezoCore::wantsCtrl ( )
{
	return ctrlOption;
}

void PiezoCore::evaluateVariations (std::vector<std::vector<parameterType>>& params, UINT totalVariations )
{
	try
	{
		for ( auto seqNum : range ( params.size ( ) ) )
		{
			experimentVals[ seqNum ].x.assertValid ( params[ seqNum ], PIEZO_PARAMETER_SCOPE );
			experimentVals[ seqNum ].y.assertValid ( params[ seqNum ], PIEZO_PARAMETER_SCOPE );
			experimentVals[ seqNum ].z.assertValid ( params[ seqNum ], PIEZO_PARAMETER_SCOPE );
			experimentVals[ seqNum ].x.internalEvaluate ( params[ seqNum ], totalVariations );
			experimentVals[ seqNum ].y.internalEvaluate ( params[ seqNum ], totalVariations );
			experimentVals[ seqNum ].z.internalEvaluate ( params[ seqNum ], totalVariations );
		}
	}
	catch ( Error& )
	{
		throwNested ( "Failed to evaluate piezo expression varations!" );
	}
}

std::pair<piezoChan<std::string>, bool> PiezoCore::getPiezoSettingsFromConfig ( std::ifstream& file, Version ver )
{
	piezoChan<std::string> valVec;
	file.get ( );
	std::getline ( file, valVec.x );
	std::getline ( file, valVec.y );
	std::getline ( file, valVec.z );
	bool ctrlOption;
	file >> ctrlOption;
	file.get ( );
	return { valVec, ctrlOption };
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
