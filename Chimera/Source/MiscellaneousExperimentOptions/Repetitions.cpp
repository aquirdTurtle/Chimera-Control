// created by Mark O. Brown
#include "stdafx.h"
#include "Repetitions.h"
#include <unordered_map>
#include "LowLevel/constants.h"
#include "PrimaryWindows/QtAuxiliaryWindow.h"
#include <boost/lexical_cast.hpp>
#include "PrimaryWindows/QtMainWindow.h"

unsigned Repetitions::getSettingsFromConfig (ConfigStream& openFile )
{
	unsigned repNum;
	openFile >> repNum;
	return repNum;
}



void Repetitions::handleSaveConfig(ConfigStream& saveFile)
{
	saveFile << "REPETITIONS\n";
	saveFile << "/*Reps:*/" << getRepetitionNumber ();
	saveFile << "\nEND_REPETITIONS\n";
}

void Repetitions::updateNumber(long repNumber)
{
	repetitionDisp->setText (cstr (repNumber));
}


void Repetitions::initialize(POINT& pos, IChimeraQtWindow* parent )
{
	repetitionNumber = 100;
	// title
	repetitionText = new QLabel ("Repetition #", parent);
	repetitionText->setGeometry (QRect (pos.x, pos.y, 180, 20));
	repetitionEdit = new CQLineEdit (cstr (repetitionNumber), parent);
	repetitionEdit->setGeometry (QRect (pos.x + 180, pos.y, 150, 20));
	parent->connect (repetitionEdit, &QLineEdit::textChanged, [parent]() {parent->configUpdated (); });
	repetitionDisp = new QLabel ("-", parent);
	repetitionDisp->setGeometry (QRect (pos.x + 330, pos.y, 150, 20));
	pos.y += 20;
}


void Repetitions::setRepetitions(unsigned number)
{
	repetitionNumber = number;
	repetitionEdit->setText (cstr (number));
	repetitionDisp->setText("---");
}


unsigned Repetitions::getRepetitionNumber()
{
	auto text = repetitionEdit->text ();
	try
	{
		repetitionNumber = boost::lexical_cast<int>(str(text));
	}
	catch ( boost::bad_lexical_cast&)
	{
		throwNested ("Failed to convert repetition number text to an integer!");
	}
	return repetitionNumber;
}

