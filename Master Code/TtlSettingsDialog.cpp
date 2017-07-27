#pragma once

#include "stdafx.h"
#include "TtlSettingsDialog.h"
#include "Control.h"
#include <array>
#include "TtlSystem.h"


BEGIN_MESSAGE_MAP(TtlSettingsDialog, CDialog)
	ON_COMMAND(IDOK, &TtlSettingsDialog::handleOk)
	ON_COMMAND(IDCANCEL, &TtlSettingsDialog::handleCancel)
END_MESSAGE_MAP()


TtlSettingsDialog::TtlSettingsDialog(ttlInputStruct* inputPtr, UINT dialogResource) : CDialog(dialogResource)
{
	input = inputPtr;
}


BOOL TtlSettingsDialog::OnInitDialog()
{
	int id = 345;

	POINT pos = { 65, 0};

	for (int numberInc = 0; numberInc < edits.front().size(); numberInc++)
	{
		numberlabels[numberInc].sPos = { pos.x, pos.y, pos.x += 65, pos.y + 30 };
		numberlabels[numberInc].ID = id++;
		numberlabels[numberInc].Create(std::to_string(numberInc + 1).c_str(), WS_CHILD | WS_VISIBLE | SS_SUNKEN 
									   | WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP,
									   numberlabels[numberInc].sPos, this, numberlabels[numberInc].ID);
	}
	
	for (int rowInc = 0; rowInc < edits.size(); rowInc++)
	{
		pos.y += 30;
		pos.x = 0;
		rowLabels[rowInc].sPos = { pos.x, pos.y, pos.x += 65, pos.y + 30 };
		rowLabels[rowInc].ID = id++;
		std::string text;
		switch (rowInc)
		{
			case 0: text = "A"; break;
			case 1: text = "B"; break;
			case 2: text = "C"; break;
			case 3: text = "D"; break;
		}
		rowLabels[rowInc].Create(text.c_str(), WS_CHILD | WS_VISIBLE, rowLabels[rowInc].sPos, this, rowLabels[rowInc].ID);
		for (int numberInc = 0; numberInc < edits[rowInc].size(); numberInc++)
		{
			edits[rowInc][numberInc].sPos = { pos.x, pos.y, pos.x += 65, pos.y + 30 };
			edits[rowInc][numberInc].ID = id++;
			edits[rowInc][numberInc].Create(WS_CHILD | WS_VISIBLE | SS_SUNKEN | WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP,
											edits[rowInc][numberInc].sPos, this, edits[rowInc][numberInc].ID);
			edits[rowInc][numberInc].SetWindowTextA(input->ttls->getName(rowInc, numberInc).c_str());
		}
	}
	return FALSE;
}


void TtlSettingsDialog::handleOk()
{
	for (int rowInc = 0; rowInc < edits.size(); rowInc++)
	{
		for (int numberInc = 0; numberInc < edits[rowInc].size(); numberInc++)
		{
			CString name;
			edits[rowInc][numberInc].GetWindowText(name);
			if (isdigit(name[0]))
			{
				errBox("ERROR: " + std::string(name) + " is an invalid name; names cannot start with numbers.");
				return;
			}
			input->ttls->setName(rowInc, numberInc, std::string(name), input->toolTips, input->master);
		}
	}
	EndDialog((WPARAM)&input);
}

void TtlSettingsDialog::handleCancel()
{
	EndDialog((WPARAM)&input);
}