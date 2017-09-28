#pragma once
#include "Control.h"
#include <vector>
#include <string>
#include <unordered_map>
#include "GpibFlume.h"


struct rsgEventForm
{
	Expression frequency;
	Expression power;
	timeType time;
};


struct rsgEvent
{
	double frequency;
	double power;
	double time;
};


/**/
class RhodeSchwarz
{
	public:
		RhodeSchwarz::RhodeSchwarz();
		void initialize( POINT& pos, cToolTips& toolTips, AuxiliaryWindow* master, int& id );
		void programRSG(UINT variation );
		void addFrequency( rsgEventForm eventInfo );
		void clearFrequencies();
		std::vector<rsgEventForm> getFrequencyForms();
		void interpretKey(key variationKey, std::vector<variableType>& vars);	
		void orderEvents(UINT variation );
		void setInfoDisp(UINT variation );
		std::string getIdentity();
		std::string getRsgTtl();
		double getTriggerLength();
		void rearrange(UINT width, UINT height, fontMap fonts);
	private:
		GpibFlume gpibFlume;
		std::vector<rsgEventForm> eventStructures;
		// outer vector is over each variation.
		std::vector<std::vector<rsgEvent>> events;
		double triggerTime;
		std::string rsgTtl;
		// 
		Control<CStatic> header;
		// this is a read-only control that shows the user how the RSG actually gets programmed in the end.
		Control<CListCtrl> infoControl;
};