#pragma once
#include "Control.h"
#include "myButton.h"
#include "VisaFlume.h"
#include "Version.h"

struct DmInfo
{
	// add any other settings for the whole machine here. 
	std::string serialNumber;
	int ActuatorCount;
};

struct pistonInfo {
	int Value;
};

struct pistonButton {
		
		Control<CEdit> Voltage;
};

class DmControl
{
	public:
		void initialize(POINT loc, CWnd* parent, int count, std::string serialNumber, LONG width, UINT control_id);
	    void handleOnPress(int i);
	private:
		
		DmInfo currentInfo;
		std::vector<pistonButton> piston;
};
