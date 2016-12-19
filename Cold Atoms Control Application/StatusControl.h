#pragma once

#include "Control.h"

class StatusControl
{
	public:
		void initialize(POINT &topLeftCorner, CWnd* parent, int& id, unsigned int size, std::string headerText, COLORREF textColor);
		void setDefaultColor(COLORREF color);
		void addStatusText(std::string text);
		void clear();
		void appendTimebar();
	private:
		Control<CStatic> header;
		Control<CRichEditCtrl> edit;
		Control<CButton> clearButton;
		COLORREF defaultColor;
};