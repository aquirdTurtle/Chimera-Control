// created by Mark O. Brown
#include "stdafx.h"
#include "ColorBox.h"
#include <tuple>


void ColorBox::initialize(POINT& pos, int& id, CWnd* parent, int length, cToolTips& tooltips, DeviceList devices)
{
	auto numCtrls = devices.list.size ();
	boxes.resize (numCtrls);
	for (auto devInc : range (numCtrls))
	{
		auto& box = boxes[devInc];
		auto& dev = devices.list[devInc].get ();
		box.delim = dev.getDelim ();
		box.ctrl = std::make_unique<Control<CEdit>> ();
		box.ctrl->sPos = { long (pos.x + devInc * length / numCtrls), pos.y, 
						   long (pos.x + (devInc+1) * length / numCtrls), pos.y + 20 };
		box.ctrl->Create( NORM_STATIC_OPTIONS | SS_ENDELLIPSIS, box.ctrl->sPos,
						  parent, id++);
		box.ctrl->SetWindowTextA (box.delim.substr (0, 3).c_str ());
		box.ctrl->fontType = fontTypes::SmallCodeFont;
		box.ctrl->setToolTip (box.delim, tooltips, parent);
	}
	pos.y += 20;
	initialized = true;
}


void ColorBox::rearrange( int width, int height, fontMap fonts )
{
	for (auto& box : boxes)
	{
		box.ctrl->rearrange (width, height, fonts);
	}
}

CBrush* ColorBox::handleColoring( int id, CDC* pDC )
{
	if (!initialized)
	{
		return NULL;
	}
	char code='\0';
	for (auto& box : boxes)
	{
		if (id == box.ctrl->GetDlgCtrlID ())
		{
			code = box.color;
		}
	}
	if (code == '\0')
	{
		return NULL;
	}
	if (code == 'W')
	{
		// Loading Settings.
		pDC->SetTextColor (_myRGBs["Black"]);
		pDC->SetBkColor (_myRGBs["White"]);
		return _myBrushes["White"];
	}
	if (code == 'G')
	{
		// Device has been programmed successfully. 
		pDC->SetTextColor(_myRGBs["White"]);
		pDC->SetBkColor( _myRGBs["Green"]);
		return _myBrushes["Green"];
	}
	else if (code == 'Y')
	{
		// Calculating Variations
		pDC->SetTextColor( _myRGBs["White"]);
		pDC->SetBkColor( _myRGBs["Gold"]);
		return _myBrushes["Gold"];
	}
	else if (code == 'R')
	{
		// Error.
		pDC->SetTextColor( _myRGBs["White"]);
		pDC->SetBkColor( _myRGBs["Red"]);
		return _myBrushes["Red"];
	}
	else if (code == 'B')
	{
		// Error.
		pDC->SetTextColor (_myRGBs["White"]);
		pDC->SetBkColor (_myRGBs["Blue"]);
		return _myBrushes["Blue"];
	}
	else
	{
		// Default, Not running, no chance to run. 
		pDC->SetTextColor( _myRGBs["White"] );
		pDC->SetBkColor( _myRGBs["Dark Grey"] );
		return _myBrushes["Dark Grey"];
	}
}


void ColorBox::changeColor( std::string delim, char color )
{
	for (auto& device : boxes)
	{
		if (device.delim == delim)
		{
			device.color = color;
			device.ctrl->RedrawWindow ();
		}
	}
}