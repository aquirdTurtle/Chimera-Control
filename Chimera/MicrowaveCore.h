#pragma once
#include "GpibFlume.h"

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

class MicrowaveCore
{
	public:
		MicrowaveCore ();
		std::string queryIdentity ();
		void orderEvents (UINT variation);
		void setFmSettings ();
		void setPmSettings ();
		void programRsg (UINT variationNumber);
		void clearFrequencies ();
		std::vector<rsgEventForm> getFrequencyForms ();
		void interpretKey (std::vector<std::vector<parameterType>>& params);
		void addFrequency (rsgEventForm info);
		std::pair<DioRows::which, UINT> getRsgTriggerLine ();
		UINT getNumTriggers (UINT variationNumber);
	private:
		const double triggerTime = 0.01;
		const std::pair<DioRows::which, UINT> rsgTriggerLine = { DioRows::which::C, 3 };
		GpibFlume gpibFlume;
		std::vector<rsgEventForm> eventForms;
		std::vector<std::vector<rsgEvent>> events;

};