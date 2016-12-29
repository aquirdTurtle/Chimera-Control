#pragma once
#include "PictureControl.h"
#include <array>

class PictureManager
{
	public:
		void initialize(POINT& loc, CWnd* parent, int& id, std::unordered_map<std::string, CFont*> fonts,
			std::vector<CToolTipCtrl*>& tooltips);
		void refreshBackgrounds(CWnd* parent);
		void drawGrids(CWnd* parent, CBrush* brush);
		void setParameters(imageParameters parameters);
		void rearrange(std::string cameraMode, std::string triggerMode, int width, int height, std::unordered_map<std::string, CFont*> fonts);
		void handleScroll(UINT nSBCode, UINT nPos, CScrollBar* scrollbar);
		// draw pictures...
	private:
		std::array<PictureControl, 4> pictures;
};

