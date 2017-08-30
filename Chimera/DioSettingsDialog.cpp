#pragma once

#include "stdafx.h"
#include "DioSettingsDialog.h"
#include "Control.h"
#include <array>
#include "DioSystem.h"


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

	for (UINT numberInc = 0; numberInc < edits.front().size(); numberInc++)
	{
		numberlabels[numberInc].sPos = { pos.x, pos.y, pos.x += 65, pos.y + 30 };
		numberlabels[numberInc].Create( cstr(numberInc + 1), WS_CHILD | WS_VISIBLE | SS_SUNKEN | WS_BORDER | 
									   ES_AUTOHSCROLL | WS_TABSTOP, numberlabels[numberInc].sPos, this, id++);
	}
	
	for (UINT rowInc = 0; rowInc < edits.size(); rowInc++)
	{
		pos.y += 30;
		pos.x = 0;
		rowLabels[rowInc].sPos = { pos.x, pos.y, pos.x += 65, pos.y + 30 };
		std::string text;
		switch (rowInc)
		{
			case 0: text = "A"; break;
			case 1: text = "B"; break;
			case 2: text = "C"; break;
			case 3: text = "D"; break;
		}
		rowLabels[rowInc].Create(cstr(text), WS_CHILD | WS_VISIBLE, rowLabels[rowInc].sPos, this, id++);
		for (UINT numberInc = 0; numberInc < edits[rowInc].size(); numberInc++)
		{
			edits[rowInc][numberInc].sPos = { pos.x, pos.y, pos.x += 65, pos.y + 30 };
			edits[rowInc][numberInc].Create( WS_CHILD | WS_VISIBLE | SS_SUNKEN | WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP,
											 edits[rowInc][numberInc].sPos, this, id++);
			edits[rowInc][numberInc].SetWindowTextA(cstr(input->ttls->getName(rowInc, numberInc)));
		}
	}
	return FALSE;
}


void TtlSettingsDialog::handleOk()
{
	for (UINT rowInc = 0; rowInc < edits.size(); rowInc++)
	{
		for (UINT numberInc = 0; numberInc < edits[rowInc].size(); numberInc++)
		{
			CString name;
			edits[rowInc][numberInc].GetWindowText(name);
			if (isdigit(name[0]))
			{
				errBox("ERROR: " + str(name) + " is an invalid name; names cannot start with numbers.");
				return;
			}
			input->ttls->setName(rowInc, numberInc, str(name), input->toolTips, input->master);
		}
	}
	EndDialog((WPARAM)&input);
}

void TtlSettingsDialog::handleCancel()
{
	EndDialog((WPARAM)&input);
}