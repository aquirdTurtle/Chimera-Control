#pragma once
#include "myButton.h"
#include "rerngParams.h"
#include "Version.h"
#include "Control.h"
#include "Windows.h"
#include "afxwin.h"

/*
 * A class for a gui end control for setting various rearrangement parameters
 */
class rerngGuiControl
{
	public:
		rerngGuiOptionsForm getParams( );
		void setParams( rerngGuiOptionsForm params );
		void initialize( int& id, POINT& loc, CWnd* parent, cToolTips& tooltips );
		void handleOpenConfig( std::ifstream& openFile, Version ver );
		void handleNewConfig( std::ofstream& newFile );
		void handleSaveConfig( std::ofstream& newFile );
		void rearrange( int width, int height, fontMap fonts );
		void handleCheck( );
	private:
		Control<CStatic> header;
		Control<CleanCheck> experimentIncludesRerng;
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

		Control<CleanCheck> preprogramMoves;
		Control<CleanCheck> useCalibration;

		Control<CleanCheck> outputRearrangeEvents;
		Control<CleanCheck> outputIndividualEvents;

		Control<CStatic> finalMoveTimeText;
		Control<CEdit> finalMoveTimeEdit;

		Control<CleanCheck> fastMoveOption;
		Control<CStatic> fastMoveTime;
		Control<CEdit> fastMoveTimeEdit;

		Control<CleanCheck> auxStaticOption;

};
