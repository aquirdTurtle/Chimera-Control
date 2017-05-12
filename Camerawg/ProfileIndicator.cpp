#include "stdafx.h"
#include "ProfileIndicator.h"

void ProfileIndicator::initialize(POINT pos, int& id, CWnd* parent, std::unordered_map<std::string, CFont*> fonts,
								  std::vector<CToolTipCtrl*>& tooltips)
{
	header.sPos = { pos.x, pos.y, pos.x + 200, pos.y + 20 };
	header.ID = id++;
	header.Create("CONFIGURATION:", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, header.sPos, parent, header.ID);
	header.SetFont(fonts["Normal Font"]);
	pos.x += 200;
	indicator.sPos = { pos.x, pos.y, pos.x + 1500, pos.y + 20 };
	indicator.ID = id++;
	indicator.Create("", WS_CHILD | WS_VISIBLE | SS_ENDELLIPSIS, indicator.sPos, parent, indicator.ID);
	indicator.SetFont(fonts["Normal Fonts"]);
	pos.y += 20;
}

void ProfileIndicator::update(std::string text)
{
	indicator.SetWindowTextA(text.c_str());
}

void ProfileIndicator::rearrange(int width, int height, std::unordered_map<std::string, CFont*> fonts)
{
	header.rearrange("", "", width, height, fonts);
	indicator.rearrange("", "", width, height, fonts);
}