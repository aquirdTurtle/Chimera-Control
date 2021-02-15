#pragma once
#include <qstring.h>
#include <GeneralUtilityFunctions/my_str.h>

struct statusMsg {
	statusMsg() {}
	statusMsg(QString onlyMsg, unsigned baseLvl=0) {
		msg = onlyMsg;
	}
	statusMsg(std::string onlyMsg, unsigned baseLvl = 0) {
		msg = qstr(onlyMsg);
	}
	statusMsg(const char * onlyMsg, unsigned baseLvl = 0) {
		msg = qstr(onlyMsg);
	}
	QString msg="";
	unsigned baseLevel = 0;
	QString systemDelim = "";
};

