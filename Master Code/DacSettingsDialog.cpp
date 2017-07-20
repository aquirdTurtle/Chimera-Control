#include "stdafx.h"
#include "DacSettingsDialog.h"
#include "DacSystem.h"

IMPLEMENT_DYNAMIC(DacSettingsDialog, CDialog)


BEGIN_MESSAGE_MAP(DacSettingsDialog, CDialog)
	ON_COMMAND(IDOK, &DacSettingsDialog::handleOk)
END_MESSAGE_MAP()


void DacSettingsDialog::handleOk()
{
	for (int dacInc = 0; dacInc < nameEdits.size(); dacInc++)
	{
		/// MAKE SURE that there are 16 numbers per row.
		CString text;
		nameEdits[dacInc].GetWindowTextA(text);
		input->dacs->setName(dacInc, std::string(text), input->toolTips, input->master);
		double min, max;
		try
		{
			minValEdits[dacInc].GetWindowTextA(text);
			min = std::stod(std::string(text));
			maxValEdits[dacInc].GetWindowTextA(text);
			max = std::stod(std::string(text));
			input->dacs->setMinMax(dacInc, min, max);
		}
		catch (Error& err)
		{
			errBox(err.what());
		}
	}
	EndDialog(0);
}

void DacSettingsDialog::handleCancel()
{
	EndDialog(0);
}

BOOL DacSettingsDialog::OnInitDialog()
{
	//input = *(dacInputStruct*)lParam;
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

	for (int dacInc = 0; dacInc < nameEdits.size(); dacInc++)
	{
		if (dacInc == nameEdits.size() / 3 || dacInc == 2 * nameEdits.size() / 3)
		{
			// go to second or third collumn
			loc.x += 380;
			loc.y -= 25 * nameEdits.size() / 3;
		}
		// create label
		numberLabels[dacInc].sPos = { loc.x, loc.y, loc.x += 20, loc.y + 20 };
		numberLabels[dacInc].Create(std::to_string(dacInc).c_str(), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, 
							  numberLabels[dacInc].sPos, this);
		nameEdits[dacInc].sPos = { loc.x, loc.y, loc.x += 120, loc.y + 20 };
		nameEdits[dacInc].ID = startID++;
		nameEdits[dacInc].Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, nameEdits[dacInc].sPos, this,
								  nameEdits[dacInc].ID);
		nameEdits[dacInc].SetWindowTextA(input->dacs->getName(dacInc).c_str());
		minValEdits[dacInc].sPos = { loc.x, loc.y, loc.x += 120, loc.y + 20 };
		minValEdits[dacInc].ID = startID++;
		minValEdits[dacInc].Create( WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, minValEdits[dacInc].sPos, this,
								   minValEdits[dacInc].ID);
		minValEdits[dacInc].SetWindowTextA(cstr(input->dacs->getDacRange(dacInc).first));

		maxValEdits[dacInc].sPos = { loc.x, loc.y, loc.x += 120, loc.y + 20 };
		maxValEdits[dacInc].ID = startID++;
		maxValEdits[dacInc].Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, maxValEdits[dacInc].sPos, this,
								   maxValEdits[dacInc].ID);
		maxValEdits[dacInc].SetWindowTextA(cstr(input->dacs->getDacRange(dacInc).second));

		loc.y += 25;
		loc.x -= 380;
	}
	return TRUE;
}

INT_PTR CALLBACK viewAndChangeDAC_NamesProcedure(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static dacInputStruct input;
	static std::array<HWND, 24> nameEdits;
	static std::array<HWND, 24> minValEdits;
	static std::array<HWND, 24> maxValEdits;
	static int startx = 30, starty = 40;
	static int width = 63;
	static int height = 28;
	static int startID = 445;

	switch (message)
	{
		case WM_INITDIALOG:
		{
			input = *(dacInputStruct*)lParam;
			RECT location = { 0, 0, 20, 20 };
			for (int dacInc = 0; dacInc < nameEdits.size(); dacInc++)
			{
				if (dacInc == nameEdits.size() / 3 || dacInc == 2 * nameEdits.size() / 3)
				{
					// go to second or third collumn
					location.left += 160;
					location.right += 160;
					location.top -= 25 * nameEdits.size() / 3;
					location.bottom -= 25 * nameEdits.size() / 3;
				}
				// create label
				CreateWindowEx(0, "STATIC", std::to_string(dacInc).c_str(), WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER,
					location.left, location.top, location.right - location.left, location.bottom - location.top,
					hDlg, (HMENU)-1, GetModuleHandle(NULL), NULL);
				location.left += 20;
				location.right += 140;

				nameEdits[dacInc] = CreateWindowEx(0, "EDIT", input.dacs->getName(dacInc).c_str(), WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					location.left, location.top, location.right - location.left, location.bottom - location.top,
					hDlg, (HMENU)startID + dacInc, GetModuleHandle(NULL), NULL);

				location.left -= 20;
				location.right -= 140;
				location.top += 25;
				location.bottom += 25;
			}
			break;
		}
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case IDCANCEL:
				{
					EndDialog(hDlg, (WPARAM)&input);
					return (INT_PTR)TRUE;
				}
				case IDOK:
				{
					for (int dacInc = 0; dacInc < nameEdits.size(); dacInc++)
					{
						TCHAR name[256];
						/// MAKE SURE that there are 16 numbers per row.
						SendMessage(nameEdits[dacInc], WM_GETTEXT, 256, (LPARAM)name);
						input.dacs->setName(dacInc, name, input.toolTips, input.master);
					}
					EndDialog(hDlg, (WPARAM)&input);
					return (INT_PTR)TRUE;
				}
			}
			break;
		}
	}
	return (INT_PTR)FALSE;
}