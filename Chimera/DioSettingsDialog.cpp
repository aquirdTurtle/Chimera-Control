// created by Mark O. Brown
#pragma once

#include "stdafx.h"
#include "DioSettingsDialog.h"
#include "Control.h"
#include <array>
#include "DioSystem.h"


BEGIN_MESSAGE_MAP(TtlSettingsDialog, CDialog)
	ON_COMMAND(IDOK, &TtlSettingsDialog::handleOk)
	ON_COMMAND(IDCANCEL, &TtlSettingsDialog::handleCancel)
	ON_WM_SIZE()
END_MESSAGE_MAP()


void TtlSettingsDialog::OnSize( UINT type, int w, int h )
{
	/*
	for ( auto& label : numberlabels )
	{
		label.rearrange( w, h );
	}
	for ( auto& row : rowLabels )
	{
		row.rearrange( w, h );
	}
	for ( auto& row : edits )
	{
		for ( auto& edit : row )
		{
			edit.rearrange( w, h );
		}
	}
	*/
}


TtlSettingsDialog::TtlSettingsDialog(ttlInputStruct* inputPtr, UINT dialogResource) : CDialog(dialogResource)
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

	for (UINT numberInc = 0; numberInc < edits.front().size(); numberInc++)
	{
		numberlabels[numberInc].sPos = { pos.x, pos.y, pos.x += columnWidth, pos.y + rowSize };
		numberlabels[numberInc].Create( cstr(numberInc), WS_CHILD | WS_VISIBLE | SS_SUNKEN | WS_BORDER | ES_CENTER |
									   ES_AUTOHSCROLL | WS_TABSTOP, numberlabels[numberInc].sPos, this, id++);
	}
	
	for (UINT rowInc = 0; rowInc < edits.size(); rowInc++)
	{
		pos.y += rowSize;
		pos.x = 0;
		rowLabels[rowInc].sPos = { pos.x, pos.y, pos.x += labelSize, pos.y + rowSize };
		std::string text;
		switch (rowInc)
		{
			case 0: text = "A"; break;
			case 1: text = "B"; break;
			case 2: text = "C"; break;
			case 3: text = "D"; break;
		}
		rowLabels[rowInc].Create(cstr(text), WS_CHILD | ES_CENTER | WS_VISIBLE , rowLabels[rowInc].sPos, this, id++);
		for (UINT numberInc = 0; numberInc < edits[rowInc].size(); numberInc++)
		{
			edits[rowInc][numberInc].sPos = { pos.x, pos.y, pos.x += columnWidth, pos.y + rowSize };
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