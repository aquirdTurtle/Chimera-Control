#pragma once
#include <qmap.h>
#include <qstring.h>

struct indvOption {
	bool show = true;
	int debugLvlOffset = 0;
};

struct StatusControlOptions {
	unsigned debugLvl=0;
	unsigned historyLength = 1000;
	bool showOrigin = false;
	QMap<QString, indvOption> indvOptions;
};
