#include "stdafx.h"
#include "ColorBox.h"

void ColorBox::initialize(POINT& pos, int& id, CWnd* parent, int length)
{
	box.sPos = { pos.x, pos.y, pos.x + length, pos.y + 20 };
	box.ID = id++;
	box.Create("", WS_CHILD | WS_VISIBLE | SS_WORDELLIPSIS | SS_CENTER, box.sPos, parent, box.ID);
	pos.y += 20;
	return;
}

void ColorBox::rearrange(std::string cameraMode, std::string triggerMode, int width, int height)
{
	this->box.rearrange(cameraMode, triggerMode, width, height);
}

CBrush* ColorBox::handleColoring(int id, CDC* pDC, std::unordered_map<std::string, CBrush*> brushes)
{
	if (id == box.ID)
	{
		if (currentColor == "G")
		{
			// Color Green. This is the "Ready to give next waveform" color. During this color you can also press esc to exit.
			pDC->SetTextColor(RGB(255, 255, 255));
			pDC->SetBkColor(RGB(0, 120, 0));
			return brushes["Green"];
		}
		else if (currentColor == "Y")
		{
			// Color Yellow. This is the "Working" Color.
			pDC->SetTextColor(RGB(255, 255, 255));
			pDC->SetBkColor(RGB(104, 104, 0));
			return brushes["Gold"];
		}
		else if (currentColor == "R")
		{
			// Color Red. This is a big visual signifier for when the program exited with error.
			pDC->SetTextColor(RGB(255, 255, 255));
			pDC->SetBkColor(RGB(120, 0, 0));
			return brushes["Red"];
		}
		else
		{
			// color Blue. This is the default, ready for user input color.
			pDC->SetTextColor(RGB(255, 255, 255));
			pDC->SetBkColor(RGB(0, 0, 120));
			return brushes["Dark Grey"];
		}
	}
	else
	{
		return NULL;
	}
}

void ColorBox::changeColor(std::string color)
{
	this->currentColor = color;
	this->box.RedrawWindow();
	return;
}