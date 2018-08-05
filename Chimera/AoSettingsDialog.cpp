#include "stdafx.h"
#include "AoSystem.h"
#include "AoSettingsDialog.h"

IMPLEMENT_DYNAMIC(AoSettingsDialog, CDialog)

BEGIN_MESSAGE_MAP(AoSettingsDialog, CDialog)
	ON_COMMAND(10001, &AoSettingsDialog::handleOk)
	//ON_COMMAND(IDOK, &AoSettingsDialog::handleEnter )
END_MESSAGE_MAP()

/*
void AoSettingsDialog::handleEnter ( )
{

}
*/

void AoSettingsDialog::handleOk()
{
	for (UINT dacInc = 0; dacInc < nameEdits.size(); dacInc++)
	{
		/// MAKE SURE that there are 16 numbers per row.
		CString text;
		nameEdits[dacInc].GetWindowTextA(text);
		if (isdigit(text[0]))
		{
			errBox("ERROR: " + str(text) + " is an invalid name; names cannot start with numbers.");
			return;
		}
		input->aoSys->setName(dacInc, str(text), input->toolTips, input->master);
		noteEdits[ dacInc ].GetWindowTextA ( text );
		input->aoSys->setNote ( dacInc, str ( text ), input->toolTips, input->master );
		double min, max;
		try
		{
			minValEdits[dacInc].GetWindowTextA(text);
			min = std::stod(str(text));
			maxValEdits[dacInc].GetWindowTextA(text);
			max = std::stod(str(text));
			input->aoSys->setMinMax(dacInc, min, max);
		}
		catch (std::invalid_argument& err)
		{
			errBox(err.what());
			return;
		}
	}
	EndDialog(0);
}

void AoSettingsDialog::handleCancel()
{
	EndDialog(0);
}

BOOL AoSettingsDialog::OnInitDialog()
{
	POINT loc = { 0,0 };
	RECT r;
	GetClientRect( &r );
	UINT columnNumber = 3;
	LONG columnSize = (r.right-20*columnNumber) / columnNumber;
	// for rounding issues.
	columnSize = columnSize / 4 + columnSize / 8 + columnSize / 8 + columnSize / 2;
	LONG rowHeight = 25;
	// headers
	for (int columnInc = 0; columnInc < columnNumber; columnInc++)
	{
		dacNumberHeaders[columnInc].sPos = { loc.x, loc.y, loc.x + 20, loc.y + 20 };
		loc.x += 20;
		dacNumberHeaders[columnInc].Create("#", NORM_HEADER_OPTIONS | WS_BORDER,
										   dacNumberHeaders[columnInc].sPos, this);

		dacNameHeaders[columnInc].sPos = { loc.x, loc.y, loc.x + columnSize/4, loc.y + rowHeight+5 };
		loc.x += columnSize/4;
		dacNameHeaders[columnInc].Create("Dac Name", NORM_HEADER_OPTIONS | WS_BORDER,
										 dacNameHeaders[columnInc].sPos, this);
		dacMinValHeaders[columnInc].sPos = { loc.x, loc.y, loc.x + columnSize / 8, loc.y + rowHeight + 5 };
		loc.x += columnSize/8;
		dacMinValHeaders[columnInc].Create("Min", NORM_HEADER_OPTIONS | WS_BORDER,
										   dacMinValHeaders[columnInc].sPos, this);
		dacMaxValHeaders[columnInc].sPos = { loc.x, loc.y, loc.x + columnSize / 8, loc.y + rowHeight + 5 };
		loc.x += columnSize/8;
		dacMaxValHeaders[columnInc].Create("Max", NORM_HEADER_OPTIONS | WS_BORDER,
										   dacMaxValHeaders[columnInc].sPos, this);
		noteHeaders[columnInc ].sPos = { loc.x, loc.y, loc.x + columnSize / 2, loc.y + rowHeight + 5 };
		loc.x += columnSize / 2;
		noteHeaders[ columnInc ].Create ( "Notes", NORM_HEADER_OPTIONS | WS_BORDER, 
										  noteHeaders[ columnInc ].sPos, this );
	}

	loc.y += rowHeight + 5;
	loc.x -= (columnSize +20) * columnNumber;

	for (UINT dacInc = 0; dacInc < nameEdits.size(); dacInc++)
	{
		if (dacInc == nameEdits.size() / columnNumber || dacInc == 2 * nameEdits.size() / columnNumber )
		{
			// go to second or third collumn
			loc.x += columnSize+20;
			loc.y -= rowHeight * nameEdits.size() / columnNumber;
		}
		// create label
		numberLabels[dacInc].sPos = { loc.x, loc.y, loc.x += 20, loc.y + rowHeight };
		numberLabels[dacInc].Create(cstr(dacInc), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, 
									numberLabels[dacInc].sPos, this);

		nameEdits[dacInc].sPos = { loc.x, loc.y, loc.x += columnSize / 4, loc.y + rowHeight };
		nameEdits[dacInc].Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, nameEdits[dacInc].sPos, this, id++);
		nameEdits[dacInc].SetWindowTextA(cstr(input->aoSys->getName(dacInc)));

		minValEdits[dacInc].sPos = { loc.x, loc.y, loc.x += columnSize / 8, loc.y + rowHeight };
		minValEdits[dacInc].Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, minValEdits[dacInc].sPos, this, id++);
		auto range = input->aoSys->getDacRange ( dacInc );
		minValEdits[dacInc].SetWindowTextA(cstr(range.first));

		maxValEdits[dacInc].sPos = { loc.x, loc.y, loc.x += columnSize / 8, loc.y + rowHeight };
		maxValEdits[dacInc].Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, maxValEdits[dacInc].sPos, this, id++);
		maxValEdits[dacInc].SetWindowTextA(cstr( range.second));

		noteEdits[ dacInc ].sPos = { loc.x, loc.y, loc.x += columnSize / 2, loc.y + rowHeight };
		noteEdits[ dacInc ].Create ( WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_MULTILINE | ES_WANTRETURN, 
									 noteEdits[ dacInc ].sPos, this, id++ );
		noteEdits[ dacInc ].SetWindowTextA ( cstr ( input->aoSys->getNote ( dacInc ) ) );

		loc.y += rowHeight;
		loc.x -= columnSize+20;
	}
	return TRUE;
}
