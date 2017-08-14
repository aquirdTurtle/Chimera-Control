#pragma once

#include "Control.h"
#include "commonTypes.h"

struct mainOptions
{
	bool dontActuallyGenerate;
	bool programIntensity;
};

class MainOptionsControl
{
	public:
		void handleSaveConfig(std::ofstream& saveFile);
		void handleOpenConfig(std::ifstream& openFile, double version);
		void initialize(int& idStart, POINT& loc, CWnd* parent, fontMap fonts, cToolTips& tooltips);
		bool handleEvent(UINT id, MainWindow* comm);
		mainOptions getOptions();
		void setOptions(mainOptions options);
		void rearrange(int width, int height, fontMap fonts);
	private:
		Control<CStatic> header;
		Control<CButton> controlIntensity;
		mainOptions currentOptions;
};