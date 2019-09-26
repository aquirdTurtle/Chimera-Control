// created by Mark O. Brown
#pragma once

#include "GpibFlume.h"
#include "Control.h"
#include <vector>
#include <string>
#include <unordered_map>

class AuxiliaryWindow;

struct rsgEventForm
{
	Expression frequency;
	Expression power;
	// I think that the time here is no longer very important at all. Only used to order the events in case the user
	// writes a weird script, normal scripts the user would have them ordered properly anyways.
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
		RhodeSchwarz();
		void initialize( POINT& pos, cToolTips& toolTips, AuxiliaryWindow* master, int& id );
		void programRsg( UINT variation );
		void addFrequency( rsgEventForm eventInfo );
		void clearFrequencies();
		std::vector<rsgEventForm> getFrequencyForms();
		void interpretKey( std::vector<std::vector<parameterType>>& variables);
		void orderEvents( UINT variation );
		void setInfoDisp( UINT variation );
		void setFmSettings ( );
		void setPmSettings ( );
		std::string getIdentity();
		std::string getRsgTtl();
		double getTriggerLength();
		void rearrange( UINT width, UINT height, fontMap fonts);
	private:
		GpibFlume gpibFlume;
		std::vector<rsgEventForm> eventForms;
		// outer vector is over each variation.
		std::vector<std::vector<rsgEvent>> events;
		double triggerTime;
		std::string rsgTtl;
		// 
		Control<CStatic> header;
		// this is a read-only control that shows the user how the RSG actually gets programmed in the end.
		Control<CListCtrl> infoControl;
};