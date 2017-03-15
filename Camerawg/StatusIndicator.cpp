#include "stdafx.h"
#include "StatusIndicator.h"

void StatusIndicator::initialize(POINT &loc, CWnd* parent, int& id, std::unordered_map<std::string, CFont*> fonts,
	std::vector<CToolTipCtrl*>& tooltips)
{
	this->status.ID = id++;
	status.sPos = { loc.x, loc.y, loc.x + 960, loc.y + 100 };
	status.Create("Passively Outputing Default Waveform", WS_CHILD | WS_VISIBLE | SS_CENTER, status.sPos, parent, status.ID);
	status.SetFont(fonts["Large Heading Font"]);
}


void StatusIndicator::setText(std::string text)
{
	status.SetWindowText(text.c_str());
}


void StatusIndicator::setColor(std::string color)
{
	currentColor = color;
	status.RedrawWindow();
}


CBrush* StatusIndicator::handleColor(CWnd* window, CDC* pDC, std::unordered_map<std::string, COLORREF> rgbs, 
	std::unordered_map<std::string, CBrush*> brushes)
{
	DWORD controlID = GetDlgCtrlID(window->GetSafeHwnd());
	if (controlID == this->status.ID)
	{
		pDC->SetTextColor(rgbs["White"]);
		if (this->currentColor == "G")
		{
			pDC->SetBkColor(rgbs["Green"]);
			return brushes["Green"];
		}
		else if (currentColor == "Y")
		{
			pDC->SetTextColor(rgbs["Black"]);
			pDC->SetBkColor(rgbs["Gold"]);
			return brushes["Gold"];
		}
		else if (currentColor == "R")
		{
			pDC->SetBkColor(rgbs["Dark Red"]);
			return brushes["Dark Red"];
		}
		else
		{
			return NULL;
			// blue
		}
	}
	else
	{
		return NULL;
	}
}

