//Created by Max Kolanz
#include "stdafx.h"
#include "DmControl.h"
#include "ConfigurationSystems/ProfileSystem.h"
#include <boost/lexical_cast.hpp>


DmControl::DmControl(std::string serialNumber, bool safeMode) : defObject(serialNumber, safeMode), Profile() 
{
	defObject.initialize();
	temp = std::vector<double>(137);
	std::string location = DM_FLAT_PROFILE;
	Profile.createZernikeArray(Profile.getCurrAmps(), location, false);
	Profile.readZernikeFile(location);
}

void DmControl::initializeTable(POINT& pos, int width, int height, CWnd* parent, UINT id) 
{

	magLabel.sPos = { pos.x, pos.y, pos.x + width, pos.y + height };
	magLabel.Create(cstr("Mag."), NORM_STATIC_OPTIONS, magLabel.sPos, parent, id + 1);
	angleLabel.sPos = { pos.x + width, pos.y, pos.x + 2*width, pos.y += height};
	angleLabel.Create(cstr("Angle"), NORM_STATIC_OPTIONS, angleLabel.sPos, parent, id + 2);

	comaMag.sPos = { pos.x, pos.y, pos.x + width, pos.y + height };
	comaMag.Create (NORM_EDIT_OPTIONS | WS_BORDER | ES_CENTER, comaMag.sPos, parent, id + 7);
	comaAngle.sPos = { pos.x + width, pos.y, pos.x + width + width, pos.y + height };
	comaAngle.Create (NORM_EDIT_OPTIONS | WS_BORDER | ES_CENTER, comaAngle.sPos, parent, id + 8);
	comaLabel.sPos = { pos.x + 2*width, pos.y, pos.x + 2 * width +90, pos.y += height };
	comaLabel.Create(cstr("Coma"), NORM_STATIC_OPTIONS, comaLabel.sPos, parent, id + 3);

	astigMag.sPos = { pos.x, pos.y, pos.x + width, pos.y + height };
	astigMag.Create (NORM_EDIT_OPTIONS | WS_BORDER | ES_CENTER, astigMag.sPos, parent, id + 9);
	astigAngle.sPos = { pos.x + width, pos.y, pos.x + width + width, pos.y + height };
	astigAngle.Create (NORM_EDIT_OPTIONS | WS_BORDER | ES_CENTER, astigAngle.sPos, parent, id + 10); 
	astigmatismLabel.sPos = { pos.x + 2 * width, pos.y, pos.x + 2 * width + 90, pos.y += height};
	astigmatismLabel.Create(cstr("Astigmatism"), NORM_STATIC_OPTIONS, astigmatismLabel.sPos, parent, id + 4);
	
	trefoilMag.sPos = { pos.x, pos.y, pos.x + width, pos.y + height };
	trefoilMag.Create (NORM_EDIT_OPTIONS | WS_BORDER | ES_CENTER, trefoilMag.sPos, parent, id + 11);
	trefoilAngle.sPos = { pos.x + width, pos.y, pos.x + width + width, pos.y + height };
	trefoilAngle.Create (NORM_EDIT_OPTIONS | WS_BORDER | ES_CENTER, trefoilAngle.sPos, parent, id + 12); 
	trefoilLabel.sPos = { pos.x + 2 * width, pos.y, pos.x + 2 * width + 90, pos.y += height };
	trefoilLabel.Create(cstr("Trefoil"), NORM_STATIC_OPTIONS, trefoilLabel.sPos, parent, id + 5);
	
	sphereMag.sPos = { pos.x, pos.y , pos.x + width, pos.y + height };
	sphereMag.Create (NORM_EDIT_OPTIONS | WS_BORDER | ES_CENTER, sphereMag.sPos, parent, id + 13);
	sphereAngle.sPos = { pos.x + width, pos.y , pos.x + width + width, pos.y + height };
	sphereAngle.Create (NORM_EDIT_OPTIONS | WS_BORDER | ES_CENTER, sphereAngle.sPos, parent, id + 14); 
	sphereAngle.EnableWindow ( false );
	sphericalLabel.sPos = { pos.x + 2 * width, pos.y, pos.x + 2 * width + 90, pos.y += height };
	sphericalLabel.Create(cstr("Spherical"), NORM_STATIC_OPTIONS, sphericalLabel.sPos, parent, id + 6);

	applyCorrections.sPos = { pos.x, pos.y, pos.x + 2 * width + 90, pos.y + height };
	applyCorrections.Create("Apply Corrections", NORM_PUSH_OPTIONS, applyCorrections.sPos, parent, id + 15);
}

void DmControl::initialize(POINT loc, CWnd* parent, int count, std::string serialNumber, LONG width, UINT &control_id) 
{
	programNow.sPos = { loc.x, loc.y, loc.x + 240, loc.y += 25 };
	programNow.Create ("Program Now", NORM_PUSH_OPTIONS, programNow.sPos, parent, IDC_DM_PROGRAMNOW);
	profileSelector.sPos = { loc.x, loc.y, loc.x + 240, loc.y + 500 };
	loc.y -= 25;
	profileSelector.Create (NORM_COMBO_OPTIONS, profileSelector.sPos, parent, IDC_DM_PROFILE_COMBO);

	theDMInfo.ActuatorCount = count;
	theDMInfo.serialNumber = serialNumber;
	piston = std::vector<pistonButton>(count);
	POINT B[137];
	loc.x += 4 * width;
	POINT A[13] = { {loc}, 
					{loc.x - (2 * width), loc.y + width }, 
					{loc.x - (3 * width), loc.y +  2 * width},
					{loc.x - (3 * width), loc.y +  3 * width}, 
					{loc.x - (4 * width), loc.y +  4 * width},
					{loc.x - (4 * width), loc.y +  5 * width}, 
					{loc.x - (4 * width), loc.y +  6 * width},
					{loc.x - (4 * width), loc.y +  7 * width}, 
					{loc.x - (4 * width), loc.y +  8 * width},
					{loc.x - (3 * width), loc.y +  9 * width},	
					{loc.x - (3 * width), loc.y + 10 * width},
					{loc.x - (2 * width), loc.y + 11 * width}, 
					{loc.x, loc.y + 12 * width } };
	loc.x -= 4 * width;
	loc.y += 12 * width;
	int numCollumns[13] = { 5, 9, 11, 11, 13, 13, 13, 13, 13, 11, 11, 9, 5 };
	int editNum = 0;
	for (int rowInc : range(13))
	{
		for (int z : range(numCollumns[rowInc]))
		{
			B[editNum++] = { A[rowInc].x, A[rowInc].y };
			A[rowInc].x += width;
		}
	}
	if (count != 137)
	{
		thrower("GUI Only Works with acutator count of 137!");
	}
	else 
	{
		for (int i = 0; i < 137; i++) 
		{
			piston[i].Voltage.sPos = { B[i].x, B[i].y, B[i].x + width, B[i].y + width };
			piston[i].Voltage.Create(NORM_EDIT_OPTIONS | WS_BORDER | ES_CENTER | ES_WANTRETURN | WS_EX_STATICEDGE
										| ES_MULTILINE, piston[i].Voltage.sPos, parent, IDC_DM_EDIT_START + i);
		}
	}
	ProfileSystem::reloadCombo(profileSelector.GetSafeHwnd(), DM_PROFILES_LOCATION, str("*") + "txt", "flatProfile");
	initializeTable(loc, 50, 25, parent, IDC_DM_ADD_ZERNIKE);
	loadProfile ("flatProfile");
	refreshAbberationDisplays ();
}

void DmControl::handleOnPress(int i) 
{
	piston[i].Voltage.EnableWindow();
}

void DmControl::updateButtons() 
{
	defObject.readDMArray(temp);
	double value;
	for (int i = 0; i < 137; i++) 
	{
		value = temp[i];
		auto editTxt = str (value, 5);
		piston[i].Voltage.SetWindowTextA(editTxt.c_str ());
	}
}

void DmControl::ProgramNow() 
{
	std::vector<double> values;
	for (int i = 0; i < 137; i++) 
	{
		CString s1;
		piston[i].Voltage.GetWindowTextA(s1);
		std::string s2 = str(s1, 4, false, true);
		try{
			double s3 = boost::lexical_cast<double>(s2);
			if (s3 < 0 || s3 > 1) 
			{
				thrower("cannot enter values greater than 1 or less than 0");
				values.push_back(0.0);
			}
			else 
			{
				values.push_back(s3);
			}
		}
		catch(Error&)
		{
			values.push_back(0.0);
		}	
	}
	defObject.loadArray(values.data());
	updateButtons();
}

void DmControl::setMirror(double *A) 
{
	defObject.loadArray(A);
	updateButtons(); //pass an array and program it

}

int DmControl::getActNum() 
{
	return defObject.getActCount();
}

bool DmControl::isFloat(const std::string& someString) //put in class
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
	for (auto& pis : piston) 
	{
		if (controlID == pis.Voltage.GetDlgCtrlID()) 
		{
			CString s1;
			double s3 = 0;
			pis.Voltage.GetWindowTextA(s1);
			std::string s2 = str(s1, 4, false, true);
			try {
				if (isFloat(s2) )
				{
					s2.erase(std::remove(s2.begin(), s2.end(), '\n'), s2.end());
					s3 = boost::lexical_cast<double>(s2);
					if (s3 < 0 || s3 > 1)
					{
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
			if (s3 >= 0.75) 
			{
				cDC->SetTextColor(_myRGBs["Black"]);
			}
			else 
			{
				cDC->SetTextColor(_myRGBs["Text"]);
			}
			return *_myBrushes[name];
		}
	}
	
	return NULL;
}

void DmControl::reColor(UINT id) 
{
	for (auto& pis : piston) 
	{
		if (id == pis.Voltage.GetDlgCtrlID()) 
		{
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
	angleLabel.rearrange(width, height, fonts);
	magLabel.rearrange(width, height, fonts);
	astigmatismLabel.rearrange(width, height, fonts);
	sphericalLabel.rearrange(width, height, fonts);
	trefoilLabel.rearrange(width, height, fonts);
	comaLabel.rearrange(width, height, fonts);
	programNow.rearrange(width, height, fonts);
	profileSelector.rearrange(width, height, fonts);
	for (auto& pis : piston) 
	{
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
		try 
		{
			voltage = boost::lexical_cast<double>(value);
		}
		catch (boost::bad_lexical_cast) 
		{
			voltage = 0.0;
		}
		auto editTxt = str (voltage, 5);
		pis.Voltage.SetWindowTextA(editTxt.c_str());
		temp[count] = voltage;
		reColor(IDC_DM_PROFILE_COMBO + count);
		count++;
	}
	setMirror(temp.data()); 	
}

void DmControl::loadProfile(std:: string filename)
{
		std::ifstream file(DM_PROFILES_LOCATION + "\\" + filename + ".txt");
		if (!file.is_open()) 
		{
			thrower("File did not open");
		}
		std::string value;
		double voltage;
		int count = 0;
		for (auto& pis : piston) 
		{
			std::getline(file, value);
			try 
			{
				voltage = boost::lexical_cast<double>(value);
			}
			catch (boost::bad_lexical_cast) 
			{
				voltage = 0.0;
			}
			auto editTxt = str (voltage, 5);
			pis.Voltage.SetWindowTextA(editTxt.c_str ());
			temp[count] = voltage;
			reColor(IDC_DM_PROFILE_COMBO + count);
			count++;
		}	
		setMirror(temp.data());
}

void DmControl::writeCurrentFile(std::string out_file) 
{
	std::ofstream outWrite(DM_PROFILES_LOCATION + "//" + out_file);
	for (auto& element : writeArray) 
	{
		outWrite << element << std::endl;
	}
}

std::vector<double> DmControl::getTableValues() 
{
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
	for (int i = 0; i < 8; i++) 
	{
		s2 = str(s[i], 4, false, true);
		if (s2.length() == 0) 
		{
			s2 = "0.0";
		}
		try 
		{
			parameters[i] = boost::lexical_cast<double>(s2);
		}
		catch (boost::bad_lexical_cast&) {
			parameters[i] = 0.0;
			thrower("Error:  bad lexical cast");
		}
	}
	return parameters;
}

void DmControl::add_Changes() 
{
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

DmCore &DmControl::getCore() 
{
	return defObject;
}

DMOutputForm DmControl::getExpressionValues() 
{
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

void DmControl::handleSaveConfig(ConfigStream& saveStream) 
{
	defObject.handleSaveConfig(saveStream, getExpressionValues());
}

void DmControl::refreshAbberationDisplays ()
{
	comaMag.SetWindowTextA (cstr (currentValues.coma.expressionStr));
	comaAngle.SetWindowTextA (cstr (currentValues.comaAng.expressionStr));
	astigMag.SetWindowTextA (cstr (currentValues.astig.expressionStr));
	astigAngle.SetWindowTextA (cstr (currentValues.astigAng.expressionStr));
	trefoilMag.SetWindowTextA (cstr (currentValues.trefoil.expressionStr));
	trefoilAngle.SetWindowTextA (cstr (currentValues.trefoilAng.expressionStr));
	sphereMag.SetWindowTextA (cstr (currentValues.spherical.expressionStr));
	ProfileSystem::reloadCombo (profileSelector.GetSafeHwnd (), DM_PROFILES_LOCATION, str ("*") + "txt",
		currentValues.base);
}

void DmControl::openConfig() 
{
	currentValues = defObject.getInfo();
	refreshAbberationDisplays ();
}

void DmControl::setCoreInfo (DMOutputForm form)
{
	defObject.setCurrentInfo (form);
}


