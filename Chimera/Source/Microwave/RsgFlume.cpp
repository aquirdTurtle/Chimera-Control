#include "stdafx.h"
#include "RsgFlume.h"

RsgFlume::RsgFlume (short deviceID, bool safemode) : GpibFlume::GpibFlume (deviceID, safemode)
{

}
std::string RsgFlume::queryIdentity ()
{
	return GpibFlume::queryIdentity ();
}

void RsgFlume::programList (std::vector<microwaveListEntry> list, unsigned varNum)
{
	send ("OUTP ON");
	send ("SOURce:LIST:SEL 'freqList" + str (list.size ()) + "'");
	std::string freqList = "SOURce:LIST:FREQ " + str (list[0].frequency.getValue (varNum), 13) + " GHz";
	std::string powerList = "SOURce:LIST:POW " + str (list[0].power.getValue (varNum), 13) + "dBm";
	for (unsigned eventInc = 1; eventInc < list.size (); eventInc++)
	{
		freqList += ", " + str (list[eventInc].frequency.getValue (varNum), 13) + " GHz";
		powerList += ", " + str (list[eventInc].power.getValue (varNum), 13) + "dBm";
	}
	send (cstr (freqList));
	send (cstr (powerList));
	send ("SOURce:LIST:MODE STEP");
	send ("SOURce:LIST:TRIG:SOURce EXT");
	send ("SOURce:FREQ:MODE LIST");
}

void RsgFlume::programSingleSetting (microwaveListEntry setting, unsigned varNumber)
{
	send ("OUTPUT OFF");
	send ("SOURce:FREQuency:MODE CW");
	send ("FREQ " + str (setting.frequency.getValue (varNumber), 13) + " GHz");
	send ("POW " + str (setting.power.getValue (varNumber), 13) + " dBm");
	send ("OUTPUT ON");
}

void RsgFlume::setPmSettings ()
{
	send ("SOURCE:FM1:STATe OFF");
	send ("SOURCE:PM:MODE HDEViation");
	send ("SOURCE:PM:RATio 100PCT");
	send ("SOURCE:PM1:SOURce EXT1");
	send ("SOURCE:PM1:DEViation 6.28RAD");
	send ("SOURCE:PM1:STATe ON");
}

void RsgFlume::setFmSettings ()
{
	send ("SOURCE:PM1:STATe OFF");
	send ("SOURCE:FM:MODE NORMal");
	send ("SOURCE:FM:RATio 100PCT");
	send ("SOURCE:FM1:SOURce EXT1");
	send ("SOURCE:FM1:DEViation 20kHz");
	send ("SOURCE:FM1:STATe ON");
}