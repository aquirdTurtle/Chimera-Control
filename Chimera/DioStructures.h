#pragma once
#include <array>
#include "windows.h"
#include "commonTypes.h"

// this struct keeps variable names.
struct DioCommandForm
{
	// the hardware location of this line
	std::pair<unsigned short, unsigned short> line;
	// the time to make the change
	timeType time;
	// the value to set it to. 
	bool value;
};

// no variables in this version. It's calculated each variation based on corresponding ComandForm structs.
struct DioCommand
{
	// the hardware location of this line
	std::pair<unsigned short, unsigned short> line;
	// the time to make the change
	double time;
	// the value to set it to. 
	bool value;
};

// an object constructed for having all info the ttls for a single time
struct DioSnapshot
{
	// the time of the snapshot
	double time;
	// all values at this time.
	std::array< std::array<bool, 16>, 4 > ttlStatus;
};

typedef struct _DIO64STAT 
{
	USHORT pktsize;
	USHORT portCount;
	USHORT writePtr;
	USHORT readPtr;
	USHORT time[2];
	ULONG fifoSize;
	USHORT fifo0;
	ULONG  ticks;
	USHORT flags;
	USHORT clkControl;
	USHORT startControl;
	USHORT stopControl;
	ULONG AIControl;
	USHORT AICurrent;
	USHORT startTime[2];
	USHORT stopTime[2];
	USHORT user[4];
} DIO64STAT;
