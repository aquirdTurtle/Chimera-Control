#pragma once

#include "Control.h"
#include "Windows.h"

class ProfileIndicator
{
	public:
		void initialize(POINT position, int& id, CWnd* parent, fontMap fonts, std::vector<CToolTipCtrl*>& tooltips);
		void update(std::string text);
		void rearrange(int width, int height, fontMap fonts);
	private:
		Control<CStatic> header;
		Control<CStatic> indicator;
};

