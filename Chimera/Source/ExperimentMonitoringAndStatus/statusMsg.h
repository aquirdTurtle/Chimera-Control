#pragma once
#include <qstring.h>
#include <GeneralUtilityFunctions/my_str.h>

struct statusMsg {
	statusMsg() {}
	statusMsg(QString onlyMsg, unsigned baseLvl=0, QString sysDelim="") {
		msg = onlyMsg;
		baseLevel = baseLvl;
		systemDelim = sysDelim;
	}
	statusMsg(std::string onlyMsg, unsigned baseLvl = 0, std::string sysDelim="") {
		msg = qstr(onlyMsg);
		baseLevel = baseLvl;
		systemDelim = qstr(sysDelim);
	}
	statusMsg(const char * onlyMsg, unsigned baseLvl = 0, const char * sysDelim="") {
		msg = qstr(onlyMsg);
		baseLevel = baseLvl;
		systemDelim = sysDelim;
	}
	QString msg="";
	unsigned baseLevel = 0;
	QString systemDelim = "";
};

Q_DECLARE_METATYPE(statusMsg)
