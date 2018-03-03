#include "stdafx.h"
#include "AoSystem.h"
#include "AoSettingsDialog.h"

IMPLEMENT_DYNAMIC(AoSettingsDialog, CDialog)

BEGIN_MESSAGE_MAP(AoSettingsDialog, CDialog)
	ON_COMMAND(IDOK, &AoSettingsDialog::handleOk)
END_MESSAGE_MAP()


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
	LONG rowHeight = 25;
	// headers
	for (int columnInc = 0; columnInc < columnNumber; columnInc++)
	{
		dacNumberHeaders[columnInc].sPos = { loc.x, loc.y, loc.x + 20, loc.y + 20 };
		loc.x += 20;
		dacNumberHeaders[columnInc].Create("#", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER | WS_BORDER,
										   dacNumberHeaders[columnInc].sPos, this);

		dacNameHeaders[columnInc].sPos = { loc.x, loc.y, loc.x + columnSize/2, loc.y + rowHeight+5 };
		loc.x += columnSize/2;
		dacNameHeaders[columnInc].Create("Dac Name", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER | WS_BORDER,
										 dacNameHeaders[columnInc].sPos, this);
		dacMinValHeaders[columnInc].sPos = { loc.x, loc.y, loc.x + columnSize / 4, loc.y + rowHeight + 5 };
		loc.x += columnSize/4;
		dacMinValHeaders[columnInc].Create("Min Value", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER | WS_BORDER,
										   dacMinValHeaders[columnInc].sPos, this);
		dacMaxValHeaders[columnInc].sPos = { loc.x, loc.y, loc.x + columnSize / 4, loc.y + rowHeight + 5 };
		loc.x += columnSize/4;
		dacMaxValHeaders[columnInc].Create("Max Value", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER | WS_BORDER,
										   dacMaxValHeaders[columnInc].sPos, this);
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

		nameEdits[dacInc].sPos = { loc.x, loc.y, loc.x += columnSize/2, loc.y + rowHeight };
		nameEdits[dacInc].Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, nameEdits[dacInc].sPos, this, id++);
		nameEdits[dacInc].SetWindowTextA(cstr(input->aoSys->getName(dacInc)));

		minValEdits[dacInc].sPos = { loc.x, loc.y, loc.x += columnSize/4, loc.y + rowHeight };
		minValEdits[dacInc].Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, minValEdits[dacInc].sPos, this, id++);
		minValEdits[dacInc].SetWindowTextA(cstr(input->aoSys->getDacRange(dacInc).first));

		maxValEdits[dacInc].sPos = { loc.x, loc.y, loc.x += columnSize/4, loc.y + rowHeight };
		maxValEdits[dacInc].Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, maxValEdits[dacInc].sPos, this, id++);
		maxValEdits[dacInc].SetWindowTextA(cstr(input->aoSys->getDacRange(dacInc).second));

		loc.y += rowHeight;
		loc.x -= columnSize+20;
	}
	return TRUE;
}
