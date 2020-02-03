#pragma once
#include "Control.h"
#include <string>
#include <unordered_map>

class RichEditControl
{
	public:
		bool appendText(std::string text, int color);
		void deleteChars( int num );
		void clear();
		void initialize(RECT editSize, std::string titleText, COLORREF defaultTextColor, AuxiliaryWindow* master, 
						 int& id);
		INT_PTR handleColorMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, brushMap brushes);
		std::string getText();
		void rearrange(UINT width, UINT height, fontMap fonts);
	private:
		Control<CStatic> title;
		Control<CRichEditCtrl> richEdit;
		Control<CButton> clearButton;
		COLORREF defaultTextColor;
		unsigned int textColor;
		unsigned int backgroundColor;
};