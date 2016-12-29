#pragma once

#include "Control.h"
#include "Windows.h"

class ProfileIndicator
{
	public:
		void initialize(POINT position, int& id, CWnd* parent, std::unordered_map<std::string, CFont*> fonts, 
			std::vector<CToolTipCtrl*>& tooltips);
		void update(std::string text);
	private:
		Control<CStatic> header;
		Control<CStatic> indicator;
};

