#include "stdafx.h"
#include <ExcessDialogs/doChannelInfoDialog.h>
#include <DigitalOutput/DoSystem.h>

doChannelInfoDialog::doChannelInfoDialog (ttlInputStruct* inputPtr){
	input = inputPtr;
	LONG columnWidth = 120;
	LONG labelSize = 65;
	LONG rowSize = 30;
	POINT pos = { labelSize, 0 };

	for (unsigned numInc : range (edits.front ().size ())) {
		numberlabels[numInc] = new QLabel (qstr (numInc), this);
		numberlabels[numInc]->setGeometry (pos.x, pos.y, columnWidth, rowSize);
		numberlabels[numInc]->setAlignment (Qt::AlignCenter);
		pos.x += columnWidth;
	}
	for (auto row : DoRows::allRows){
		pos.y += rowSize;
		pos.x = 0;
		rowLabels[int (row)] = new QLabel (qstr (DoRows::toStr (row)), this); 
		rowLabels[int (row)]->setGeometry (pos.x, pos.y, labelSize, rowSize);
		rowLabels[int (row)]->setAlignment (Qt::AlignCenter);

		pos.x += labelSize;
		for (unsigned numberInc : range( edits[int (row)].size ())){
			edits[int (row)][numberInc] = new QLineEdit (this);
			edits[int (row)][numberInc]->setGeometry (pos.x, pos.y, columnWidth, rowSize);
			edits[int (row)][numberInc]->setText (qstr (input->ttls->getName (row, numberInc)));
			edits[int (row)][numberInc]->setToolTip("Original: " + qstr (input->ttls->getName (row, numberInc)));
			pos.x += columnWidth;
		}
	}
	pos.x = 0;
	pos.y += rowSize;
	okBtn = new QPushButton ("OK", this);
	okBtn->setGeometry (pos.x, pos.y, 200, 30);
	connect (okBtn, &QPushButton::released, this, &doChannelInfoDialog::handleOk);
	cancelBtn = new QPushButton ("CANCEL", this);
	cancelBtn->setGeometry (pos.x+200, pos.y, 200, 30);
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