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
class RohdeSchwarz
{
	public:
		RohdeSchwarz();
		void initialize( POINT& pos, cToolTips& toolTips, AuxiliaryWindow* parentWin, int& id );
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
		std::pair<DioRows::which, UINT> getRsgTriggerLine();
		void rearrange(UINT width, UINT height, fontMap fonts);
		UINT getNumTriggers (UINT );
	private:
		GpibFlume gpibFlume;
		std::vector<rsgEventForm> eventForms;
		// outer vector is over each variation.
		std::vector<std::vector<rsgEvent>> events;
		const double triggerTime = 0.01;
		const std::pair<DioRows::which, UINT> rsgTriggerLine = { DioRows::which::A, 15 };
		// 
		Control<CStatic> header;
		// this is a read-only control that shows the user how the RSG actually gets programmed in the end.
		Control<CListCtrl> infoControl;
};

