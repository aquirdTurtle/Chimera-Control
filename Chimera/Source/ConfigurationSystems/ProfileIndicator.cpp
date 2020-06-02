// created by Mark O. Brown
#include "stdafx.h"
#include "ProfileIndicator.h"
#include "GeneralUtilityFunctions/my_str.h"

void ProfileIndicator::initialize(POINT pos, QWidget* parent ) {
	header = new QLabel ("Congfiguration: ", parent);
	header->setGeometry (pos.x, pos.y, 200, 20);
	indicator = new QLabel ("", parent);
	indicator->setGeometry (pos.x + 200, pos.y, 1000, 20);
	pos.y += 20;
}

void ProfileIndicator::update(std::string text)
{
	indicator->setText(cstr(text));
}

void ProfileIndicator::rearrange(int width, int height, fontMap fonts){}