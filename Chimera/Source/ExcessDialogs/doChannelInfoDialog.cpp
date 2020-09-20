#include "stdafx.h"

#include <ExcessDialogs/doChannelInfoDialog.h>
#include <DigitalOutput/DoSystem.h>

doChannelInfoDialog::doChannelInfoDialog (ttlInputStruct* inputPtr){
	input = inputPtr;
	long columnWidth = 120;
	long labelSize = 65;
	long rowSize = 30;
	int px=labelSize, py = 0;
	for (unsigned numInc : range (edits.front ().size ())) {
		numberlabels[numInc] = new QLabel (qstr (numInc), this);
		numberlabels[numInc]->setGeometry (px, py, columnWidth, rowSize);
		numberlabels[numInc]->setAlignment (Qt::AlignCenter);
		px += columnWidth;
	}
	for (auto row : DoRows::allRows){
		py += rowSize;
		px = 0;
		rowLabels[int (row)] = new QLabel (qstr (DoRows::toStr (row)), this); 
		rowLabels[int (row)]->setGeometry (px, py, labelSize, rowSize);
		rowLabels[int (row)]->setAlignment (Qt::AlignCenter);
		px += labelSize;
		for (unsigned numberInc : range( edits[int (row)].size ())){
			edits[int (row)][numberInc] = new QLineEdit (this);
			edits[int (row)][numberInc]->setGeometry (px, py, columnWidth, rowSize);
			edits[int (row)][numberInc]->setText (qstr (input->ttls->getName (row, numberInc)));
			edits[int (row)][numberInc]->setToolTip("Original: " + qstr (input->ttls->getName (row, numberInc)));
			px += columnWidth;
		}
	}
	px = 0;
	py += rowSize;
	okBtn = new QPushButton ("OK", this);
	okBtn->setGeometry (px,py, 200, 30);
	connect (okBtn, &QPushButton::released, this, &doChannelInfoDialog::handleOk);
	cancelBtn = new QPushButton ("CANCEL", this);
	cancelBtn->setGeometry (px+200, py, 200, 300);
	connect (cancelBtn, &QPushButton::released, this, &doChannelInfoDialog::handleCancel);
}

void doChannelInfoDialog::handleOk (){
	for (auto row : DoRows::allRows){
		for (unsigned numberInc = 0; numberInc < edits[int (row)].size (); numberInc++)	{
			QString name = edits[int (row)][numberInc]->text ();
			if (name[0].isDigit ()){
				errBox ("ERROR: " + str (name) + " is an invalid name; names cannot start with numbers.");
				return;
			}
			input->ttls->setName (row, numberInc, str (name));
		}
	}
	close ();
}

void doChannelInfoDialog::handleCancel (){
	close ();
}

