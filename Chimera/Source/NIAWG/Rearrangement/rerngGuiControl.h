// created by Mark O. Brown
#pragma once
#include "CustomMfcControlWrappers/myButton.h"
#include "NIAWG/Rearrangement/rerngOptionStructures.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "CustomMfcControlWrappers/Control.h"
#include "Windows.h"
#include "afxwin.h"
;
/*
 * A class for a gui end control for setting various rearrangement parameters
 */
class rerngGuiControl
{
	public:
		rerngGuiOptions getParams( );
		void setParams( rerngGuiOptions params );
		void initialize( int& id, POINT& loc, CWnd* parent, cToolTips& tooltips );
		static rerngGuiOptions getSettingsFromConfig (ConfigStream& config);
		void handleOpenConfig( ConfigStream& openFile);
		void handleSaveConfig( ConfigStream& newFile );
		void rearrange( int width, int height, fontMap fonts );
		void updateActive ( );
		void setEnabled (bool enabled);
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

		Control<CStatic> fastMoveTime;
		Control<CEdit> fastMoveTimeEdit;

		Control<CComboBox> rerngModeCombo;
};
