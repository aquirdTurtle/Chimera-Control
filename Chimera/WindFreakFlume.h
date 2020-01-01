#pragma once

#include "WinSerialFlume.h"

// this is a small wrapper around a serial flume to introduce microwave-specific commands.
class WindFreakFlume : private WinSerialFlume
{
	public:
		void initiailize ();
		void programList ();
};
