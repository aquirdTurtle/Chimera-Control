#pragma once
namespace myPlot 
{
	int saveAnalysisPixelLocations(HWND dialogHandle, bool clearEdits);
	int savePositiveConditions(HWND dialogHandle, bool clear);
	int savePostSelectionConditions(HWND dialogHandle, bool clear);
	int saveDataSet(HWND dialogHandle, bool clear);
	int enableAndDisable(HWND dialogHandle);
	std::string returnAllInfo();
}
