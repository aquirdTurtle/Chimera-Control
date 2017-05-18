#include "stdafx.h"
#include "Repetitions.h"
#include <unordered_map>
#include "constants.h"
#include "MasterWindow.h"


void Repetitions::rearrange(UINT width, UINT height, fontMap fonts)
{
	repetitionEdit.rearrange(width, height, fonts);
	repetitionDisp.rearrange(width, height, fonts);
	setRepetitionButton.rearrange(width, height, fonts);
}


void Repetitions::handleButtonPush()
{
	CString text;
	this->repetitionEdit.GetWindowTextA(text);
	try
	{
		unsigned int repetitionNumber = std::stoi(text.GetBuffer());
		this->setRepetitions(repetitionNumber);
	}
	catch (std::invalid_argument& exception)
	{
		thrower(std::string("ERROR: repetition number text did not convert to int! Text was") + text.GetBuffer());
	}
}


void Repetitions::initialize(POINT& pos, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master, int& id)
{
	// title
	setRepetitionButton.sPos = { pos.x, pos.y, pos.x + 180, pos.y + 20 };
	setRepetitionButton.ID = id++;
	if ( setRepetitionButton.ID != SET_REPETITION_ID )
	{
		throw;
	}
	setRepetitionButton.Create("Set Repetition #", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 
								setRepetitionButton.sPos, master, setRepetitionButton.ID);
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
	return;
}

void Repetitions::setRepetitions(unsigned int number)
{
	// check number for reasonable-ness?
	this->repetitionNumber = number;
	this->repetitionDisp.SetWindowText(std::to_string(number).c_str());
}

unsigned int Repetitions::getRepetitionNumber()
{
	return this->repetitionNumber;
}

INT_PTR Repetitions::handleColorMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, std::unordered_map<std::string, HBRUSH> brushes)
{
	DWORD controlID = GetDlgCtrlID((HWND)lParam);
	HDC hdcStatic = (HDC)wParam;
	if (controlID == this->repetitionDisp.ID)
	{
		SetTextColor(hdcStatic, RGB(218, 165, 32));
		SetBkColor(hdcStatic, RGB(30, 30, 30));
		return (LRESULT)brushes["Medium Grey"];
	}
	else if (controlID == this->repetitionEdit.ID)
	{
		SetTextColor(hdcStatic, RGB(255, 255, 255));
		SetBkColor(hdcStatic, RGB(15, 15, 15));
		return (LRESULT)brushes["Dark Grey"];
	}
	else
	{
		return NULL;
	}
}