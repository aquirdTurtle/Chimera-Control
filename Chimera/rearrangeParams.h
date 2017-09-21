#pragma once

struct rearrangeParams
{
	bool active;
	double moveSpeed;
	double flashingRate;
	double moveBias;
	// 
	double dutyCycle;
	// fraction of time during which the moving tweezer is active vs. the static tweezers.
	double movingDutyCycle;
};
