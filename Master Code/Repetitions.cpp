#include "stdafx.h"
#include "Repetitions.h"
#include <unordered_map>
#include "constants.h"

Repetitions::Repetitions(int& id)
{
	if (id != SET_REPETITION_ID)
	{
		MessageBox(0, "ERROR: SET_REPETITION_ID is not the actual id number!", 0, 0);
	}
	this->setRepetitionButton.ID = id;
	this->repetitionEdit.ID = id + 1;
	this->repetitionDisp.ID = id + 2;
	id += 3;
}

bool Repetitions::handleButtonPush()
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
		errBox(std::string("ERROR: repetition number text did not convert to int! Text was") + text.GetBuffer());
		return false;
	}
	return true;
}

bool Repetitions::initialize(POINT& topLeftPosition, HWND parentWindow, std::vector<CToolTipCtrl*>& toolTips, MasterWindow* master)
{
	// title
	RECT position;
	position = this->setRepetitionButton.position = { topLeftPosition.x, topLeftPosition.y, topLeftPosition.x + 180, topLeftPosition.y + 20 };
	this->setRepetitionButton.Create("Set Repetition #", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, position, CWnd::FromHandle(parentWindow), setRepetitionButton.ID);
	position = this->repetitionEdit.position = { topLeftPosition.x + 180, topLeftPosition.y, topLeftPosition.x + 330, topLeftPosition.y + 25 };
	this->repetitionEdit.Create(WS_CHILD | WS_VISIBLE | SS_SUNKEN, position, CWnd::FromHandle(parentWindow), repetitionEdit.ID);	
	position = this->repetitionDisp.position = { topLeftPosition.x + 330, topLeftPosition.y, topLeftPosition.x + 480, topLeftPosition.y + 25 };
	this->repetitionDisp.Create(WS_CHILD | WS_VISIBLE | SS_SUNKEN | ES_CENTER | ES_READONLY | WS_BORDER, position, CWnd::FromHandle(parentWindow), repetitionDisp.ID);
	repetitionDisp.SetWindowTextA("100");
	// initialize the number to match the display.
	this->repetitionNumber = 100;
	topLeftPosition.y += 20;
	return true;
}

bool Repetitions::setRepetitions(unsigned int number)
{
	// check number for reasonable-ness.
	this->repetitionNumber = number;
	this->repetitionDisp.SetWindowText(std::to_string(number).c_str());
	return true;
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