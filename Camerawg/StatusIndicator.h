#pragma once
#include "Control.h"
class StatusIndicator
{
	public:
		void initialize(POINT &loc, CWnd* parent, int& id);
		void setText(std::string text);
		void setColor(std::string color);
		CBrush* handleColor(CWnd* window, CDC* pDC, std::unordered_map<std::string, COLORREF> rgbs, std::unordered_map<std::string, CBrush*> brushes);
	private:
		Control<CStatic> status;
		std::string currentColor;
};
