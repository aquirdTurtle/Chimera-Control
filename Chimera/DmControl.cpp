#include "stdafx.h"
#include "Thrower.h"
#include "ProfileSystem.h"
#include "range.h"
#include "DmControl.h"
#include "string.h"
#include <boost/lexical_cast.hpp>
#include <fstream>
#include "cameraPositions.h"
#include "ProfileSystem.h"


DmControl::DmControl(std::string serialNumber, bool safeMode) : defObject(serialNumber, safeMode){
	defObject.initialize();
	temp = std::vector<double>(137);
}

void DmControl::initialize(POINT loc, CWnd* parent, int count, std::string serialNumber, LONG width, UINT &control_id) {
	currentInfo.ActuatorCount = count;
	currentInfo.serialNumber = serialNumber;
	piston = std::vector<pistonButton>(count);
	POINT B[137];
	POINT A[13] = { {loc}, {loc.x - (2 * width), loc.y + width }, {loc.x - (3 * width), loc.y + 2 * width},
				{loc.x - (3 * width), loc.y + 3 * width}, {loc.x - (4 * width), loc.y + 4 * width},
				{loc.x - (4 * width), loc.y + 5 * width}, {loc.x - (4 * width), loc.y + 6 * width},
				{loc.x - (4 * width), loc.y + 7 * width} , {loc.x - (4 * width), loc.y + 8 * width},
				{loc.x - (3 * width), loc.y + 9 * width} , {loc.x - (3 * width), loc.y + 10 * width},
				{loc.x - (2 * width), loc.y + 11 * width}, {loc.x, loc.y + 12 * width } };
	int collumns[13] = { 5, 9, 11, 11, 13, 13, 13, 13, 13, 11, 11, 9, 5 };
	int w = 0;
	for (int y = 0; y < 13; y++) {
		for (int z = 0; z < collumns[y]; z++) {
			B[w] = { A[y].x, A[y].y };
			A[y].x += width;
			w++;
		}
	}
	if (count != 137) {
		thrower("GUI Only Works with acutator count of 137");
	}
	else {
		//POINT init = A[0];
		int i;
			for (i = 0; i < 137; i++) {
				piston[i].Voltage.sPos = { B[i].x, B[i].y, B[i].x + width, B[i].y + width };
				piston[i].Voltage.Create(NORM_EDIT_OPTIONS | WS_BORDER | ES_CENTER | ES_WANTRETURN
										 | ES_MULTILINE, piston[i].Voltage.sPos, parent, IDC_DM_EDIT_START + i);//control_id++);
			}
	}
	programNow.sPos = { 40, 50, 240, 100 };
	programNow.Create("Program Now", NORM_PUSH_OPTIONS, programNow.sPos, parent, IDC_DM_PROGRAMNOW);
	/*std::vector<double> initial = std::vector<double>(137, 0.0);
	setMirror(initial.data());
	updateButtons();*/
	//loadProfile("flatProfile.txt");
	cameraPositions positions;
	positions.sPos = { 40, 200 };
	positions.videoPos = positions.amPos = positions.seriesPos = positions.sPos;
	profileSelector.setPositions(positions, 0, 0, 100, 500);
	profileSelector.Create(NORM_COMBO_OPTIONS, profileSelector.seriesPos, parent, IDC_DM_PROFILE_COMBO);
	ProfileSystem::reloadCombo(profileSelector.GetSafeHwnd(), DM_PROFILES_LOCATION, str("*") + "txt", "flatProfile");
}

void DmControl::handleOnPress(int i) {

	piston[i].Voltage.EnableWindow();
}

void DmControl::updateButtons() {
	defObject.readDMArray(temp);
	double value;
	for (int i = 0; i < 137; i++) {
	
		value = temp[i];
		double rounded = (int)(value * 100000.0) / 100000.0;
		std::string s1 = boost::lexical_cast<std::string>(rounded);
		piston[i].Voltage.SetWindowTextA(cstr(s1));
	}
}

void DmControl::ProgramNow() {
	std::vector<double> values;
	for (int i = 0; i < 137; i++) {
		CString s1;
		piston[i].Voltage.GetWindowTextA(s1);
		std::string s2 = str(s1, 4, false, true);
		try{
			double s3 = boost::lexical_cast<double>(s2);
			if (s3 < 0 || s3 > 1) {
				thrower("cannot enter values greater than 1 or less than 0");
				values.push_back(0.0);
			}
			else {
				values.push_back(s3);
			}
		}
		catch(Error&){
			values.push_back(0.0);
		}
		
	}
	defObject.loadArray(values.data());
	updateButtons();
}

void DmControl::setMirror(double *A) {
	defObject.loadArray(A);
	updateButtons(); //pass an array and program it

}

int DmControl::getActNum() {
	return defObject.getActCount();
}

bool DmControl::isFloat(const std::string& someString)//put in class
{

	try
	{
		boost::lexical_cast<float>(someString);
	}
	catch (boost::bad_lexical_cast&)
	{
		return false;
	}

	return true;
}

HBRUSH DmControl::handleColorMessage(CWnd* window, CDC* cDC)
{
	DWORD controlID = window->GetDlgCtrlID();
	if (controlID == programNow.GetDlgCtrlID()) {
		cDC->SetBkColor(_myRGBs["Slate Grey"]);
		cDC->SetTextColor(_myRGBs["Text"]);
		return *_myBrushes["Static-Bkgd"];
	}
	if (controlID == profileSelector.GetDlgCtrlID()) {
		cDC->SetBkColor(_myRGBs["Slate Grey"]);
		cDC->SetTextColor(_myRGBs["Text"]);
		return *_myBrushes["Static-Bkgd"];
	}
	for (auto& pis : piston) {
		if (controlID == pis.Voltage.GetDlgCtrlID()) {
			CString s1;
			double s3 = 0;
			pis.Voltage.GetWindowTextA(s1);
			std::string s2 = str(s1, 4, false, true);
			try {
				if (isFloat(s2) ){
					s2.erase(std::remove(s2.begin(), s2.end(), '\n'), s2.end());
					s3 = boost::lexical_cast<double>(s2);
						if (s3 < 0 || s3 > 1) {
							return *_myBrushes["DmColor1"];
						}
				}
				
			}
			catch (Error&) {
				return *_myBrushes["DmColor1"];
			}
			int value = int(254 * s3) + 1;
			std::string name = "DmColor" + str(value);
			cDC->SetBkColor(_myRGBs[name]);
			if (s3 >= 0.8) {
				cDC->SetTextColor(_myRGBs["Black"]);
			}
			else {
				cDC->SetTextColor(_myRGBs["Text"]);
			}
			//updateButton(controlID - IDC_DM_PROGRAMNOW, s3);
			return *_myBrushes[name];
		}
	}
	
	return NULL;
}

void DmControl::reColor(UINT id) {
	for (auto& pis : piston) {
		if (id == pis.Voltage.GetDlgCtrlID()) {
			//.Voltage.SetRedraw();
			pis.Voltage.RedrawWindow();
		}
	}
}

void DmControl::rearrange(int width, int height, fontMap fonts)
{
	programNow.rearrange(width, height, fonts);
	for (auto& pis : piston) {
		pis.Voltage.rearrange(width, height, fonts);
	}
	
}

void DmControl::loadProfile()
{
	CString file;
	int id = profileSelector.GetCurSel();
	profileSelector.GetLBText(id,file);
	std::string filename = str(file);
	if (DM_SAFEMODE) {
		std::ifstream file(DM_PROFILES_LOCATION + "\\" + filename + ".txt");
		if (!file.is_open()) {
			thrower("File did not open");
		}
		std::string value;
		double temp;
		for (auto& pis : piston) {
			std::getline(file, value);
			try {
				temp = boost::lexical_cast<double>(value);
			}
			catch (boost::bad_lexical_cast) {
				temp = 0.0;
			}
			double rounded = (int)( temp * 100000.0) / 100000.0;
			std::string s1 = boost::lexical_cast<std::string>(rounded);
			pis.Voltage.SetWindowTextA(cstr(s1));
		}
	}
	else {
		std::ifstream file(DM_PROFILES_LOCATION + "\\" + filename);
		std::string value;
		int length = temp.size();
		double voltage;
		for (int counter = 0; counter < length; counter++) {
			std::getline(file, value);
			try {
				voltage = boost::lexical_cast<double>(value);
			}
			catch (boost::bad_lexical_cast) {
				voltage = 0.0;
			}
			temp[counter] = voltage;
		}
		setMirror(temp.data());
	}
}

