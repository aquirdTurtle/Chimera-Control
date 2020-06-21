// created by Mark O. Brown
#pragma once
#include "control.h"
#include "AoStructures.h"
#include <CustomQtControls/AutoNotifyCtrls.h>
#include <qlabel.h>
#include "PrimaryWindows/IChimeraWindowWidget.h"


class AnalogOutput
{
	public:
		AnalogOutput ( );
		void initialize ( POINT& pos, IChimeraWindowWidget* parent, int whichDac );
		void handleEdit ( bool roundToDacPrecision=false );
		void updateEdit ( bool roundToDacPrecision );
		static double roundToDacResolution ( double num );
		double getVal ( bool useDefault );
		bool eventFilter (QObject* obj, QEvent* event);
		void setNote ( std::string note );
		AoInfo info;
		void setName ( std::string name );
		void disable ( );
		bool handleArrow ( CWnd* focus, bool up );
	private:
		UINT dacNum;
		CQLineEdit* edit;
		QLabel* label;
};