#pragma once
#include "Control.h"
#include <string>
#include <unordered_map>

class RichEditControl
{
	public:
		bool appendText(std::string text, int color);
		void deleteChars( int num );
		bool clear();
		bool initialize(RECT editSize, std::string titleText, COLORREF defaultTextColor, MasterWindow* master, 
						 int& id);
		INT_PTR handleColorMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, 
									std::unordered_map<std::string, HBRUSH> brushes);
		std::string getText();

	private:
		Control<CStatic> title;
		Control<CRichEditCtrl> richEdit;
		Control<CButton> clearButton;
		COLORREF defaultTextColor;
		unsigned int textColor;
		unsigned int backgroundColor;

};