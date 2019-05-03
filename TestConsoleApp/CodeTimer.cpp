// created by Mark O. Brown
#include "stdafx.h"
#include "CodeTimer.h"
#include "my_str.h"
#include "range.h"

void CodeTimer::tick(std::string timeName)
{
	times.push_back({ timeName, chronoClock::now() });
}

double CodeTimer::getTime ( bool ms )
{
	// get the most recent time.
	return getTime ( times.size ( ) - 2, ms );
}

double CodeTimer::getTime( UINT which, bool ms)
{
	if ( ms )
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>( times[ which + 1 ].second - times[ which ].second ).count ( ) / 1e6;
	}
	else
	{
		// seconds
		return std::chrono::duration_cast<std::chrono::nanoseconds>( times[ which + 1 ].second - times[ which ].second ).count ( ) / 1e9;
	}
}


std::string CodeTimer::getTimingMessage(bool ms)
{
	std::string msg;
	if ( times.size ( ) <= 1 )
	{
		return "No Timing Info." + str((times.size() == 0 ? " Zero times recorded." : " Only one time recorded."));
	}
	for (auto count : range(times.size() - 1))
	{
		msg += times[count].first + " to " + times[count + 1].first + ":";
		msg += str ( getTime ( count, ms ) ) + (ms? "ms\r\n" : "s\r\n");
	}
	return msg;
}


