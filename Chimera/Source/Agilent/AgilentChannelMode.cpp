// created by Mark O. Brown
#include "stdafx.h"
#include "AgilentChannelMode.h"

/*
A constant array which can be used to iterate through all values of DoRows. For example:
for (auto row : DoRows::allRows)
{
// ...
}
*/
const std::array<AgilentChannelMode::which, 7> AgilentChannelMode::allModes = { which::No_Control,which::Output_Off,
	which::DC,which::Sine, which::Square, which::Preloaded, which::Script };


/*
Note that this function is not case-sensitive - it will always convert the input string to lowercase before testing.
*/
AgilentChannelMode::which AgilentChannelMode::fromStr ( std::string rowStr )
{
	for ( auto w : allModes )
	{
		if ( str ( rowStr, 13, false, true ) == toStr ( w ) )
		{
			return w;
		}
	}
	thrower ( "Failed to convert string to Agilent Channel Mode!" );
	return which::No_Control;
}


std::string AgilentChannelMode::toStr ( which m )
{
	switch ( m )
	{
		case which::No_Control:
			return "no_control";
		case which::Output_Off:
			return "output_off";
		case which::DC:
			return "dc";
		case which::Sine:
			return "sine";
		case which::Square:
			return "square";
		case which::Preloaded:
			return "preloaded";
		case which::Script:
			return "script";

	}
	thrower ( "Faied to convert dio row to string!" );
	return "";
}
