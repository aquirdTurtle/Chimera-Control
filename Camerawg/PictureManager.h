#pragma once
#include "PictureControl.h"
#include <array>

class PictureManager
{
	public:
		void initialize(POINT& loc, CWnd* parent, int& id);
		void refreshBackgrounds(CWnd* parent);
		void drawGrids(CWnd* parent, CBrush* brush);
		void setParameters(imageParameters parameters);
		void rearrange(std::string cameraMode, std::string triggerMode, int width, int height);
		void handleScroll(UINT nSBCode, UINT nPos, CScrollBar* scrollbar);
		// draw pictures...
	private:
		std::array<PictureControl, 4> pictures;
};

