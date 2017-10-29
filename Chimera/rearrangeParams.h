#pragma once

struct rearrangeParams
{
	bool active = false;
	double moveSpeed = 60e-6;
	double flashingRate = 1e6;
	double moveBias=1;
	// 
	double deadTime=0;
	// the static / moving time ratio, i.e. if 2 this might imply out of a total 60us move, the static part takes up
	// 40us and the moving part takes up 20us.
	double staticMovingRatio=1;
	bool outputInfo = false;
	bool outputIndv = false;
	bool preprogram = false;
	// not implemented yet.
	bool useCalibration = false;
};
