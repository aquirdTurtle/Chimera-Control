//Created by Max Kolanz
#pragma once
#include "Control.h"
#include "CustomMfcControlWrappers/myButton.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "DmCore.h"
#include "DmProfileCreator.h"

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
		DmControl(std::string serialNumber, bool safeMode);
		void initialize(POINT loc, CWnd* parent, int count, std::string serialNumber, LONG width, UINT &control_id);
	    void handleOnPress(int i);
		void ProgramNow();
		void setMirror(double *A);
		void loadProfile();
		void loadProfile(std::string filename);
		void updateButtons();
		int getActNum();

		HBRUSH handleColorMessage(CWnd* window, CDC* cDC);
		void handleSaveConfig(ConfigStream& newFile);
		void reColor(UINT id);
		void rearrange(int width, int height, fontMap fonts);
		bool isFloat(const std::string& someString);
		void add_Changes();
		std::vector<double> getTableValues();
		void writeCurrentFile(std::string out_file);
		void initializeTable(POINT& pos, int width, int height, CWnd* parent, UINT id);

		DmCore &getCore();
		DMOutputForm getExpressionValues();
		void openConfig();
		void setCoreInfo(DMOutputForm form);
		void refreshAbberationDisplays ();

	private:		
		DmInfo theDMInfo;
		DmCore defObject;
		DmProfileCreator Profile;
		DMOutputForm currentValues;
		std::vector<pistonButton> piston;
		Control<CEdit> comaMag;
		Control<CEdit> trefoilMag;
		Control<CEdit> astigMag;
		Control<CEdit> sphereMag;
		Control<CEdit> comaAngle;
		Control<CEdit> trefoilAngle;
		Control<CEdit> astigAngle;
		Control<CEdit> sphereAngle;
		Control<CleanPush> applyCorrections;
		Control<CStatic> angleLabel;
		Control<CStatic> magLabel;
		Control<CStatic> astigmatismLabel;
		Control<CStatic> sphericalLabel;
		Control<CStatic> trefoilLabel;
		Control<CStatic> comaLabel;
		Control<CleanPush> programNow;
		Control<CComboBox> profileSelector;
		std::vector<double> temp;
		std::vector<double> writeArray;
};
