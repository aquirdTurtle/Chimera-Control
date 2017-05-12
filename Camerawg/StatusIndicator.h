#pragma once
#include "Control.h"
class StatusIndicator
{
	public:
		void initialize(POINT &loc, CWnd* parent, int& id, std::unordered_map<std::string, CFont*> fonts,
			std::vector<CToolTipCtrl*>& tooltips);
		void setText(std::string text);
		void setColor(std::string color);
		CBrush* handleColor(CWnd* window, CDC* pDC, std::unordered_map<std::string, COLORREF> rgbs, 
			std::unordered_map<std::string, CBrush*> brushes);
		void rearrange(int width, int height, std::unordered_map<std::string, CFont*> fonts);
	private:
		Control<CStatic> status;
		std::string currentColor;
};
