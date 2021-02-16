// created by Mark O. Brown
#pragma once
#include "control.h"
#include "AoStructures.h"
#include <CustomQtControls/AutoNotifyCtrls.h>
#include <qlabel.h>
#include "PrimaryWindows/IChimeraQtWindow.h"


class AnalogOutput
{
	public:
		AnalogOutput ( );
		void initialize ( QPoint& pos, IChimeraQtWindow* parent, int whichDac );
		void handleEdit ( bool roundToDacPrecision=false );
		void updateEdit ( bool roundToDacPrecision );
		static double roundToDacResolution ( double num );
		double getVal ( bool useDefault );
		bool eventFilter (QObject* obj, QEvent* event);
		void setNote ( std::string note );
		AoInfo info;
		void setName ( std::string name );
		void disable ( );
	private:
		unsigned dacNum;
		CQLineEdit* edit;
		QLabel* label;
};