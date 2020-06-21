#include "stdafx.h"
#include "WindFreakFlume.h"

WindFreakFlume::WindFreakFlume (std::string portAddress, bool safemode)
	: WinSerialFlume::WinSerialFlume(safemode, portAddress)
{}

std::string WindFreakFlume::queryIdentity (){
	auto model = query ("+");
	return "WindFreak Model/SN: " + model;
}

void WindFreakFlume::setPmSettings (){
	write ("C0");
}

void WindFreakFlume::setFmSettings (){
	write ("C0");
}

void WindFreakFlume::programSingleSetting (microwaveListEntry setting, UINT varNumber){
	write ("C0");
	// lock the pll
	write ("E1");
	write ("Ld");
	write ("w0");
	write ("f" + str(setting.frequency.getValue (varNumber)*1e3));
	write ("W" + str (setting.power.getValue (varNumber), 5));
}

void WindFreakFlume::programList (std::vector<microwaveListEntry> list, UINT varNum){
	// Settings for RfoutA
	write ("C0");
	// lock the pll
	write ("E1");
	// set trigger mode
	write ("w2");
	// set sweep mode to "list"
	write ("X1");
	// set trigger time
	write ("t2");
	// delete prev list
	write ("Ld");
	// sweep continuously
	write ("c0");
	UINT count = 0;
	for (auto entry : list)
	{
		auto ln = "L" + str (count);
		write (ln + "f" + str (entry.frequency.getValue (varNum)*1e3, 13, false, false, true));
		write (ln + "a" + str (entry.power.getValue (varNum),13,false,false,true));
		count++;
	}
	resetConnection ();
	// start sweep
	//writebtn ("g1");
}

