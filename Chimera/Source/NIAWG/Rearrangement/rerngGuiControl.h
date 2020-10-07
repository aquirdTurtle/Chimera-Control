// created by Mark O. Brown
#pragma once
#include "NIAWG/Rearrangement/rerngOptionStructures.h"
#include "ConfigurationSystems/Version.h"
#include "ConfigurationSystems/ConfigStream.h"
#include "PrimaryWindows/IChimeraQtWindow.h"
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qcombobox.h>
/*
 * A class for a gui end control for setting various rearrangement parameters
 */
class rerngGuiControl
{
	public:
		rerngGuiOptions getParams( );
		void setParams( rerngGuiOptions params );
		void initialize( QPoint& loc, QWidget* parent );
		static rerngGuiOptions getSettingsFromConfig (ConfigStream& config);
		void handleOpenConfig( ConfigStream& openFile);
		void handleSaveConfig( ConfigStream& newFile );
		void updateActive ( );
		void setEnabled (bool enabled);
	private:
		QLabel* header;
		QCheckBox* experimentIncludesRerng;
		QLabel* flashingRateText;
		QLineEdit* flashingRateEdit;
		QLabel* moveSpeedText;
		QLineEdit* moveSpeedEdit;
		QLabel* movingBiasText;
		QLineEdit* movingBiasEdit;

		QLabel* deadTimeText;
		QLineEdit* deadTimeEdit;

		QLabel* staticMovingRatioText;
		QLineEdit* staticMovingRatioEdit;

		QCheckBox* preprogramMoves;
		QCheckBox* useCalibration;

		QCheckBox* outputRearrangeEvents;
		QCheckBox* outputIndividualEvents;

		QLabel* finalMoveTimeText;
		QLineEdit* finalMoveTimeEdit;

		QLabel* fastMoveTime;
		QLineEdit* fastMoveTimeEdit;

		QComboBox* rerngModeCombo;
};
