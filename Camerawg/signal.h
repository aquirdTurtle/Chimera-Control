#pragma once
#include "stdafx.h"

#include <string>

/**
 * A "Signal" structure contains all of the information for a single signal. Vectors of these are included in a "waveInfo" structure.
 */
struct signal
{
	double freqInit;
	double freqFin;
	std::string freqRampType;

	double initPower;
	double finPower;
	std::string powerRampType;

	double initPhase;
	// Asssigned only after a waveform is calculated or read.
	double finPhase;
	// 0 means normal. Get set to -1 if grabbing previous phase.
	int phaseOption = 0;
};
