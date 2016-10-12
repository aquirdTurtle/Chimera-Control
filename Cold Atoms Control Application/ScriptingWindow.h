#pragma once

#include "stdafx.h"

class ScriptingWindow : public CDialog
{
	using CDialog::CDialog;
	DECLARE_DYNAMIC(ScriptingWindow);

	public:
		ScriptingWindow() : CDialog()
		{
			scriptRGBs["Dark Grey"] = (RGB(15, 15, 15));
			scriptRGBs["Dark Grey Red"] = RGB(20, 12, 12);
			scriptRGBs["Medium Grey"] = (RGB(30, 30, 30));
			scriptRGBs["Light Grey"] = (RGB(60, 60, 60));
			scriptRGBs["Green"] = (RGB(50, 200, 50));
			scriptRGBs["Red"] = (RGB(200, 50, 50));
			scriptRGBs["Blue"] = RGB(50, 50, 200);
			scriptRGBs["Gold"] = RGB(218, 165, 32);
			scriptRGBs["White"] = RGB(255, 255, 255);
			scriptRGBs["Light Red"] = (RGB(255, 100, 100));
			scriptRGBs["Light Blue"] = RGB(100, 100, 255);
			scriptRGBs["Forest Green"] = RGB(34, 139, 34);
			scriptRGBs["Dull Red"] = RGB(107, 35, 35);
			scriptRGBs["Dark Lavender"] = RGB(100, 100, 205);
			scriptRGBs["Teal"] = RGB(0, 255, 255);
			scriptRGBs["Tan"] = RGB(210, 180, 140);
			scriptRGBs["Purple"] = RGB(147, 112, 219);
			scriptRGBs["Orange"] = RGB(255, 165, 0);
			scriptRGBs["Brown"] = RGB(139, 69, 19);
			scriptRGBs["Black"] = RGB(0, 0, 0);
			scriptRGBs["Dark Red"] = RGB(75, 0, 0);

			// there are less brushes because these are only used for backgrounds.
			scriptBrushes["Dark Grey"] = CreateSolidBrush(scriptRGBs["Dark Grey"]);
			scriptBrushes["Dark Grey Red"] = CreateSolidBrush(scriptRGBs["Dark Grey Red"]);
			scriptBrushes["Medium Grey"] = CreateSolidBrush(scriptRGBs["Medium Grey"]);
			scriptBrushes["Light Grey"] = CreateSolidBrush(scriptRGBs["Light Grey"]);
			scriptBrushes["Green"] = CreateSolidBrush(scriptRGBs["Green"]);
			scriptBrushes["Red"] = CreateSolidBrush(scriptRGBs["Red"]);
			scriptBrushes["White"] = CreateSolidBrush(scriptRGBs["White"]);
			scriptBrushes["Dull Red"] = CreateSolidBrush(scriptRGBs["Dull Red"]);
			scriptBrushes["Dark Red"] = CreateSolidBrush(scriptRGBs["Dark Red"]);
		}
		HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
		BOOL OnInitDialog() override;
	private:
		DECLARE_MESSAGE_MAP();
		
		std::unordered_map<std::string, HBRUSH> scriptBrushes;
		std::unordered_map<std::string, COLORREF> scriptRGBs;
};
