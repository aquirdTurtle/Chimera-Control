#include "stdafx.h"
#include "Repetitions.h"
#include <unordered_map>
#include "constants.h"
#include "MasterWindow.h"


void Repetitions::rearrange(UINT width, UINT height, fontMap fonts)
{
	repetitionEdit.rearrange(width, height, fonts);
	repetitionDisp.rearrange(width, height, fonts);
	repetitionText.rearrange(width, height, fonts);
}


HBRUSH Repetitions::handleColorMessage(CWnd* window, brushMap brushes, rgbMap rGBs, CDC* cDC)
{
	DWORD controlID = window->GetDlgCtrlID();
	if (controlID == repetitionDisp.ID || controlID == repetitionEdit.ID)
	{
		cDC->SetBkColor(rGBs["Medium Grey"]);
		cDC->SetTextColor(rGBs["White"]);
		return brushes["Medium Grey"];
	}
	else
	{
		return NULL;
	}

}


void Repetitions::updateNumber(long repNumber)
{
	repetitionDisp.SetWindowText(cstr(repNumber));
}

/*
void Repetitions::handleButtonPush()
{
	CString text;
	repetitionEdit.GetWindowTextA(text);
	try
	{
		unsigned int repetitionNumber = std::stoi(text.GetBuffer());
		setRepetitions(repetitionNumber);
	}
	catch (std::invalid_argument& exception)
	{
		thrower(std::string("ERROR: repetition number text did not convert to int! Text was") + text.GetBuffer());
	}
}
*/

void Repetitions::initialize(POINT& pos, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master, int& id)
{
	// title
	repetitionText.sPos = { pos.x, pos.y, pos.x + 180, pos.y + 20 };
	repetitionText.ID = id++;
	repetitionText.Create("Repetition #", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, repetitionText.sPos, master, 
						  repetitionText.ID);

	repetitionEdit.sPos = { pos.x + 180, pos.y, pos.x + 330, pos.y + 20 };
	repetitionEdit.ID = id++;
	repetitionEdit.Create(WS_CHILD | WS_VISIBLE | SS_SUNKEN, repetitionEdit.sPos, master, repetitionEdit.ID);
	repetitionDisp.sPos = { pos.x + 330, pos.y, pos.x + 480, pos.y + 20 };
	repetitionDisp.ID = id++;
	repetitionDisp.Create(WS_CHILD | WS_VISIBLE | SS_SUNKEN | ES_CENTER | ES_READONLY | WS_BORDER, 
						   repetitionDisp.sPos, master, repetitionDisp.ID);
	repetitionDisp.SetWindowTextA("100");
	// initialize the number to match the display.
	repetitionNumber = 100;
	pos.y += 20;
}

void Repetitions::setRepetitions(unsigned int number)
{
	// check number for reasonable-ness?
	repetitionNumber = number;
	repetitionEdit.SetWindowTextA(std::to_string(number).c_str());
	repetitionDisp.SetWindowTextA("---");
}


unsigned int Repetitions::getRepetitionNumber()
{
	CString text;
	repetitionEdit.GetWindowText(text);
	try
	{
		repetitionNumber = std::stoi(std::string(text));
	}
	catch (std::invalid_argument& err)
	{
		thrower("ERROR: Failed to convert repetition number text to an integer!");
	}
	return repetitionNumber;
}

