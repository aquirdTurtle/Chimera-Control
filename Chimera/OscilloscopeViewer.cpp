#include "stdafx.h"
#include "OscilloscopeViewer.h"

OscilloscopeViewer::OscilloscopeViewer( std::string usbAddress, bool safemode ) : visa( safemode, usbAddress )
{
	errBox( "Scope identity: " + visa.identityQuery( ) );
}
