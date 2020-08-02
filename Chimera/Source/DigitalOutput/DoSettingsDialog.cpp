// created by Mark O. Brown
#pragma once

#include "stdafx.h"
#include "DoSettingsDialog.h"
#include "Control.h"
#include <array>
#include "DoSystem.h"


BEGIN_MESSAGE_MAP(TtlSettingsDialog, CDialog)
	ON_COMMAND(IDOK, &TtlSettingsDialog::handleOk)
	ON_COMMAND(IDCANCEL, &TtlSettingsDialog::handleCancel)
	ON_WM_SIZE()
END_MESSAGE_MAP()


void TtlSettingsDialog::OnSize( unsigned type, int w, int h ){
}


TtlSettingsDialog::TtlSettingsDialog(ttlInputStruct* inputPtr, unsigned dialogResource) : CDialog(dialogResource)
{
	input = inputPtr;
}


BOOL TtlSettingsDialog::OnInitDialog()
{
	int id = 345;

	RECT r;
	GetClientRect( &r );
	LONG columnWidth = (r.right - 65) / 16;
	LONG labelSize = 65;
	LONG rowSize = 30;
	POINT pos = { labelSize, 0};

	for (unsigned numberInc = 0; numberInc < edits.front().size(); numberInc++)
	{
		numberlabels[numberInc].sPos = { pos.x, pos.y, pos.x += columnWidth, pos.y + rowSize };
		numberlabels[numberInc].Create( cstr(numberInc), WS_CHILD | WS_VISIBLE | SS_SUNKEN | WS_BORDER | ES_CENTER |
									   ES_AUTOHSCROLL | WS_TABSTOP, numberlabels[numberInc].sPos, this, id++);
	}
	for (auto row : DoRows::allRows )
	{
		pos.y += rowSize;
		pos.x = 0;
		rowLabels[int(row)].sPos = { pos.x, pos.y, pos.x += labelSize, pos.y + rowSize };
		rowLabels[int(row)].Create(cstr(DoRows::toStr(row)), WS_CHILD | ES_CENTER | WS_VISIBLE , 
									rowLabels[ int ( row ) ].sPos, this, id++);
		for (unsigned numberInc = 0; numberInc < edits[ int ( row ) ].size(); numberInc++)
		{
			edits[ int ( row ) ][numberInc].sPos = { pos.x, pos.y, pos.x += columnWidth, pos.y + rowSize };
			edits[ int ( row ) ][numberInc].Create( WS_CHILD | WS_VISIBLE | SS_SUNKEN | WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP,
											 edits[ int ( row ) ][numberInc].sPos, this, id++);
			edits[ int ( row ) ][numberInc].SetWindowTextA(cstr(input->ttls->getName(row, numberInc)));
		}
	}
	return FALSE;
}


void TtlSettingsDialog::handleOk()
{
	for (auto row : DoRows::allRows )
	{
		for (unsigned numberInc = 0; numberInc < edits[int(row)].size(); numberInc++)
		{
			CString name;
			edits[ int ( row ) ][numberInc].GetWindowText(name);
			if (isdigit(name[0]))
			{
				errBox("ERROR: " + str(name) + " is an invalid name; names cannot start with numbers.");
				return;
			}
			input->ttls->setName(row, numberInc, str(name));
		}
	}
	EndDialog((WPARAM)&input);
}

void TtlSettingsDialog::handleCancel()
{
	EndDialog((WPARAM)&input);
}