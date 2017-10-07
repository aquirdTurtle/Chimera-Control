#pragma once

struct rearrangeParams
{
	bool active = false;
	double moveSpeed = 60e-6;
	double flashingRate = 1e6;
	double moveBias=1;
	// 
	double deadTime=0;
	// the static / moving ratio
	double staticMovingRatio=1;
	bool outputInfo = false;
};
