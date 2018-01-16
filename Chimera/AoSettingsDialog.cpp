#include "stdafx.h"
#include "AoSettingsDialog.h"
#include "AoSystem.h"

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
	//input = *(aoInputStruct*)lParam;
	POINT loc = { 0,0 };
	// headers
	for (int columnInc = 0; columnInc < 3; columnInc++)
	{
		dacNumberHeaders[columnInc].sPos = { loc.x, loc.y, loc.x + 20, loc.y + 20 };
		loc.x += 20;
		dacNumberHeaders[columnInc].Create("#", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER | WS_BORDER,
										   dacNumberHeaders[columnInc].sPos, this);

		dacNameHeaders[columnInc].sPos = { loc.x, loc.y, loc.x + 120, loc.y + 20 };
		loc.x += 120;
		dacNameHeaders[columnInc].Create("Dac Name", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER | WS_BORDER,
										 dacNameHeaders[columnInc].sPos, this);
		dacMinValHeaders[columnInc].sPos = { loc.x, loc.y, loc.x + 120, loc.y + 20 };
		loc.x += 120;
		dacMinValHeaders[columnInc].Create("Min Value", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER | WS_BORDER,
										   dacMinValHeaders[columnInc].sPos, this);
		dacMaxValHeaders[columnInc].sPos = { loc.x, loc.y, loc.x + 120, loc.y + 20 };
		loc.x += 120;
		dacMaxValHeaders[columnInc].Create("Max Value", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER | WS_BORDER,
										   dacMaxValHeaders[columnInc].sPos, this);
	}

	loc.y += 25;
	loc.x -= 380 * 3;

	for (UINT dacInc = 0; dacInc < nameEdits.size(); dacInc++)
	{
		if (dacInc == nameEdits.size() / 3 || dacInc == 2 * nameEdits.size() / 3)
		{
			// go to second or third collumn
			loc.x += 380;
			loc.y -= 25 * nameEdits.size() / 3;
		}
		// create label
		numberLabels[dacInc].sPos = { loc.x, loc.y, loc.x += 20, loc.y + 20 };
		numberLabels[dacInc].Create(cstr(dacInc), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, 
									numberLabels[dacInc].sPos, this);

		nameEdits[dacInc].sPos = { loc.x, loc.y, loc.x += 120, loc.y + 20 };
		nameEdits[dacInc].Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, nameEdits[dacInc].sPos, this, id++);
		nameEdits[dacInc].SetWindowTextA(cstr(input->aoSys->getName(dacInc)));

		minValEdits[dacInc].sPos = { loc.x, loc.y, loc.x += 120, loc.y + 20 };
		minValEdits[dacInc].Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, minValEdits[dacInc].sPos, this, id++);
		minValEdits[dacInc].SetWindowTextA(cstr(input->aoSys->getDacRange(dacInc).first));

		maxValEdits[dacInc].sPos = { loc.x, loc.y, loc.x += 120, loc.y + 20 };
		maxValEdits[dacInc].Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, maxValEdits[dacInc].sPos, this, id++);
		maxValEdits[dacInc].SetWindowTextA(cstr(input->aoSys->getDacRange(dacInc).second));

		loc.y += 25;
		loc.x -= 380;
	}
	return TRUE;
}
