#include "stdafx.h"
#include <boost/lexical_cast.hpp>
#include "WindFreakFlume.h"
#include <boost/algorithm/string.hpp>
#include <algorithm>

WindFreakFlume::WindFreakFlume (std::string portAddress, bool safemode)
	: WinSerialFlume::WinSerialFlume(safemode, portAddress)
{}

std::string WindFreakFlume::queryIdentity (){
	auto model = query ("+");
	return "WindFreak Model/SN: " + model;
}

void WindFreakFlume::setPmSettings (){
	write ("C0");
	// todo. I think modulation like this is possible though.
}

void WindFreakFlume::setFmSettings (){
	write ("C0");
	// todo
}

void WindFreakFlume::programSingleSetting (microwaveListEntry setting, unsigned varNumber){
	std::string answer;
	answer = query ("C0");
	answer = query ("E1");
	answer = query ("Ld");
	answer = query ("w0");
	answer = query ("f" + str(setting.frequency.getValue (varNumber)*1e3));
	answer = query ("W" + str (setting.power.getValue (varNumber), 5));
}

void WindFreakFlume::programList (std::vector<microwaveListEntry> list, unsigned varNum, double triggerTime){
	resetConnection ();
	// Settings for RfoutA
	std::string answer;
	answer = query ("C0");
	unsigned count = 0;
	// delete prev list
	answer = query ("Ld");
	for (auto entry : list)	{
		auto ln = "L" + str (count);
		answer = query (ln + "f" + str (entry.frequency.getValue (varNum)*1e3, 7, false, false, true));
		answer = query (ln + "a" + str (entry.power.getValue (varNum),3,false,false,true));
		count++;
	}
	// lock the pll (probably already locked but ok)
	answer = query ("E1");
	// set trigger mode
	answer = query ("w2");
	// set sweep mode to "list"
	answer = query ("X1");
	// set trigger time
	answer = query ("t" + str(triggerTime, 3));
	// sweep continuously
	answer = query ("c0");
}

std::string WindFreakFlume::getListString () {
	std::string answer = "";
	unsigned count = 0;
	auto misc_msg2 = read ();
	while (true) {
		auto res = query ("L" + str (count) + "f?");
		try {
			res.erase (std::remove (res.begin (), res.end (), '\n'), res.end ());
			auto freq = boost::lexical_cast<double>(res);
			if (freq == 0) {
				break;
			}
		}
		catch (boost::bad_lexical_cast&) {
			throwNested ("Bad Frequency Value From Windfreak??? String was: \"" + res + "\"");
		}
		answer += str (count + 1) + ". ";
		answer += res + ", ";
		res = query ("L" + str (count) + "a?");
		res.erase (std::remove (res.begin (), res.end (), '\n'), res.end ());
		answer += res + "; ";
		count++;
		if (count > 100) {
			break;
		}
	}
	return answer;
}
