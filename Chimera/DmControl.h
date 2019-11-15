#pragma once
#include "Control.h"
#include "myButton.h"
#include "VisaFlume.h"
#include "Version.h"
#include "DmCore.h"

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
	DmControl::DmControl(std::string serialNumber, bool safeMode);
		void initialize(POINT loc, CWnd* parent, int count, std::string serialNumber, LONG width, UINT &control_id);
	    void handleOnPress(int i);
		void ProgramNow();
		void setMirror(double *A);
		void loadProfile();
		void updateButtons();
		int getActNum();
		HBRUSH handleColorMessage(CWnd* window, CDC* cDC);
		void reColor(UINT id);
		void rearrange(int width, int height, fontMap fonts);
		bool isFloat(const std::string& someString);

	private:
		
		DmInfo currentInfo;
		DmCore defObject;
		std::vector<pistonButton> piston;
		Control<CButton> programNow;
		Control<CComboBox> profileSelector;
		std::vector<double> temp;
};
