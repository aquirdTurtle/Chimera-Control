#pragma once
#include "Control.h"
#include "CameraImageParameters.h"

class PictureControl
{
	public:
		void initialize(POINT& loc, CWnd* parent, int& id, int width, int height);
		void updateGridSpecs(imageParameters newParameters);
		void drawBitmap();
		void drawBackground(CWnd* parent);
		void drawGrid(CWnd* parent, CBrush* brush);
		void drawRectangle(CWnd* parent, CBrush* brush);
		void drawCircle(CWnd* parent, CBrush* brush);
		void rearrange(std::string cameraMode, std::string triggerMode, int width, int height);
		void handleScroll(int id, UINT nPos);
	private:
		RECT backgroundArea;
		// 
		int maxSliderPosition;
		int minSliderPosition;
		// grid data
		std::vector<std::vector<RECT>> grid;
		// Picture location data
		Control<CSliderCtrl> sliderMax;
		Control<CSliderCtrl> sliderMin;
		//
		Control<CStatic> labelMax;
		Control<CStatic> labelMin;
		//
		Control<CEdit> editMax;
		Control<CEdit> editMin;
};
