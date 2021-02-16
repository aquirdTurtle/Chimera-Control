#pragma once
#include <qstring.h>
#include <GeneralUtilityFunctions/my_str.h>

struct statusMsg {
	statusMsg() {}
	statusMsg(QString onlyMsg, unsigned baseLvl=0) {
		msg = onlyMsg;
		baseLevel = baseLvl;
	}
	statusMsg(std::string onlyMsg, unsigned baseLvl = 0) {
		msg = qstr(onlyMsg);
		baseLevel = baseLvl;
	}
	statusMsg(const char * onlyMsg, unsigned baseLvl = 0) {
		msg = qstr(onlyMsg);
		baseLevel = baseLvl;
	}
	QString msg="";
	unsigned baseLevel = 0;
	QString systemDelim = "";
};

Q_DECLARE_METATYPE(statusMsg)
