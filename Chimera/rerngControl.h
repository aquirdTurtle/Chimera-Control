#pragma once
#include "rerngParams.h"
#include "Control.h"
#include "Windows.h"
#include "afxwin.h"

/*
 * A class for a gui end control for setting various rearrangement parameters
 */
class rerngControl
{
	public:
		rerngParams getParams( );
		void setParams( rerngParams params );
		void initialize( int& id, POINT& loc, CWnd* parent, cToolTips& tooltips );
		void handleOpenConfig( std::ifstream& openFile, int versionMajor, int versionMinor );
		void handleNewConfig( std::ofstream& newFile );
		void handleSaveConfig( std::ofstream& newFile );
		void rearrange( int width, int height, fontMap fonts );
	private:
		Control<CStatic> header;
		Control<CButton> experimentIncludesRerng;
		Control<CStatic> flashingRateText;
		Control<CEdit> flashingRateEdit;
		Control<CStatic> moveSpeedText;
		Control<CEdit> moveSpeedEdit;
		Control<CStatic> movingBiasText;
		Control<CEdit> movingBiasEdit;

		Control<CStatic> deadTimeText;
		Control<CEdit> deadTimeEdit;

		Control<CStatic> staticMovingRatioText;
		Control<CEdit> staticMovingRatioEdit;

		Control<CButton> preprogramMoves;
		Control<CButton> useCalibration;

		Control<CButton> outputRearrangeEvents;
		Control<CButton> outputIndividualEvents;
};