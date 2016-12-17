#include "stdafx.h"
#include "ColorBox.h"

void ColorBox::initialize(POINT pos, int& id, CWnd* parent)
{
	box.position = { pos.x, pos.y, pos.x + 40, pos.y + 20 };
	box.ID = id++;
	box.Create("", WS_CHILD | WS_VISIBLE | SS_WORDELLIPSIS | SS_CENTER, box.position, parent, box.ID);
	return;
}

void ColorBox::redraw()
{
	box.RedrawWindow();
	return;
}

bool ColorBox::isColoringThisBox(int id)
{
	if (id == box.ID)
	{
		return true;
	}
	else
	{
		return false;
	}
}
