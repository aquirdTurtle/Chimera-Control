// created by Mark O. Brown
#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>
#include "QMessageBox.h"
#include <GeneralObjects/ChimeraStyleSheets.h>
#include <GeneralUtilityFunctions/my_str.h>

// this function takes any argument, converts it to a string, and displays it on the screen. It can be useful for debuging.
template <typename T> void errBox( T msg ){
	QMessageBox* messageBox = new QMessageBox();
	messageBox->setStyleSheet ("QDialog {background-color: red;}"
		"QPushButton {background-color: blue;}");
	//messageBox.setStyleSheet (chimeraStyleSheets::stdStyleSheet ());
	messageBox->critical (0, "Error", cstr(msg));
	messageBox->setFixedSize (500, 200);
}

// this function takes any argument, converts it to a string, and displays it on the screen. It can be useful for debuging.
template <typename T> void infoBox( T msg ){
	QMessageBox* messageBox = new QMessageBox ();
	// seems not working for some reason.
	messageBox->setStyleSheet (chimeraStyleSheets::stdStyleSheet ());
	messageBox->information(0, "Information", cstr(msg));
	messageBox->setFixedSize (500, 200);
}

unsigned long getNextFileIndex( std::string fileBase, std::string ext );

