#pragma once
#include "VisaFlume.h"

class OscilloscopeViewer
{
	public:
		OscilloscopeViewer( std::string usbAddress, bool safemode );
	private:
		const std::string usbAddress;
		VisaFlume visa;
	
};