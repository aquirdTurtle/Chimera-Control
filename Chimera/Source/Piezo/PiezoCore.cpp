#include "stdafx.h"
#include "Piezo/PiezoCore.h"
#include "Piezo/PiezoType.h"
#include "ConfigurationSystems/Version.h"

PiezoCore::PiezoCore (piezoSetupInfo info) :
	controllerType ( info.type),
	flume ( info.type != PiezoType::B, info.addr),
	serFlume ( info.type != PiezoType::A, info.addr),
	configDelim( info.name )
{}

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
	switch ( controllerType )
	{
		case PiezoType::A:
			serFlume.open ( );
			break;
		case PiezoType::B:
			flume.open ( );
	}
}

double PiezoCore::getCurrentXVolt ( )
{
	switch (controllerType)
	{
		case PiezoType::A:
			return serFlume.getXAxisVoltage ();
		case PiezoType::B:
			return flume.getXAxisVoltage ();
	}
	return 0;
}
double PiezoCore::getCurrentYVolt ()
{
	switch (controllerType)
	{
	case PiezoType::A:
		return serFlume.getYAxisVoltage ();
	case PiezoType::B:
		return flume.getYAxisVoltage ();
	}
	return 0;
}

double PiezoCore::getCurrentZVolt ()
{
	switch (controllerType)
	{
		case PiezoType::A: return serFlume.getZAxisVoltage (); break;
		case PiezoType::B: return flume.getZAxisVoltage (); break;
	}
	return 0;
}

void PiezoCore::programXNow ( double val )
{
	switch (controllerType)
	{
		case PiezoType::A: serFlume.setXAxisVoltage (val); break;
		case PiezoType::B: flume.setXAxisVoltage (val); break;
	}
} 
 
void PiezoCore::programYNow ( double val )
{
	switch (controllerType)
	{
		case PiezoType::A: serFlume.setYAxisVoltage (val); break;
		case PiezoType::B: flume.setYAxisVoltage (val); break;
	}
}

void PiezoCore::programZNow ( double val )
{
	switch (controllerType)
	{
		case PiezoType::A: serFlume.setZAxisVoltage (val); break;
		case PiezoType::B: flume.setZAxisVoltage (val); break;
	}
}

std::string PiezoCore::getDeviceInfo ( )
{
	switch ( controllerType )
	{
		case PiezoType::A:
			return serFlume.getDeviceInfo ( );
		case PiezoType::B:
			return flume.getDeviceInfo ( );
		case PiezoType::NONE:
			return "NONE";
		default:
			return "PIEZO TYPE NOT RECOGNIZED!";
	}
}


std::string PiezoCore::getDeviceList ( )
{
	return flume.list ( );
}
