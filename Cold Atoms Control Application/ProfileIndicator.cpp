#include "stdafx.h"
#include "ProfileIndicator.h"

void ProfileIndicator::initialize(POINT position, int& id, CWnd* parent)
{
	this->header.position = { position.x, position.y, position.x + 200, position.y + 20 };
	header.ID = id++;
	header.Create("CONFIGURATION:", WS_CHILD | WS_VISIBLE | SS_SUNKEN | SS_CENTER, header.position, parent, header.ID);
	header.SetFont(&eNormalFont);
	position.x += 200;
	this->indicator.position = { position.x, position.y, position.x + 1500, position.y + 20 };
	indicator.ID = id++;
	indicator.Create("", WS_CHILD | WS_VISIBLE | SS_ENDELLIPSIS, indicator.position, parent, indicator.ID);
	indicator.SetFont(&eNormalFont);
	position.y += 20;
	return;
}

void ProfileIndicator::update(std::string text)
{
	this->indicator.SetWindowTextA(text.c_str());
	return;
}