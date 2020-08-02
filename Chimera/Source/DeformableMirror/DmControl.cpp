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

void DmControl::initializeTable(POINT& pos, int width, int height, IChimeraQtWindow* parent)
{
	magLabel = new QLabel ("Mag.", parent);
	magLabel->setGeometry (pos.x, pos.y, width, height);
	angleLabel = new QLabel ("Angle", parent);
	angleLabel->setGeometry (pos.x+width, pos.y+height, width, height);

	comaMag = new QLineEdit (parent);
	comaMag->setGeometry (pos.x, pos.y+= height, width, height);
	comaAngle = new QLineEdit (parent);
	comaAngle->setGeometry (pos.x + width, pos.y, width, height);
	comaLabel = new QLabel ("Coma", parent);
	comaLabel->setGeometry (pos.x + 2*width, pos.y, width, height);

	astigMag = new QLineEdit (parent);
	astigMag->setGeometry (pos.x, pos.y += height, width, height);
	astigAngle = new QLineEdit (parent);
	astigAngle->setGeometry (pos.x + width, pos.y, width, height);
	astigmatismLabel = new QLabel ("Astigmatism", parent);
	astigmatismLabel->setGeometry (pos.x + 2 * width, pos.y, width, height);

	trefoilMag = new QLineEdit (parent);
	trefoilMag->setGeometry (pos.x, pos.y += height, width, height);
	trefoilAngle = new QLineEdit (parent);
	trefoilAngle->setGeometry (pos.x + width, pos.y, width, height);
	trefoilLabel = new QLabel ("Trefoil", parent);
	trefoilLabel->setGeometry (pos.x + 2 * width, pos.y, width, height);

	sphereMag = new QLineEdit (parent);
	sphereMag->setGeometry (pos.x, pos.y += height, width, height);
	sphereAngle = new QLineEdit (parent);
	sphereAngle->setGeometry (pos.x + width, pos.y, width, height);
	sphericalLabel = new QLabel ("Trefoil", parent);
	sphericalLabel->setGeometry (pos.x + 2 * width, pos.y, width, height);

	applyCorrections = new QPushButton ("Apply Corrections", parent);
	applyCorrections->setGeometry (pos.x, pos.y += height, 2 * width + 90, height);
}

void DmControl::initialize( POINT loc, IChimeraQtWindow* win, int count, std::string serialNumber, LONG width )
{
	programNow = new QPushButton ("Program Now", win);
	programNow->setGeometry (loc.x, loc.y, 240, 25);
	profileSelector = new QComboBox (win);
	profileSelector->setGeometry (loc.x, loc.y+=25, 240, 25);
	loc.y -= 25;

	theDMInfo.ActuatorCount = count;
	theDMInfo.serialNumber = serialNumber;
	piston = std::vector<QLineEdit*>(count);
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
			piston[i] = new QLineEdit (win);
			piston[i]->setGeometry (B[i].x, B[i].y, width, width);
		}
	}
	ProfileSystem::reloadCombo(profileSelector, DM_PROFILES_LOCATION, str("*") + "txt", "flatProfile");
	initializeTable(loc, 50, 25, win);
	loadProfile ("flatProfile");
	refreshAbberationDisplays ();
}

void DmControl::handleOnPress(int i) 
{
	piston[i]->setEnabled(true);
}

void DmControl::updateButtons() 
{
	defObject.readDMArray(temp);
	double value;
	for (int i = 0; i < 137; i++) 
	{
		value = temp[i];
		auto editTxt = str (value, 5);
		piston[i]->setText(editTxt.c_str ());
	}
}

void DmControl::ProgramNow() 
{
	std::vector<double> values;
	for (int i = 0; i < 137; i++) 
	{
		std::string s2 = str(piston[i]->text(), 4, false, true);
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
		catch(ChimeraError&)
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

void DmControl::loadProfile()
{
	int id = profileSelector->currentIndex();
	auto file = profileSelector->itemText(id);
	std::string filename = str(file);
	std::ifstream in_file(DM_PROFILES_LOCATION + "\\" + filename + ".txt");
	if (!in_file.is_open()) {
		thrower("DM profile fails to open!");
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
		pis->setText(editTxt.c_str());
		temp[count] = voltage;
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
			pis->setText(editTxt.c_str ());
			temp[count] = voltage;
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
	std::string s[8];
	s[0] = str (comaMag->text());
	s[1] = str (comaAngle->text());
	s[2] = str (astigMag->text ());
	s[3] = str (astigAngle->text ());
	s[4] = str (trefoilMag->text ());
	s[5] = str (trefoilAngle->text ());
	s[6] = str (sphereMag->text ());
	s[7] = str (sphereAngle->text ());
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
	int id = profileSelector->currentIndex();
	auto file = profileSelector->itemText(id);
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
	std::string s[8];
	s[0] = str(comaMag->text());
	s[1] = str (comaAngle->text ());
	s[2] = str (astigMag->text ());
	s[3] = str (astigAngle->text ());
	s[4] = str (trefoilMag->text ());
	s[5] = str (trefoilAngle->text ());
	s[6] = str (sphereMag->text ());
	s[7] = cstr(profileSelector->currentText ());

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
	comaMag->setText(cstr (currentValues.coma.expressionStr));
	comaAngle->setText (cstr (currentValues.comaAng.expressionStr));
	astigMag->setText (cstr (currentValues.astig.expressionStr));
	astigAngle->setText (cstr (currentValues.astigAng.expressionStr));
	trefoilMag->setText (cstr (currentValues.trefoil.expressionStr));
	trefoilAngle->setText (cstr (currentValues.trefoilAng.expressionStr));
	sphereMag->setText (cstr (currentValues.spherical.expressionStr));
	ProfileSystem::reloadCombo (profileSelector, DM_PROFILES_LOCATION, str ("*") + "txt",
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


