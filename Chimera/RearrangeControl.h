#pragma once
#include "Control.h"
#include "Windows.h"
#include "afxwin.h"

#include "rearrangeParams.h"

/*
 	A class for a gui end control for setting various rearrangement parameters
 */
class RearrangeControl
{
	public:
		rearrangeParams getParams( );
		void setParams( rearrangeParams params );
		void initialize( int& id, POINT& loc, CWnd* parent, cToolTips& tooltips );
		void handleOpenConfig( std::ifstream& openFile, double version );
		void handleNewConfig( std::ofstream& newFile );
		void handleSaveConfig( std::ofstream& newFile );
		void rearrange( int width, int height, fontMap fonts );
	private:
		Control<CStatic> header;
		Control<CButton> experimentIncludesRearrangement;
		Control<CStatic> flashingRateText;
		Control<CEdit> flashingRateEdit;
		Control<CStatic> moveSpeedText;
		Control<CEdit> moveSpeedEdit;
		Control<CStatic> movingBiasText;
		Control<CEdit> movingBiasEdit;

		//Control<CStatic> staticDutyCycleText;
		//Control<CEdit> dutyCycleEdit;
		//Control<CStatic> movingDutyCycleText;
		//Control<CEdit> movingDutyCycleEdit;

		Control<CStatic> deadTimeText;
		Control<CEdit> deadTimeEdit;

		Control<CStatic> staticMovingRatioText;
		Control<CEdit> staticMovingRatioEdit;
};