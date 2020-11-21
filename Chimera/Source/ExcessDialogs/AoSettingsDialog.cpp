// created by Mark O. Brown
#include "stdafx.h"
#include "AoSettingsDialog.h"
#include <boost/lexical_cast.hpp>

AoSettingsDialog::AoSettingsDialog (aoInputStruct* inputPtr) {
	input = inputPtr;
	this->resize (1800, 800);
	int px = 1, py = 1;
	unsigned numWidth = 24;
	unsigned columnNumber = 3;
	int colWidth = (1800 - numWidth * columnNumber) / columnNumber;
	colWidth = colWidth / 4 + colWidth / 8 + colWidth / 8 + colWidth / 2;
	int rowHeight = 30;
	// The header row
	for (int columnInc = 0; columnInc < columnNumber; columnInc++) {
		dacNumberHeaders[columnInc] = new QLabel ("#", this);
		dacNumberHeaders[columnInc]->setGeometry (px, py, numWidth, 20);

		dacNameHeaders[columnInc] = new QLabel ("Dac Name", this);
		dacNameHeaders[columnInc]->setGeometry (px += numWidth, py, colWidth / 4, rowHeight + 5);

		dacMinValHeaders[columnInc] = new QLabel ("Min", this);
		dacMinValHeaders[columnInc]->setGeometry (px += colWidth / 4, py, colWidth / 8, rowHeight + 5);

		dacMaxValHeaders[columnInc] = new QLabel ("Max", this);
		dacMaxValHeaders[columnInc]->setGeometry (px += colWidth / 8, py, colWidth / 8, rowHeight + 5);

		noteHeaders[columnInc] = new QLabel ("Notes", this);
		noteHeaders[columnInc]->setGeometry (px += colWidth / 8, py, colWidth / 2, rowHeight + 5);
		px += colWidth / 2;
	}

	py += rowHeight + 5;
	px -= (colWidth + numWidth) * columnNumber;

	for (unsigned dacInc = 0; dacInc < nameEdits.size (); dacInc++) {
		if (dacInc == nameEdits.size () / columnNumber || dacInc == 2 * nameEdits.size () / columnNumber) {
			// go to second or third collumn
			px += colWidth + numWidth;
			py -= rowHeight * nameEdits.size () / columnNumber;
		}
		// create label
		numberLabels[dacInc] = new QLabel (qstr (dacInc), this);
		numberLabels[dacInc]->setGeometry (px, py, numWidth, rowHeight);

		nameEdits[dacInc] = new QLineEdit (qstr (input->aoSys->getName (dacInc)), this);
		nameEdits[dacInc]->setGeometry (px += numWidth, py, colWidth / 4, rowHeight);

		minValEdits[dacInc] = new QLineEdit (qstr (input->aoSys->getDacRange (dacInc).first,4), this);
		minValEdits[dacInc]->setGeometry (px += colWidth / 4, py, colWidth / 8, rowHeight);

		maxValEdits[dacInc] = new QLineEdit (qstr (input->aoSys->getDacRange (dacInc).second,4), this);
		maxValEdits[dacInc]->setGeometry (px += colWidth / 8, py, colWidth / 8, rowHeight);

		noteEdits[dacInc] = new QLineEdit (qstr (input->aoSys->getNote (dacInc)), this);
		noteEdits[dacInc]->setGeometry (px += colWidth / 8, py, colWidth / 2, rowHeight);
		py += rowHeight; 
		px += colWidth / 2;
		px -= colWidth + numWidth;
	}

	okBtn = new QPushButton ("OK", this);
	okBtn->setGeometry (px, py, 100, rowHeight);
	connect (okBtn, &QPushButton::released, this, &AoSettingsDialog::handleOk);
	cancelBtn = new QPushButton ("Cancel", this);
	cancelBtn->setGeometry (px + 100, py, 100, rowHeight);
	connect (cancelBtn, &QPushButton::released, this, &AoSettingsDialog::handleCancel);
}

void AoSettingsDialog::handleOk(){
	for (unsigned dacInc = 0; dacInc < nameEdits.size(); dacInc++){
		auto text = nameEdits[dacInc]->text ();
		if (text[0].isDigit()){
			errBox("ERROR: " + str(text) + " is an invalid name; names cannot start with numbers.");
			return;
		}
		input->aoSys->setName(dacInc, str(text));
		text = noteEdits[dacInc]->text ();
		input->aoSys->setNote ( dacInc, str ( text ) );
		double min, max;
		try	{
			text = minValEdits[dacInc]->text ();
			min = boost::lexical_cast<double>(str(text));
			text = maxValEdits[dacInc]->text ();
			max = boost::lexical_cast<double>(str(text));
			input->aoSys->setMinMax(dacInc, min, max);
		}
		catch ( boost::bad_lexical_cast& err) {
			errBox(err.what());
			return;
		}
	}
	close ();
}

void AoSettingsDialog::handleCancel(){
	close();
}
