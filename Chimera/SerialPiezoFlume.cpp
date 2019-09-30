#include "stdafx.h"
#include "SerialPiezoFlume.h"

SerialPiezoFlume::SerialPiezoFlume ( bool sMode, std::string sn ) : safemode(sMode), flumeCore( sMode, "" )
{

}


void SerialPiezoFlume::open ( )
{
	flumeCore;
}
