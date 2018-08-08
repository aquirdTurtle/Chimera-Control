#include "stdafx.h"
#include "CodeTimer.h"
#include "range.h"
#include "my_str.h"

void CodeTimer::tick(std::string timeName)
{
	times.push_back({ timeName, chronoClock::now() });
}


std::string CodeTimer::getTimingMessage(bool ms)
{
	std::string msg;
	for (auto count : range(times.size() - 1))
	{
		msg += times[count].first + " to " + times[count + 1].first + ":";
		if (ms)
		{
			msg += str(std::chrono::duration_cast<std::chrono::nanoseconds>(times[count + 1].second - times[count].second).count() / 1e6) + "ms\r\n";
		}
		else
		{
			msg += str(std::chrono::duration_cast<std::chrono::nanoseconds>(times[count + 1].second - times[count].second).count() / 1e9) + "s\r\n";
		}
	}
	return msg;
}


