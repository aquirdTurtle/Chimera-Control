#pragma once
#include "stdafx.h"

class MainWindow : public CDialog
{
	using CDialog::CDialog;
	DECLARE_DYNAMIC(MainWindow);
	public:
		MainWindow() : CDialog()
		{
			mainRGBs["Dark Grey"] = (RGB(15, 15, 15));
			mainRGBs["Dark Grey Red"] = RGB(20, 12, 12);
			mainRGBs["Medium Grey"] = (RGB(30, 30, 30));
			mainRGBs["Light Grey"] = (RGB(60, 60, 60));
			mainRGBs["Green"] = (RGB(50, 200, 50));
			mainRGBs["Red"] = (RGB(200, 50, 50));
			mainRGBs["Blue"] = RGB(50, 50, 200);
			mainRGBs["Gold"] = RGB(218, 165, 32);
			mainRGBs["White"] = RGB(255, 255, 255);
			mainRGBs["Light Red"] = (RGB(255, 100, 100));
			mainRGBs["Light Blue"] = RGB(100, 100, 255);
			mainRGBs["Forest Green"] = RGB(34, 139, 34);
			mainRGBs["Dull Red"] = RGB(107, 35, 35);
			mainRGBs["Dark Lavender"] = RGB(100, 100, 205);
			mainRGBs["Teal"] = RGB(0, 255, 255);
			mainRGBs["Tan"] = RGB(210, 180, 140);
			mainRGBs["Purple"] = RGB(147, 112, 219);
			mainRGBs["Orange"] = RGB(255, 165, 0);
			mainRGBs["Brown"] = RGB(139, 69, 19);
			mainRGBs["Black"] = RGB(0, 0, 0);
			mainRGBs["Dark Blue"] = RGB(0, 0, 75);
			// there are less brushes because these are only used for backgrounds.
			mainBrushes["Dark Grey"] = CreateSolidBrush(mainRGBs["Dark Grey"]);
			mainBrushes["Dark Grey Red"] = CreateSolidBrush(mainRGBs["Dark Grey Red"]);
			mainBrushes["Medium Grey"] = CreateSolidBrush(mainRGBs["Medium Grey"]);
			mainBrushes["Light Grey"] = CreateSolidBrush(mainRGBs["Light Grey"]);
			mainBrushes["Green"] = CreateSolidBrush(mainRGBs["Green"]);
			mainBrushes["Red"] = CreateSolidBrush(mainRGBs["Red"]);
			mainBrushes["White"] = CreateSolidBrush(mainRGBs["White"]);
			mainBrushes["Dull Red"] = CreateSolidBrush(mainRGBs["Dull Red"]);
			mainBrushes["Dark Blue"] = CreateSolidBrush(mainRGBs["Dark Blue"]);
		}
		BOOL OnInitDialog() override;
		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	private:
		DECLARE_MESSAGE_MAP();
		std::unordered_map<std::string, HBRUSH> mainBrushes;
		std::unordered_map<std::string, COLORREF> mainRGBs;

		
};
