#pragma once

#include "Control.h"
#include "commonTypes.h"


struct mainOptions
{
	bool dontActuallyGenerate;
	bool rearrange;
	bool randomizeVariations;
	bool randomizeRepetitions;
};

// this got whittled down recently, but keeping so that I can put more stuff in later.
class MainOptionsControl
{
	public:
		void handleSaveConfig(std::ofstream& saveFile);
		void handleOpenConfig(std::ifstream& openFile, double version);
		void initialize(int& idStart, POINT& loc, CWnd* parent, cToolTips& tooltips);
		bool handleEvent(UINT id, MainWindow* comm);
		mainOptions getOptions();
		void rearrange(int width, int height, fontMap fonts);
	private:
		Control<CStatic> header;
		Control<CButton> rearrangeButton;
		Control<CButton> randomizeVariationsButton;
		Control<CButton> randomizeRepetitionsButton;
		mainOptions currentOptions;
};