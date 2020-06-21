// created by Mark O. Brown
#include "stdafx.h"
#include "StatusIndicator.h"
#include <PrimaryWindows/IChimeraWindowWidget.h>

void StatusIndicator::rearrange(int width, int height, fontMap fonts)
{
	//status.rearrange( width, height, fonts);
}


void StatusIndicator::initialize(POINT &loc, IChimeraWindowWidget* parent )
{
	status = new QLabel ("Passively Outputing Default Waveform", parent);
	status->setGeometry (loc.x, loc.y, 960, 100);
	status->setStyleSheet ("QLabel {font: bold 30pt;}");
}


void StatusIndicator::setText(std::string text)
{
	status->setText (cstr (text));
}


void StatusIndicator::setColor(std::string color)
{
	currentColor = color;
}


CBrush* StatusIndicator::handleColor(CWnd* window, CDC* pDC )
{
	DWORD controlID = GetDlgCtrlID(window->GetSafeHwnd());
	/*
	if (controlID == status.GetDlgCtrlID())
	{
		pDC->SetTextColor(_myRGBs["White"]);
		if (this->currentColor == "G")
		{
			pDC->SetBkColor( _myRGBs["Green"]);
			return _myBrushes["Green"];
		}
		else if (currentColor == "Y")
		{
			pDC->SetTextColor( _myRGBs["Black"]);
			pDC->SetBkColor(_myRGBs["Gold"]);
			return _myBrushes["Gold"];
		}
		else if (currentColor == "R")
		{
			pDC->SetBkColor( _myRGBs["Dark Red"]);
			return _myBrushes["Dark Red"];
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
	*/
	return NULL;
}

