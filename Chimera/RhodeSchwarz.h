#pragma once
#include "Control.h"
#include <vector>
#include <string>
#include <unordered_map>
#include "GPIB.h"


struct rsgEventStructuralInfo
{
	std::string frequency;
	std::string power;
	timeType time;
};


struct rsgEventInfoFinal
{
	double frequency;
	double power;
	double time;
};

/**/
class RhodeSchwarz
{
	public:
		void initialize( POINT& pos, cToolTips& toolTips, DeviceWindow* master, int& id );
		void programRSG(Gpib* gpib, UINT var);
		void addFrequency( rsgEventStructuralInfo eventInfo );
		void clearFrequencies();
		std::vector<rsgEventStructuralInfo> getFrequencyForms();
		void interpretKey(key variationKey, std::vector<variable>& vars);	
		void orderEvents(UINT var);
		void setInfoDisp(UINT var);
		std::string getRsgTtl();
		double getTriggerTime();
		void rearrange(UINT width, UINT height, fontMap fonts);
	private:
		std::vector<rsgEventStructuralInfo> eventStructures;
		// outer vector is over each variation.
		std::vector<std::vector<rsgEventInfoFinal>> events;
		double triggerTime;
		std::string rsgTtl;
		//
		Control<CStatic> header;
		// this is a read-only control that shows the user how the RSG actually gets programmed in the end.
		Control<CListCtrl> infoControl;
};