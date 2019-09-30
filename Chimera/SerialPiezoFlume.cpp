#include "stdafx.h"
#include "SerialPiezoFlume.h"

SerialPiezoFlume::SerialPiezoFlume ( bool sMode, std::string addr ) : safemode(sMode), flumeCore( sMode, addr )
{

}

std::string SerialPiezoFlume::getDeviceInfo ( )
{
	return flumeCore.query ( "fuck you" );
}

void SerialPiezoFlume::open ( )
{
	//flumeCore.open();
}
