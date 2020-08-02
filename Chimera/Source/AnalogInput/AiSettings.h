#pragma once

struct AiSettings
{
	bool queryContinuously=false;
	int continuousModeInterval=1000;
	bool queryBtwnVariations=false;
	unsigned int numberMeasurementsToAverage=100;
};
