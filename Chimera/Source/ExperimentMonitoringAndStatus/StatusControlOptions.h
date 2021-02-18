#pragma once
#include <qmap.h>
#include <qstring.h>

struct StatusControlOptions {
	unsigned debugLvl=0;
	unsigned historyLength = 10000;
	bool showOrigin = false;
	QMap<QString, int> indvOptions;
};
