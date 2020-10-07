// created by Mark O. Brown
#include "stdafx.h"
#include "BaslerSettings.h"

std::string BaslerAutoExposure::toStr( BaslerAutoExposure::mode m ){
	switch ( m ){
		case mode::Continuous:
			return "Auto-Exposure-Continous";
		case mode::Once:
			return "Auto-Exposure-Once";
		case mode::Off:
			return "Auto-Exposure-Off";
		default:
			return "None";
	}
}

BaslerAutoExposure::mode BaslerAutoExposure::fromStr ( std::string txt ){
	for ( auto m : { mode::Continuous, mode::Once, mode::Off } )	{
		if ( txt == toStr ( m ) ){
			return m;
		}
	}
	errBox ("Failed to convert text (" + txt + ") to balser auto exposure mode! defaulting to auto exposure off.");
	return BaslerAutoExposure::mode::Off;
}


std::string BaslerTrigger::toStr( BaslerTrigger::mode m ){	
	switch ( m ){
		case mode::External:
			return "External-Trigger";
		case mode::AutomaticSoftware:
			return "Automatic-Software-Trigger";
		case mode::ManualSoftware:
			return "Manual-Software-Trigger";
		default:
			return "None";
	}
}


BaslerTrigger::mode BaslerTrigger::fromStr ( std::string txt ){
	for ( auto m : { mode::External, mode::ManualSoftware, mode::AutomaticSoftware } ){
		if ( txt == toStr ( m ) ){
			return m;
		}
	}
	// doesn't match any.
	errBox ("Failed to convert text (" + txt + ") to balser trigger mode! defaulting to external trigger.");
	return BaslerTrigger::mode::External;
}

BaslerAcquisition::mode BaslerAcquisition::fromStr ( std::string txt ){
	for ( auto m : { mode::Continuous, mode::Finite } )	{
		if ( txt == toStr ( m ) ) {
			return m;
		}
	}
	errBox ("Failed to convert text (" + txt + ") to balser acquisition mode! defaulting to Finite acquisition.");
	return BaslerAcquisition::mode::Finite;
}

std::string BaslerAcquisition::toStr ( BaslerAcquisition::mode m ){
	switch ( m ){
		case mode::Finite:
			return "Finite-Acquisition";
		case mode::Continuous:
			return "Continuous-Acquisition";
		default:
			return "None";
	}
}

