//Created by Max Kolanz
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


DmControl::DmControl(std::string serialNumber, bool safeMode) : defObject(serialNumber, safeMode),
Profile() {
	defObject.initialize();
	temp = std::vector<double>(137);
	std::string location = DM_PROFILES_LOCATION + "\\" + "25CW012#060_CLOSED_LOOP_COMMANDS.txt";
	Profile.createZernikeArray(Profile.getCurrAmps(), location, false);
	Profile.readZernikeFile(location);
}

void DmControl::initializeTable(int xPos, int yPos, int width, int height, CWnd* parent, UINT id) {

	//xPos = (3840 * xPos)/1920;

	int xPos2 = xPos + width + 5;
	int xPos3 = xPos2 + width + 5;
	int yPos2 = yPos + height + 5;
	int yPos3 = yPos2 + height + 5;
	int yPos4 = yPos3 + height + 5;
	int yPos5 = yPos4 + height + 5;

	Mag.sPos = { xPos, yPos, xPos+width, yPos+height };
	Mag.Create(cstr("Magnitude"), NORM_STATIC_OPTIONS, Mag.sPos, parent, id + 1);
	Angle.sPos = { xPos2, yPos, xPos2 + width, yPos + height};
	Angle.Create(cstr("Angle"), NORM_STATIC_OPTIONS, Angle.sPos, parent, id + 2);

	Coma.sPos = { xPos3, yPos2, xPos3+90, yPos2 + height };
	Coma.Create(cstr("Coma"), NORM_STATIC_OPTIONS, Coma.sPos, parent, id + 3);
	Astigmatism.sPos = { xPos3, yPos3, xPos3 + 90, yPos3 + height };
	Astigmatism.Create(cstr("Astigmatism"), NORM_STATIC_OPTIONS, Astigmatism.sPos, parent, id + 4);
	Trefoil.sPos = { xPos3, yPos4, xPos3 + 90, yPos4 + height };
	Trefoil.Create(cstr("Trefoil"), NORM_STATIC_OPTIONS, Trefoil.sPos, parent, id + 5);
	Spherical.sPos = { xPos3, yPos5, xPos3 + 90, yPos5 + height };
	Spherical.Create(cstr("Spherical"), NORM_STATIC_OPTIONS, Spherical.sPos, parent, id + 6);

	comaMag.sPos = { xPos, yPos2, xPos + width, yPos2 + height };
	comaMag.Create(NORM_EDIT_OPTIONS | WS_BORDER | ES_CENTER | ES_WANTRETURN
				   | ES_MULTILINE, comaMag.sPos, parent, id+7);
	comaAngle.sPos = { xPos2, yPos2, xPos2+width, yPos2 + height};
	comaAngle.Create(NORM_EDIT_OPTIONS | WS_BORDER | ES_CENTER | ES_WANTRETURN
					 | ES_MULTILINE, comaAngle.sPos, parent, id+8);
	astigMag.sPos = { xPos, yPos3, xPos + width, yPos3 + height };
	astigMag.Create(NORM_EDIT_OPTIONS | WS_BORDER | ES_CENTER | ES_WANTRETURN
					| ES_MULTILINE, astigMag.sPos, parent, id + 9);
	astigAngle.sPos = { xPos2, yPos3, xPos2 + width, yPos3 + height };
	astigAngle.Create(NORM_EDIT_OPTIONS | WS_BORDER | ES_CENTER | ES_WANTRETURN
					  | ES_MULTILINE, astigAngle.sPos, parent, id + 10);
	trefoilMag.sPos = { xPos, yPos4, xPos + width, yPos4+height };
	trefoilMag.Create(NORM_EDIT_OPTIONS | WS_BORDER | ES_CENTER | ES_WANTRETURN
					  | ES_MULTILINE, trefoilMag.sPos, parent, id + 11);
	trefoilAngle.sPos = { xPos2, yPos4, xPos2 +width, yPos4 + height };
	trefoilAngle.Create(NORM_EDIT_OPTIONS | WS_BORDER | ES_CENTER | ES_WANTRETURN
						| ES_MULTILINE, trefoilAngle.sPos, parent, id + 12);
	sphereMag.sPos = { xPos, yPos5 , xPos + width, yPos5 + height };
	sphereMag.Create(NORM_EDIT_OPTIONS | WS_BORDER | ES_CENTER | ES_WANTRETURN
					 | ES_MULTILINE, sphereMag.sPos, parent, id + 13);
	sphereAngle.sPos = {xPos2, yPos5 , xPos2 + width, yPos5 + height };
	sphereAngle.Create(NORM_EDIT_OPTIONS | WS_BORDER | ES_CENTER | ES_WANTRETURN
					   | ES_MULTILINE, sphereAngle.sPos, parent, id + 14);

	applyCorrections.sPos = { xPos3, yPos5 + height + 5, xPos3 + 140, yPos5 + (2 * height) + 5 };
	applyCorrections.Create("Apply Corrections", NORM_PUSH_OPTIONS, applyCorrections.sPos, parent, id + 15);
}

void DmControl::initialize(POINT loc, CWnd* parent, int count, std::string serialNumber, LONG width, UINT &control_id) {
	theDMInfo.ActuatorCount = count;
	theDMInfo.serialNumber = serialNumber;
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
	loadProfile("flatProfile");
	cameraPositions positions;
	positions.sPos = { 40, 200 };
	positions.videoPos = positions.amPos = positions.seriesPos = positions.sPos;
	profileSelector.setPositions(positions, 0, 0, 100, 500);
	profileSelector.Create(NORM_COMBO_OPTIONS, profileSelector.seriesPos, parent, IDC_DM_PROFILE_COMBO);
	ProfileSystem::reloadCombo(profileSelector.GetSafeHwnd(), DM_PROFILES_LOCATION, str("*") + "txt", "flatProfile");
	
	initializeTable(1450, 100, 50, 25, parent, IDC_DM_ADD_ZERNIKE);
	
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
	comaMag.rearrange(width, height, fonts);
	trefoilMag.rearrange(width, height, fonts);;
	astigMag.rearrange(width, height, fonts);
	sphereMag.rearrange(width, height, fonts);
	comaAngle.rearrange(width, height, fonts);
	trefoilAngle.rearrange(width, height, fonts);
	astigAngle.rearrange(width, height, fonts);
	sphereAngle.rearrange(width, height, fonts);
	applyCorrections.rearrange(width, height, fonts);
	Angle.rearrange(width, height, fonts);
	Mag.rearrange(width, height, fonts);
	Astigmatism.rearrange(width, height, fonts);
	Spherical.rearrange(width, height, fonts);
	Trefoil.rearrange(width, height, fonts);
	Coma.rearrange(width, height, fonts);
	programNow.rearrange(width, height, fonts);
	profileSelector.rearrange(width, height, fonts);
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
	//if (DM_SAFEMODE) {
	std::ifstream in_file(DM_PROFILES_LOCATION + "\\" + filename + ".txt");
	if (!in_file.is_open()) {
		thrower("File did not open");
	}
	std::string value;
	double voltage;
	int count = 0;
	for (auto& pis : piston) {
		std::getline(in_file, value);
		try {
			voltage = boost::lexical_cast<double>(value);
		}
		catch (boost::bad_lexical_cast) {
			voltage = 0.0;
		}
		double rounded = (int)(voltage * 100000.0) / 100000.0;
		std::string s1 = boost::lexical_cast<std::string>(rounded);
		pis.Voltage.SetWindowTextA(cstr(s1));
		temp[count] = voltage;
		reColor(IDC_DM_PROFILE_COMBO + count);
		count++;

	}
	setMirror(temp.data()); 
	
}

void DmControl::loadProfile(std:: string filename)
{

		std::ifstream file(DM_PROFILES_LOCATION + "\\" + filename + ".txt");
		if (!file.is_open()) {
			thrower("File did not open");
		}
		std::string value;
		double voltage;
		int count = 0;
		for (auto& pis : piston) {
			std::getline(file, value);
			try {
				voltage = boost::lexical_cast<double>(value);
			}
			catch (boost::bad_lexical_cast) {
				voltage = 0.0;
			}
			double rounded = (int)(voltage * 100000.0) / 100000.0;
			std::string s1 = boost::lexical_cast<std::string>(rounded);
			pis.Voltage.SetWindowTextA(cstr(s1));
			temp[count] = voltage;
			reColor(IDC_DM_PROFILE_COMBO + count);
			count++;
			
		}	
		setMirror(temp.data());
}

void DmControl::writeCurrentFile(std::string out_file) {
	std::ofstream outWrite(DM_PROFILES_LOCATION + "//" + out_file);
	for (auto& element : writeArray) {
		outWrite << element << std::endl;
	}
}

std::vector<double> DmControl::getTableValues() {
	CString s[8];
	comaMag.GetWindowTextA(s[0]);
	comaAngle.GetWindowTextA(s[1]);
	astigMag.GetWindowTextA(s[2]);
	astigAngle.GetWindowTextA(s[3]);
	trefoilMag.GetWindowTextA(s[4]);
	trefoilAngle.GetWindowTextA(s[5]);
	sphereMag.GetWindowTextA(s[6]);
	sphereAngle.GetWindowTextA(s[7]);
	std::string s2;
	std::vector<double> parameters(8);
	for (int i = 0; i < 8; i++) {
		s2 = str(s[i], 4, false, true);
		if (s2.length() == 0) {
			s2 = "0.0";
		}
		try {
			parameters[i] = boost::lexical_cast<double>(s2);
		}
		catch (boost::bad_lexical_cast&) {
			parameters[i] = 0.0;
			thrower("Error:  bad lexical cast");
		}
	}
	return parameters;
}

void DmControl::add_Changes() {
	std::vector<double> params = getTableValues();
	Profile.addComa(params[0], params[1]);
	Profile.addAstigmatism(params[2], params[3]);
	Profile.addTrefoil(params[4], params[5]);
	Profile.addSpherical(params[6]);
	CString file;
	int id = profileSelector.GetCurSel();
	profileSelector.GetLBText(id, file);
	std::string filename = str(file);
	std::string location = DM_PROFILES_LOCATION + "\\" + filename + ".txt";
	writeArray = Profile.createZernikeArray(Profile.getCurrAmps(), location, false);
	writeCurrentFile("currentLoadOut.txt");
	loadProfile("currentLoadOut");
}

DmCore &DmControl::getCore() {
	return defObject;
}

DMOutputForm DmControl::getExpressionValues() {
	DMOutputForm output;
	CString s[8];
	comaMag.GetWindowTextA(s[0]);
	comaAngle.GetWindowTextA(s[1]);
	astigMag.GetWindowTextA(s[2]);
	astigAngle.GetWindowTextA(s[3]);
	trefoilMag.GetWindowTextA(s[4]);
	trefoilAngle.GetWindowTextA(s[5]);
	sphereMag.GetWindowTextA(s[6]);
	profileSelector.GetWindowTextA(s[7]);

	output.coma = str(s[0]);
	output.comaAng = str(s[1]);
	output.astig= str(s[2]);
	output.astigAng = str(s[3]);
	output.trefoil = str(s[4]);
	output.trefoilAng = str(s[5]);
	output.spherical = str(s[6]);
	output.base = str(s[7]);
	return output;
}

void DmControl::handleSaveConfig(std::ofstream& newFile) {
	defObject.handleSaveConfig(newFile, getExpressionValues());
}

void DmControl::openConfig() {
	DMOutputForm Form = defObject.getInfo();
	comaMag.SetWindowTextA(cstr(Form.coma.expressionStr));
	comaAngle.SetWindowTextA(cstr(Form.comaAng.expressionStr));
	astigMag.SetWindowTextA(cstr(Form.astig.expressionStr));
	astigAngle.SetWindowTextA(cstr(Form.astigAng.expressionStr));
	trefoilMag.SetWindowTextA(cstr(Form.trefoil.expressionStr));
	trefoilAngle.SetWindowTextA(cstr(Form.trefoilAng.expressionStr));
	sphereMag.SetWindowTextA(cstr(Form.spherical.expressionStr));
	ProfileSystem::reloadCombo(profileSelector.GetSafeHwnd(), DM_PROFILES_LOCATION, str("*") + "txt", Form.base);
}

void DmControl::setCoreInfo(DMOutputForm form) {
	defObject.setCurrentInfo(form);
}



