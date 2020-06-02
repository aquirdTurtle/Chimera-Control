// created by Mark O. Brown
#include "stdafx.h"
#include "ColorBox.h"
#include <tuple>
#include <PrimaryWindows/IChimeraWindowWidget.h>


void ColorBox::initialize(POINT& pos, IChimeraWindowWidget* parent, int length, DeviceList devices) {
	auto numCtrls = devices.list.size ();
	boxes.resize (numCtrls);
	for (auto devInc : range (numCtrls)) {
		auto& box = boxes[devInc];
		auto& dev = devices.list[devInc].get ();
		box.delim = dev.getDelim ();
		box.ctrl = new QLabel (box.delim.substr(0,3).c_str(), parent);
		box.ctrl->setToolTip (box.delim.c_str ());
		box.ctrl->setGeometry (pos.x + devInc * length / numCtrls, pos.y, length/numCtrls, 20);
	}
	pos.y += 20;
	initialized = true;
}


CBrush* ColorBox::handleColoring( int id, CDC* pDC )
{
	return NULL;
}


void ColorBox::changeColor( std::string delim, std::string color )
{
	for (auto& device : boxes)
	{
		if (device.delim == delim)
		{
			device.color = color;
			device.ctrl->setStyleSheet (("QLabel {background-color: \"" + str(color) + "\"}").c_str());
		}
	}
}