#include "stdafx.h"
#include "Repetitions.h"
#include <unordered_map>
#include "constants.h"
#include "AuxiliaryWindow.h"


void Repetitions::rearrange(UINT width, UINT height, fontMap fonts)
{
	repetitionEdit.rearrange(width, height, fonts);
	repetitionDisp.rearrange(width, height, fonts);
	repetitionText.rearrange(width, height, fonts);
}


void Repetitions::handleOpenConfig(std::ifstream& openFile, int versionMajor, int versionMinor )
{
	ProfileSystem::checkDelimiterLine(openFile, "REPETITIONS");
	UINT repNum;
	openFile >> repNum;
	ProfileSystem::checkDelimiterLine(openFile, "END_REPETITIONS");
	setRepetitions(repNum);
}

void Repetitions::handleNewConfig( std::ofstream& newFile )
{
	newFile << "REPETITIONS\n";
	newFile << 100 << "\n";
	newFile << "END_REPETITIONS\n";
}

void Repetitions::handleSaveConfig(std::ofstream& saveFile)
{
	saveFile << "REPETITIONS\n";
	saveFile << getRepetitionNumber() << "\n";
	saveFile << "END_REPETITIONS\n";
}


HBRUSH Repetitions::handleColorMessage(CWnd* window, brushMap brushes, rgbMap rGBs, CDC* cDC)
{
	DWORD controlID = window->GetDlgCtrlID();
	if (controlID == repetitionDisp.GetDlgCtrlID() || controlID == repetitionEdit.GetDlgCtrlID())
	{
		cDC->SetBkColor(rGBs["Medium Grey"]);
		cDC->SetTextColor(rGBs["White"]);
		return *brushes["Medium Grey"];
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


void Repetitions::initialize(POINT& pos, cToolTips& toolTips, MainWindow* mainWin, int& id)
{
	// title
	repetitionText.sPos = { pos.x, pos.y, pos.x + 180, pos.y + 20 };
	repetitionText.Create("Repetition #", NORM_STATIC_OPTIONS, repetitionText.sPos, mainWin, id++);
	
	repetitionEdit.sPos = { pos.x + 180, pos.y, pos.x + 330, pos.y + 20 };
	repetitionEdit.Create( NORM_EDIT_OPTIONS, repetitionEdit.sPos, mainWin, IDC_REPETITION_EDIT );
	repetitionEdit.SetWindowText("100");

	repetitionDisp.sPos = { pos.x + 330, pos.y, pos.x + 480, pos.y += 20 };
	repetitionDisp.Create( NORM_STATIC_OPTIONS | SS_SUNKEN , repetitionDisp.sPos,
						  mainWin, id++);
	repetitionDisp.SetWindowText("-");
	// initialize the number to match the display.
	repetitionNumber = 100;
}


void Repetitions::setRepetitions(UINT number)
{
	// check number for reasonable-ness?
	repetitionNumber = number;
	repetitionEdit.SetWindowTextA(cstr(number));
	repetitionDisp.SetWindowTextA("---");
}


UINT Repetitions::getRepetitionNumber()
{
	CString text;
	repetitionEdit.GetWindowText(text);
	try
	{
		repetitionNumber = std::stoi(str(text));
	}
	catch (std::invalid_argument&)
	{
		thrower("ERROR: Failed to convert repetition number text to an integer!");
	}
	return repetitionNumber;
}

