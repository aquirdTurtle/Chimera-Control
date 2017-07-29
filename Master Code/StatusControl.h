#pragma once

#include "Control.h"
#include "commonTypes.h"

class StatusControl
{
	public:
		void initialize(POINT &topLeftCorner, CWnd* parent, int& id, long width, long height,
						std::string headerText, COLORREF textColor, fontMap fonts, std::vector<CToolTipCtrl*>& tooltips);
		void setDefaultColor(COLORREF color);
		void addStatusText(std::string text);
		void addStatusText(std::string text, bool noColor);
		void deleteChars(int num);
		void clear();
		void setColor();
		void setColor(COLORREF color);
		void appendTimebar();
		void rearrange(int width, int height, fontMap fonts);
		std::string getText();
	private:
		Control<CStatic> header;
		Control<CRichEditCtrl> edit;
		Control<CButton> clearButton;
		COLORREF defaultColor;
};