#pragma once

enum class ExperimentType
{
	// normal experiments
	Normal,
	// simple load mot, F1
	LoadMot,
	// camera background calibration
	CameraCal,
	// 
	AutoCal,
	// Calibrations for determining Mot # & temperature
	/*MotSize,
	MotTemperature,
	PgcTemperature,
	GreyTemperature,*/

	// is part of machine optimization procedure
	MachineOptimization
};
