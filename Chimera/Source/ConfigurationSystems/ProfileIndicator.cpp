// created by Mark O. Brown
#include "stdafx.h"
#include "ProfileIndicator.h"
#include "GeneralUtilityFunctions/my_str.h"

void ProfileIndicator::initialize(QPoint pos, QWidget* parent ) {
	auto& px = pos.rx (), & py = pos.ry ();
	header = new QLabel ("Congfiguration: ", parent);
	header->setGeometry (px, py, 200, 20);
	indicator = new QLabel ("", parent);
	indicator->setGeometry (px + 200, py, 1000, 20);
	py += 20;
}

void ProfileIndicator::update(std::string text)
{
	indicator->setText(cstr(text));
}

