//Created by Max Kolanz
#pragma once
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "DmCore.h"
#include "DmProfileCreator.h"
#include <qcombobox.h>
#include "PrimaryWindows/IChimeraQtWindow.h"
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>

struct DmInfo
{
	// add any other settings for the whole machine here. 
	std::string serialNumber;
	int ActuatorCount;
};

struct pistonInfo {
	int Value;
};

class DmControl
{
	public:
		DmControl(std::string serialNumber, bool safeMode);
		void initialize( POINT loc, IChimeraQtWindow* parent, int count, std::string serialNumber, LONG width );
	    void handleOnPress(int i);
		void ProgramNow();
		void setMirror(double *A);
		void loadProfile();
		void loadProfile(std::string filename);
		void updateButtons();
		int getActNum();

		void handleSaveConfig(ConfigStream& newFile);
		bool isFloat(const std::string& someString);
		void add_Changes();
		std::vector<double> getTableValues();
		void writeCurrentFile(std::string out_file);
		void initializeTable(POINT& pos, int width, int height, IChimeraQtWindow* parent);

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
		std::vector<QLineEdit*> piston;
		QLineEdit* comaMag;
		QLineEdit* trefoilMag;
		QLineEdit* astigMag;
		QLineEdit* sphereMag;
		QLineEdit* comaAngle;
		QLineEdit* trefoilAngle;
		QLineEdit* astigAngle;
		QLineEdit* sphereAngle;
		QPushButton* applyCorrections;
		QLabel* angleLabel;
		QLabel* magLabel;
		QLabel* astigmatismLabel;
		QLabel* sphericalLabel;
		QLabel* trefoilLabel;
		QLabel* comaLabel;
		QPushButton* programNow;
		QComboBox* profileSelector;
		std::vector<double> temp;
		std::vector<double> writeArray;
};
