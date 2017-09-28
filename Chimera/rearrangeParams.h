#pragma once

struct rearrangeParams
{
	bool active = false;
	double moveSpeed = 1e-6;
	double flashingRate;
	double moveBias;
	// 
	double deadTime;
	// the static / moving ratio
	double staticMovingRatio;
};
