#include "stdafx.h"
#include "SerialPiezoFlume.h"
#include <boost/lexical_cast.hpp>

SerialPiezoFlume::SerialPiezoFlume ( bool sMode, std::string addr ) : safemode(sMode), flumeCore( sMode, addr )
{

}

std::string SerialPiezoFlume::getDeviceInfo ( )
{
	return flumeCore.query ( "i\r" );
}

void SerialPiezoFlume::open ( ) 
{
}
void SerialPiezoFlume::close ()
{

}
void SerialPiezoFlume::setXAxisVoltage (double val)
{

}
double SerialPiezoFlume::getXAxisVoltage ()
{
	auto res = flumeCore.query ("xr?\r");
	double dres = 0;
	try
	{
		dres = boost::lexical_cast<double>(dres);
	}
	catch (boost::bad_lexical_cast)
	{
		throwNested ("ERROR: Failed to read x-axis voltage from serial piezo driver?");
	}
	return dres;
}
void SerialPiezoFlume::setYAxisVoltage (double val)
{
	flumeCore.write ("YV " + str (val));
}
double SerialPiezoFlume::getYAxisVoltage ()
{
	return 0;
}
void SerialPiezoFlume::setZAxisVoltage (double val)
{
	flumeCore.write ("ZV" + str (val));
}
double SerialPiezoFlume::getZAxisVoltage ()
{
	return 0;
}