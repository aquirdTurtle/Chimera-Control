#pragma once
#include "afxwin.h"

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
